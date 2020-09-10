#include "mergestmt.hh"
#include <memory_resource>

namespace engine {

void mergestmt::add_ident(const std::string &col) {
    columns.emplace_back(sortspec{false, false, 0, col});
    ++curr_pos;
}

void mergestmt::add_oper(const std::string &oper) {
    if (oper == "r") {
        columns[curr_pos - 1].descending = true;
    } else if (oper == "n") {
        columns[curr_pos - 1].numeric = true;
    } else {
        throw std::runtime_error(std::string("unknown operator to sort: ") +
                                 oper);
    }
}

void mergestmt::set_header(models::header_row &h) {
    for (auto &col : columns) {
        bool found = false;
        for (auto ii = 0; ii < h.size(); ++ii) {
            if (col.name == h[ii].name) {
                col.pos = ii;
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::runtime_error("column not found in header:" + col.name);
        }
    }
}

std::string mergestmt::string() { return ""; }

bool mergestmt::apply(models::bin_chunk & /*chunk*/,
                      std::stack<models::value> & /*eval_stack*/) {
    return false; // don't want print stuff to pick this up.
}

bool mergestmt::run_worker(
    threading::bin_queue &in_queue,
    const std::function<void(models::bin_chunk &)> &forwarder) {
    std::pmr::monotonic_buffer_resource mem{};

    bool reverse_compare = true;
    struct row {
        int chunk_id;
        int chunk_pos;
        models::row m_row;
    };

    const auto comp_func = [&](const row &a, const row &b) {
        for (auto &col : this->columns) {
            if (a.m_row[col.pos] > b.m_row[col.pos]) {
                return reverse_compare != col.descending;
            }
            if (a.m_row[col.pos] < b.m_row[col.pos]) {
                return reverse_compare != (!col.descending);
            }
        }
        return reverse_compare != (a.chunk_id < b.chunk_id);
    };

    using merge_chunk = std::pmr::vector<row>;
    std::pmr::vector<merge_chunk> chunks{&mem};
    auto in_chunk = in_queue.dequeue();

    while (in_chunk.has_value()) {
        merge_chunk new_chunk{&mem};
        for (auto &r : in_chunk->data) {
            new_chunk.emplace_back(row{in_chunk->id, 0, std::move(r)});
        }
        chunks.emplace_back(std::move(new_chunk));
        in_chunk = in_queue.dequeue();
    }
    std::sort(chunks.begin(), chunks.end(),
              [](const merge_chunk &a, const merge_chunk &b) {
                  if (a.empty() || b.empty()) {
                      return false;
                  }
                  return a[0].chunk_id < b[0].chunk_id;
              });

    std::priority_queue<row, std::pmr::vector<row>, decltype(comp_func)>
        pri_queue(comp_func, &mem);

    for (int ii = 0; ii < chunks.size(); ii++) {
        if (!chunks[ii].empty()) {
            pri_queue.push(std::move(chunks[ii][0]));
        }
    }

    models::bin_chunk out_chunk;
    out_chunk.data.reserve(5000);
    out_chunk.id = 0;
    while (!pri_queue.empty()) {
        if (out_chunk.data.size() >= 5000) {
            auto next_id = out_chunk.id + 1;
            forwarder(out_chunk);
            out_chunk.id = next_id;
            out_chunk.data.clear();
        }
        auto next = pri_queue.top();
        out_chunk.data.emplace_back(std::move(next.m_row));
        pri_queue.pop();
        ++next.chunk_pos;
        if (next.chunk_pos < chunks[next.chunk_id].size()) {
            chunks[next.chunk_id][next.chunk_pos].chunk_pos = next.chunk_pos;
            pri_queue.push(std::move(chunks[next.chunk_id][next.chunk_pos]));
        }
    }
    if (!out_chunk.data.empty()) {
        forwarder(out_chunk);
    }
    return true;
}
} // namespace engine
