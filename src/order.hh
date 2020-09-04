#ifndef CSVM_ORDER_HH
#define CSVM_ORDER_HH

#include "locks.hh"
#include <atomic>
#include <chrono>
#include <condition_variable>

namespace threading {

using namespace std::chrono_literals;

class ordering_lock {
    std::atomic<int> order{0};
    std::condition_variable_any order_cond;

    fake_lock flock;

  public:
    void lock(int pos) {
        auto vv = order.load(std::memory_order_relaxed);
        if (pos == vv) {
            return;
        }
        // using wait_for 5ms here, because sometimes, the
        // order_cond.notify_all() call in the unlock function
        // fails to wake up a thread and the whole thing dead-locks.
        while (!order_cond.wait_for(flock, 5ms, [&]() {
            vv = order.load(std::memory_order_relaxed);
            return pos == vv;
        })) {
            ;
        }
    }

    void unlock() {
        order.fetch_add(1, std::memory_order_relaxed);
        order_cond.notify_all();
    }
};
} // namespace threading
#endif
