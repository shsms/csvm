#ifndef CSVM_QUEUE_HH
#define CSVM_QUEUE_HH

#include "locks.hh"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <optional>
#include <queue>
#include <string>
#include "models/models.hh"
namespace threading {
using namespace std::chrono_literals;

template <typename T> class queue;

using raw_queue = queue<models::raw_chunk>;
using bin_queue = queue<models::bin_chunk>;

template <typename T> class queue {
    int limit = 10;
    std::queue<T> q;
    std::atomic<std::size_t> q_size{0};
    std::atomic<bool> eof{false};
    std::condition_variable_any enq_cond, deq_cond;

    spin_lock spin_mu;

  public:
    queue() {}
    // atomics don't have move constructors,  so had to fake it.
    queue(queue &&other)
        : limit(other.limit), q(std::move(other.q)),
          q_size(other.q_size.load()), eof(other.eof.load()) {
        q_size = 0;
        eof = false;
    }
    void set_limit(int lim) { limit = lim; }

    auto enqueue(T &&item) {
        std::unique_lock lock(spin_mu);
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

    std::optional<T> dequeue() {
        std::unique_lock lock(spin_mu);
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
#endif /* CSVM_QUEUE_HH */
