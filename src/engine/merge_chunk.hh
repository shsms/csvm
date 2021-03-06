#ifndef CSVM_MERGE_CHUNK_HH
#define CSVM_MERGE_CHUNK_HH

#include "../cli_args.hh"
#include "../csv/csv.hh"
#include "../input.hh"
#include "../models/models.hh"
#include <atomic>
#include <filesystem>
#include <fstream>
#include <future>

namespace engine {

struct merge_row {
    int orig_chunk_id; // for stable sorting
    int curr_chunk_id; // to pick next row from, while merging.
    models::row m_row;
};

using sorted_rows = std::vector<merge_row>;

struct fetch_file_chunk_task {
    std::reference_wrapper<std::fstream> fs;
    double max_chunk_size;
    std::promise<sorted_rows> promise;
    std::vector<int> num_pos;
};

class merge_chunk {
    enum { mem, disk } src{mem};
    sorted_rows curr_chunk;

    std::string tmp_filename;
    std::fstream fs;
    int curr_pos{},      // curr_pos in curr_block.
        curr_chunk_id{}; // to pick next row from, while merging.  known only at
                         // time of merging.
    static std::atomic<int> filenum;
    bool num_check_pending{true};
    std::vector<int> num_pos;

    void check_num_pos(const merge_row &mr);
    cli_args args;
    std::optional<std::future<sorted_rows>> fut_chunk;

    std::optional<std::reference_wrapper<threading::queue<fetch_file_chunk_task>>> fetch_task_queue;

  public:
    // constructor for in-mem mode.
    merge_chunk(sorted_rows &&r) noexcept : curr_chunk(std::move(r)) {}

    // constructor for tmp-file mode.
    merge_chunk(const cli_args &args, threading::queue<fetch_file_chunk_task> &fetch_task_queue);
    ~merge_chunk();

    merge_chunk(const merge_chunk &r) = delete;
    merge_chunk &operator=(const merge_chunk &r) = delete;

    merge_chunk(merge_chunk &&r) noexcept = default;
    merge_chunk &operator=(merge_chunk &&r) noexcept = default;

    inline void set_curr_chunk_id(int c) { curr_chunk_id = c; }

    void write(sorted_rows &r);
    void finish_writing();
    bool empty();
    merge_row next_row();
};

} // namespace engine
#endif /* CSVM_MERGE_CHUNK_HH */
