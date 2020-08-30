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
    int limit;
    std::queue<chunk> q;
    std::atomic<std::size_t> q_size{0};
    std::atomic<bool> eof{false};
    std::condition_variable_any enq_cond, deq_cond;

    spin_lock mu;

  public:
    queue(int lim) : limit(lim) {}

    void enqueue(chunk &&item) {
        std::unique_lock lock(mu);
        if (q_size >= limit) {
            while (!enq_cond.wait_for(lock, 10ms,
                                      [&]() { return q_size < limit; })) {
                ;
            }
        }
        q.emplace(std::move(item));
        q_size++;
        deq_cond.notify_one();
    }

    void set_eof() {
        eof = true;
        deq_cond.notify_all();
    }

    std::optional<chunk> dequeue() {
        std::unique_lock lock(mu);
        if (q.empty() && !eof) {
            while (!deq_cond.wait_for(lock, 10ms,
                                      [&]() { return eof || !q.empty(); })) {
                ;
            }
        }
        if (!q.empty()) {
            auto ret = std::move(q.front());
            q.pop();
            q_size--;
            enq_cond.notify_one();
            return ret;
        }
        return {};
    }

    std::size_t size() const { return q_size; }
};
} // namespace threading
#endif /* CSVQ_QUEUE_HH */
