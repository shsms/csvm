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
    return false;
}

template <typename CompFunc, typename Forwarder>
void merge_and_forward(std::vector<merge_chunk> &chunks, const CompFunc &comp_func, const Forwarder &forwarder) {
    std::priority_queue<merge_row, std::vector<merge_row>, decltype(comp_func)>
        pri_queue(comp_func);

    for (int ii = 0; ii < chunks.size(); ii++) {
        if (!chunks[ii].empty()) {
            chunks[ii].set_curr_chunk_id(ii);
            pri_queue.push(std::move(chunks[ii].next_row()));
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
        if (!chunks[next.curr_chunk_id].empty()) {
            pri_queue.push(
                std::move(chunks[next.curr_chunk_id].next_row()));
        }
    }
    if (!out_chunk.data.empty()) {
        forwarder(out_chunk);
    }

}

bool mergestmt::run_merge_worker(
    threading::queue<merge_chunk> &in_queue,
    const std::function<void(models::bin_chunk &)> &forwarder) {

    bool reverse_compare = true;

    const auto comp_func = [this, &reverse_compare](const merge_row &a,
                                                    const merge_row &b) {
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

    std::vector<merge_chunk> chunks{};
    auto in_chunk = in_queue.dequeue();

    while (in_chunk.has_value()) {
        chunks.emplace_back(std::move(in_chunk.value()));
        in_chunk = in_queue.dequeue();
    }

    merge_and_forward(chunks, comp_func, forwarder);
    return true;
}

} // namespace engine
