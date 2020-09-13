#ifndef CSVM_MERGE_CHUNK_HH
#define CSVM_MERGE_CHUNK_HH

#include "../models/models.hh"
#include "../input.hh"
#include "../csv/csv.hh"
#include <atomic>
#include <filesystem>
#include <fstream>

namespace engine {
struct merge_row {
    int orig_chunk_id; // for stable sorting
    int curr_chunk_id; // to pick next row from, while merging.
    models::row m_row;
};

namespace stdfs = std::filesystem;

using sorted_rows = std::vector<merge_row>;

class merge_chunk {
    enum { mem, disk } src{mem};
    sorted_rows curr_chunk;

    std::string filename;
    std::fstream fs;
    int curr_pos{},      // curr_pos in curr_block.
        curr_chunk_id{}; // to pick next row from, while merging.  known only at
                         // time of merging.
    static std::atomic<int> filenum;
    bool num_check_pending{true};
    std::vector<int> num_pos;

    void check_num_pos(const merge_row &mr) {
	for(int ii = 0; ii < mr.m_row.size(); ++ii) {
	    if (std::holds_alternative<double>(mr.m_row[ii])) {
		num_pos.push_back(ii);
	    }
	}
    }

  public:
    merge_chunk(merge_chunk &&r) = default;
    merge_chunk &operator=(merge_chunk &&r) = default;

    merge_chunk(sorted_rows &&r) : curr_chunk(std::move(r)) {}

    merge_chunk() {
        src = disk;
        auto fnum = filenum.fetch_add(1);
        filename = std::string("tq.csv.") + std::to_string(fnum) + "~";
        stdfs::path dir = stdfs::temp_directory_path();
        filename = dir / filename;
        fs.open(filename, std::ios::trunc | std::ios::in | std::ios::out | std::ios::binary);
    }

    ~merge_chunk() {
        if (fs.is_open()) {
            fs.close();
        }
        if (stdfs::exists(filename) && curr_pos == 0) {
            stdfs::remove(filename);
        }
    }

    void write(sorted_rows &r) {
	if (num_check_pending) {
	    check_num_pos(r[0]);
	    num_check_pending = false;
	}
        std::string out_buffer;
        for (auto &row : r) {
	    for(const auto &num_item : num_pos) {
		models::to_str(row.m_row[num_item]);
	    }
            row.m_row.emplace_back(std::to_string(row.orig_chunk_id));
            models::append_to_string(out_buffer, row.m_row);
        }
        fs.write(out_buffer.c_str(), out_buffer.size());
    }

    void finish_writing() {
        // fs.flush();
        fs.seekg(0);
    }

    void set_curr_chunk_id(int c) { curr_chunk_id = c; }

    bool empty() {
        if (src == mem) {
            if (curr_pos >= curr_chunk.size()) {
                return true;
            }
            return false;
        }
        if (curr_pos >= curr_chunk.size()) {
            if (fs.eof()) {
                return true;
            }
            curr_chunk.clear();
            std::string raw = next_chunk(fs, 1e6);
            if (raw.empty()) {
                return true;
            }

            csv::parse_body(models::raw_chunk{curr_chunk_id, raw}, [this](models::row &row) {
                auto orig_id_str = std::get<std::string>(row.back());
                auto orig_id = std::stoi(orig_id_str);
                row.pop_back();
                if (row.empty()) {
                    return;
                }
		for (const auto &num_item : num_pos) {
		    models::to_num(row[num_item]);
		}
                this->curr_chunk.emplace_back(merge_row{orig_id, 0, row});
            });
            curr_pos = 0;
        }
        return false;
    }

    // must do empty() check before calling next_row.
    merge_row next_row() {
        curr_chunk[curr_pos].curr_chunk_id = curr_chunk_id;
        return std::move(curr_chunk[curr_pos++]);
    }
};

}
#endif /* CSVM_MERGE_CHUNK_HH */
