#include "merge_chunk.hh"

namespace engine {

namespace stdfs = std::filesystem;

void merge_chunk::check_num_pos(const merge_row &mr) {
    for (int ii = 0; ii < mr.m_row.size(); ++ii) {
        if (std::holds_alternative<double>(mr.m_row[ii])) {
            num_pos.push_back(ii);
        }
    }
}

merge_chunk::merge_chunk(const cli_args &args,
                         threading::queue<fetch_file_chunk_task> &fetch_task_queue)
    : args(args), fetch_task_queue(fetch_task_queue) {
    src = disk;
    auto fnum = filenum.fetch_add(1);
    tmp_filename = args.in_filename;
    std::replace(tmp_filename.begin(), tmp_filename.end(), '/', '.');
    tmp_filename += "." + std::to_string(getpid()) + "." + std::to_string(fnum) + "~";

    tmp_filename = stdfs::path(args.temp_dir) / tmp_filename;
    fs.open(tmp_filename, std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
}

merge_chunk::~merge_chunk() {
    if (fs.is_open()) {
        fs.close();
    }
    if (stdfs::exists(tmp_filename)) {
        stdfs::remove(tmp_filename);
    }
}

void merge_chunk::write(sorted_rows &r) {
    if (num_check_pending) {
        check_num_pos(r[0]);
        num_check_pending = false;
    }
    std::string out_buffer;
    for (auto &row : r) {
        for (const auto &num_item : num_pos) {
            models::to_str(row.m_row[num_item]);
        }
        row.m_row.emplace_back(std::to_string(row.orig_chunk_id));
        models::append_to_string(out_buffer, row.m_row);
    }
    fs.write(out_buffer.c_str(), out_buffer.size());
}

void merge_chunk::finish_writing() {
    fs.flush();
    fs.seekg(0);
}

bool merge_chunk::empty() {
    if (src == mem) {
        return curr_pos >= curr_chunk.size();
    }
    if (curr_pos >= curr_chunk.size()) {
        curr_chunk.clear();
        if (!fut_chunk.has_value()) {
            std::promise<sorted_rows> promise;
            fut_chunk = std::move(promise.get_future());
            fetch_task_queue->get().enqueue(
                {std::ref(fs), args.chunk_size, std::move(promise), num_pos});
        }
        curr_chunk = std::move(fut_chunk->get());
        if (curr_chunk.empty()) {
            return true;
        }
        curr_pos = 0;
        std::promise<sorted_rows> promise;
        fut_chunk = std::move(promise.get_future());
        fetch_task_queue->get().enqueue(
            {std::ref(fs), args.chunk_size, std::move(promise), num_pos});
    }
    return false;
}

merge_row merge_chunk::next_row() {
    curr_chunk[curr_pos].curr_chunk_id = curr_chunk_id;
    return std::move(curr_chunk[curr_pos++]);
}

} // namespace engine
