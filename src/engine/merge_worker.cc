#include "merge_worker.hh"
#include <future>
#include <memory_resource>

namespace engine {

template <typename Collector>
void merge_worker::merge_and_collect(std::vector<merge_chunk> chunks, Collector collector) {
    // inverse of comp_func in sort, because pri_queue tries to sort descending instead.
    const auto comp_func = [this](const merge_row &a, const merge_row &b) {
        for (auto &col : this->columns) {
            if (a.m_row[col.pos] > b.m_row[col.pos]) {
                return !col.descending;
            }
            if (a.m_row[col.pos] < b.m_row[col.pos]) {
                return col.descending;
            }
        }
        return a.orig_chunk_id >= b.orig_chunk_id;
    };

    std::priority_queue<merge_row, std::vector<merge_row>, decltype(comp_func)> pri_queue(
        comp_func);

    for (int ii = 0; ii < chunks.size(); ii++) {
        if (!chunks[ii].empty()) {
            chunks[ii].set_curr_chunk_id(ii);
            pri_queue.push(std::move(chunks[ii].next_row()));
        }
    }

    while (!pri_queue.empty()) {
        auto next = pri_queue.top();
        collector(next, false);
        pri_queue.pop();
        if (!chunks[next.curr_chunk_id].empty()) {
            pri_queue.push(std::move(chunks[next.curr_chunk_id].next_row()));
        }
    }

    // do I really want to change collector's first argument to raw
    // pointer to avoid an extra static unused initialization below?
    // std::optional can't carry reference and we need reference.  shared_ptr is
    // probably too much unnecessary overhead.
    static merge_row unused;
    collector(unused, true);
}

bool merge_worker::run(threading::queue<merge_chunk> &in_queue, threading::bin_queue &merged) {
    std::vector<merge_chunk> chunks{};
    std::vector<merge_chunk> file_chunks{};
    threading::queue<fetch_file_chunk_task> fetch_task_queue;

    // TODO: need additional merge levels to ensure there aren't too many files
    // to merge at the same time.

    auto file_collector = [](merge_chunk &target, sorted_rows &tmp_chunk, merge_row &mr,
                             bool cleanup_only) {
        if (cleanup_only) {
            if (!tmp_chunk.empty()) {
                target.write(tmp_chunk);
                tmp_chunk.clear();
            }
            target.finish_writing();
            return;
        }
        if (tmp_chunk.size() >= 5000) {
            target.write(tmp_chunk);
            tmp_chunk.clear();
        }
        tmp_chunk.emplace_back(std::move(mr));
    };

    auto out_collector = [chunk = models::bin_chunk{}, &merged](merge_row &mr,
                                                                bool cleanup_only) mutable {
        if (cleanup_only && !chunk.data.empty()) {
            merged.enqueue(std::move(chunk));
            return;
        }
        if (chunk.data.size() >= 5000) {
            auto next_id = chunk.id + 1;
            merged.enqueue(std::move(chunk));
            chunk.id = next_id;
            chunk.data.clear();
        }
        chunk.data.emplace_back(std::move(mr.m_row));
    };

    task_queue.set_limit(1);

    // start additional workers
    for (auto ii = 0; ii < args.thread_count; ii++) {
        additional_workers.emplace_back(std::thread([this, &fetch_task_queue, &file_collector]() {
            pthread_setname_np(pthread_self(), "csvm_merge_tmp");
            auto task = this->task_queue.dequeue();
            while (task.has_value()) {
                merge_chunk target(args, fetch_task_queue);
                auto file_collector_with_target = [&target, tmp_chunk = sorted_rows{},
                                                   &file_collector](merge_row &mr,
                                                                    bool cleanup_only) mutable {
                    file_collector(target, tmp_chunk, mr, cleanup_only);
                };
                merge_and_collect(std::move(task->chunks), file_collector_with_target);
                task->promise.set_value(std::move(target));
                task = this->task_queue.dequeue();
            }
        }));
    }

    std::vector<std::future<merge_chunk>> futures;
    auto in_chunk = in_queue.dequeue();
    while (in_chunk.has_value()) {
        // if too many in-mem chunks,  merge them and collect to tmp file.
        if (chunks.size() >= 32) {
            std::promise<merge_chunk> p;
            futures.push_back(std::move(p.get_future()));
            task_queue.enqueue({std::move(chunks), std::move(p)});
        }
        chunks.emplace_back(std::move(in_chunk.value()));
        in_chunk = in_queue.dequeue();
    }

    if (!futures.empty() && !chunks.empty()) {
        file_chunks.emplace_back(merge_chunk(args, fetch_task_queue));
        std::promise<merge_chunk> p;
        futures.push_back(std::move(p.get_future()));
        task_queue.enqueue({std::move(chunks), std::move(p)});
    }

    task_queue.set_eof();

    for (auto &t : additional_workers) {
        if (t.joinable()) {
            t.join();
        }
    }

    additional_workers.clear();

    for (auto ii = 0; ii < args.thread_count; ii++) {
        additional_workers.emplace_back(std::thread([this, &fetch_task_queue]() {
            pthread_setname_np(pthread_self(), "csvm_fetch_tmp");
            auto task = fetch_task_queue.dequeue();
            while (task.has_value()) {
                sorted_rows chunk;
                if (task->fs.get().eof()) {
                    task->promise.set_value(chunk);
                    task = fetch_task_queue.dequeue();
                    continue;
                }
                std::string raw = next_chunk(task->fs.get(), task->max_chunk_size);
                if (raw.empty()) {
                    task->promise.set_value(std::move(chunk));
                    task = fetch_task_queue.dequeue();
                    continue;
                }
                csv::parse_body(models::raw_chunk{0, raw}, [this, &task, &chunk](models::row &row) {
                    auto orig_id_str = std::get<std::string>(row.back());
                    auto orig_id = std::stoi(orig_id_str);
                    row.pop_back();
                    if (row.empty()) {
                        return;
                    }
                    for (const auto &num_item : task->num_pos) {
                        models::to_num(row[num_item]);
                    }
                    chunk.emplace_back(merge_row{orig_id, 0, row});
                });
                task->promise.set_value(std::move(chunk));
                task = fetch_task_queue.dequeue();
            }
        }));
    }

    for (auto &f : futures) {
        file_chunks.emplace_back(std::move(f.get()));
    }

    // do final merge, using the out_collector
    if (!file_chunks.empty()) {
        merge_and_collect(std::move(file_chunks), out_collector);
    } else {
        merge_and_collect(std::move(chunks), out_collector);
    }
    merged.set_eof();

    fetch_task_queue.set_eof();

    for (auto &t : additional_workers) {
        if (t.joinable()) {
            t.join();
        }
    }

    return true;
}

} // namespace engine
