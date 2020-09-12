#ifndef CSVM_SORTSTMT_HH
#define CSVM_SORTSTMT_HH

#include "../threading/barrier.hh"
#include "stmt.hh"
#include <algorithm>
#include <fmt/format.h>
#include <stack>
#include <stdexcept>

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>
#include <filesystem>
#include <fstream>

namespace engine {

namespace stdfs = std::filesystem;

struct sortspec {
    bool descending;
    bool numeric;
    int pos;
    std::string name;
};

struct merge_row {
    int orig_chunk_id; // for stable sorting
    int curr_chunk_id; // to pick next row from, while merging.
    models::row m_row;

    template <typename Archive> void serialize(Archive &ar) {
        ar(orig_chunk_id, curr_chunk_id, m_row);
    }
};

using sorted_rows = std::vector<merge_row>;

class merge_chunk {
    enum { mem, disk } src{mem};
    sorted_rows curr_chunk;

    std::string filename;
    std::fstream fs;
    int total_disk_chunks{}, disk_chunk_num{},
        curr_pos{},      // curr_pos in curr_block.
        curr_chunk_id{}; // to pick next row from, while merging.  known only at
                         // time of merging.
    static std::atomic<int> filenum;
    std::unique_ptr<cereal::BinaryOutputArchive> ar_save;
    std::unique_ptr<cereal::BinaryInputArchive> ar_load;

  public:
    merge_chunk(merge_chunk &&r) = default;
    merge_chunk& operator=(merge_chunk &&r) = default;
    
    merge_chunk(sorted_rows &&r) : curr_chunk(std::move(r)) {}

    merge_chunk() {
        src = disk;
        auto fnum = filenum.fetch_add(1);
        filename = std::string("tq.csv.") + std::to_string(fnum) + "~";
        stdfs::path dir = stdfs::temp_directory_path();
        filename = dir / filename;
        fs = std::fstream(filename, std::ios::binary);
        ar_save = std::make_unique<cereal::BinaryOutputArchive>(fs);
    }

    ~merge_chunk() {
        if (fs.is_open()) {
            fs.flush();
            fs.close();
        }
        if (stdfs::exists(filename)) {
            stdfs::remove(filename);
        }
    }

    void write(sorted_rows &r) {
        (*ar_save)(r);
        ++total_disk_chunks;
    }

    void finish_writing() {
        ar_save = nullptr; // destroy output archive, forcing it to flush.
        fs.seekg(0);
        ar_load = std::make_unique<cereal::BinaryInputArchive>(fs);
    }

    void set_curr_chunk_id(int c) { curr_chunk_id = c; }

    bool empty() {
	if(src==mem) {
	    if (curr_pos >= curr_chunk.size()) {
		return true;
	    }
	    return false;
	}
	if (curr_pos >= curr_chunk.size()) {
	    if (disk_chunk_num >= total_disk_chunks) {
		return true;
	    }
	    (*ar_load)(curr_chunk);
	    disk_chunk_num++;
	    curr_pos = 0;
	}
	return false;
    }

    // must do empty() check before calling next_row.
    merge_row &&next_row() {
	curr_chunk[curr_pos].curr_chunk_id = curr_chunk_id;
	return std::move(curr_chunk[curr_pos++]);
    }
};

class sortstmt : public stmt {
  private:
    int curr_pos{};
    std::vector<sortspec> columns;

    std::atomic<bool> merge_thread_created{false};
    std::thread merge_thread;
    threading::queue<merge_chunk> to_merge;
    threading::barrier barrier;

  public:
    void add_ident(const std::string &col) override;
    void add_oper(const std::string &oper) override;
    std::string string() override;

    inline stmt::exec_order finalize() override { return sep_block; }

    void set_header(models::header_row & /*unused*/) override;
    void set_thread_count(int /*unused*/) override;
    bool apply(models::bin_chunk & /*chunk*/,
               std::stack<models::value> & /*eval_stack*/) override;
    bool run_worker(
        threading::bin_queue & /*in_queue*/,
        const std::function<void(models::bin_chunk &)> & /*unused*/) override;
};

} // namespace engine

#endif /* CSVM_SORTSTMT_HH */
