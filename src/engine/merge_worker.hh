#ifndef CSVM_MERGESTMT_HH
#define CSVM_MERGESTMT_HH

#include "../cli_args.hh"
#include "sortstmt.hh"
#include <future>

namespace engine {

class merge_worker {
  private:
    int curr_pos{};
    std::vector<sortspec> columns;
    int thread_count{1};

    struct merge_task {
        std::vector<merge_chunk> chunks;
        std::promise<merge_chunk> promise;
    };

    std::vector<std::thread> additional_workers;
    threading::queue<merge_task> task_queue;

    template <typename Collector>
    void merge_and_collect(std::vector<merge_chunk> chunks, Collector collector);
    const cli_args args;

  public:
    merge_worker(const std::vector<sortspec> &cols, const cli_args &args)
        : columns(cols), args(args) {}

    bool run(threading::queue<merge_chunk> &in_queue, threading::bin_queue &merged);
};

} // namespace engine

#endif /* CSVM_MERGESTMT_HH */
