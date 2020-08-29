#ifndef CSVQ_QUEUE_HH
#define CSVQ_QUEUE_HH

#include "locks.hh"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <optional>
#include <queue>
#include <string>

namespace threading {
struct chunk {
    int id;
    std::string data;
};

using namespace std::chrono_literals;

class queue {
    int limit = 10;
    std::queue<chunk> q;
    std::atomic<int> size{0};
    std::atomic<bool> eof{false};
    std::condition_variable_any enq_cond, deq_cond;

    spin_lock mu;

  public:
    queue(int lim) : limit(lim) {}

    void enqueue(chunk &&item) {
        std::unique_lock lock(mu);
        if (size >= limit) {
            while (
                !enq_cond.wait_for(lock, 10ms, [&]() { return size < limit; }))
                ;
        }
        q.emplace(std::move(item));
        size++;
        deq_cond.notify_one();
    }

    void set_eof() {
        eof = true;
        deq_cond.notify_all();
    }

    std::optional<chunk> dequeue() {
        std::unique_lock lock(mu);
        if (q.empty() && !eof)
            while (!deq_cond.wait_for(lock, 10ms,
                                      [&]() { return eof || !q.empty(); }))
                ;
        if (!q.empty()) {
            auto ret = std::move(q.front());
            q.pop();
            size--;
            enq_cond.notify_one();
            return ret;
        }
        return {};
    }
};
} // namespace threading
#endif /* CSVQ_QUEUE_HH */
