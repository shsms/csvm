#include "merge_worker.hh"
#include <memory_resource>

namespace engine {

template <typename Collector>
void merge_worker::merge_and_collect(std::vector<merge_chunk> &chunks, Collector &collector) {
    // because pri_queue picks biggest first and we want smallest.
    bool reverse_compare = true;

    const auto comp_func = [this, &reverse_compare](const merge_row &a, const merge_row &b) {
        for (auto &col : this->columns) {
            if (a.m_row[col.pos] > b.m_row[col.pos]) {
                return reverse_compare != col.descending;
            }
            if (a.m_row[col.pos] < b.m_row[col.pos]) {
                return reverse_compare != (!col.descending);
            }
        }
        return reverse_compare != (a.orig_chunk_id < b.orig_chunk_id);
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
    // TODO: need additional merge levels to ensure there aren't too many files
    // to merge at the same time.
    std::vector<merge_chunk> file_chunks{};

    auto file_collector = [chunk = sorted_rows{}, &file_chunks](merge_row &mr,
                                                                bool cleanup_only) mutable {
        if (cleanup_only) {
            if (!chunk.empty()) {
                file_chunks.back().write(chunk);
                chunk.clear();
            }
            file_chunks.back().finish_writing();
            return;
        }
        if (chunk.size() >= 5000) {
            file_chunks.back().write(chunk);
            chunk.clear();
        }
        chunk.emplace_back(std::move(mr));
    };

    auto in_chunk = in_queue.dequeue();
    while (in_chunk.has_value()) {
        if (chunks.size() >= 50) {
            file_chunks.emplace_back(merge_chunk());
            merge_and_collect(chunks, file_collector);
            chunks.clear();
        }
        chunks.emplace_back(std::move(in_chunk.value()));
        in_chunk = in_queue.dequeue();
    }

    if (!file_chunks.empty() && !chunks.empty()) {
        file_chunks.emplace_back(merge_chunk());
        merge_and_collect(chunks, file_collector);
    }

    auto bin_chunk_collector = [chunk = models::bin_chunk{}, &merged](merge_row &mr,
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
    if (!file_chunks.empty()) {
        merge_and_collect(file_chunks, bin_chunk_collector);
    } else {
        merge_and_collect(chunks, bin_chunk_collector);
    }
    merged.set_eof();
    return true;
}

} // namespace engine
