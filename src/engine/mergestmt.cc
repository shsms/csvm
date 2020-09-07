#include "mergestmt.hh"

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
    return;
}

std::string mergestmt::string() { return ""; }

bool mergestmt::apply(models::bin_chunk &chunk,
                     std::stack<models::value> &eval_stack) {
    return false; // don't want print stuff to pick this up.
}

bool mergestmt::run_worker(threading::bin_queue &in_queue, std::function<void(models::bin_chunk&)> forwarder) {
    
    static const auto comp_func = [this](const models::row &a, const models::row &b) {
                         for (auto &col : this->columns) {
                             if (a[col.pos] > b[col.pos]) {
                                 return std::optional(col.descending);
                             }
                             if (a[col.pos] < b[col.pos]) {
                                 return std::optional(!col.descending);
                             }
                         }
                         return std::optional<bool>({});
    };
    auto m_comp_func = [](const models::row &a, const models::row &b) {
	auto v = comp_func(a, b);
	if (v.has_value()) {
	    return v.value();
	}
	return false;
    };
    std::deque<models::bin_chunk> chunks;
    auto in_chunk = in_queue.dequeue();
    while (in_chunk.has_value()) {
	if (chunks.size()%2 == 1) {
	    auto c = chunks.front();
	    models::bin_chunk merged;
	    std::merge(c.data.begin(), c.data.end(),
		       in_chunk->data.begin(), in_chunk->data.end(),
		       std::back_inserter(merged.data), m_comp_func);
	    chunks.pop_front();
	    chunks.emplace_back(std::move(merged));
	} else {
	    chunks.emplace_back(std::move(in_chunk.value()));
	}
        in_chunk = in_queue.dequeue();
    }
    std::cerr << "got chunks: " << chunks.size() << "\n";
    struct row {
	int chunk_id;
	int chunk_pos;
	models::row m_row;
	bool operator < (const row& c_row) const {
	    auto a = comp_func(m_row, c_row.m_row);
	    if (a.has_value()) {
		return !a.value();
	    }
	    return chunk_id > c_row.chunk_id;
	}
    } a;

    std::priority_queue<row> pri_queue;

    for (int ii = 0; ii < chunks.size(); ii++) {
	if (chunks[ii].data.size() > 0) {
	    pri_queue.push(row{ii, 0, std::move(chunks[ii].data[0])});
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
	if (next.chunk_pos < chunks[next.chunk_id].data.size()) {
	    pri_queue.push(row{next.chunk_id, next.chunk_pos, std::move(chunks[next.chunk_id].data[next.chunk_pos])});
	}
    }
    if (out_chunk.data.size() > 0) {
	forwarder(out_chunk);
    }
    return true;
}
} // namespace engine
