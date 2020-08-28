#ifndef CSVQ_ORDER_HH
#define CSVQ_ORDER_HH

#include <atomic>
#include <condition_variable>

class ordering_lock {
    std::atomic<int> order{0};
    std::condition_variable_any order_cond;

    std::atomic<int> thread_lim{0};
    std::condition_variable_any thread_lim_cond;

    struct fake_lock {
        void lock() {}
        void unlock() {}
    };

    fake_lock flock;

  public:
    void lock(int pos) {
        auto vv = order.load(std::memory_order_acquire);
        if (pos == vv)
            return;
        order_cond.wait(flock, [&]() {
            vv = order.load(std::memory_order_acquire);
            return pos == vv;
        });
    }

    void unlock() {
        order.fetch_add(1, std::memory_order_release);
        order_cond.notify_all();
    }

    void t_start() {
        static const int max_threads = 4;
        auto vv = thread_lim.load(std::memory_order_acquire);
        if (vv >= max_threads) {
            thread_lim_cond.wait(flock, [&]() {
                vv = thread_lim.load(std::memory_order_acquire);
                return vv < max_threads;
            });
        }
        thread_lim.fetch_add(1, std::memory_order_acquire);
    }

    void t_end() {
        thread_lim.fetch_sub(1, std::memory_order_release);
        thread_lim_cond.notify_one();
    }
};

#endif
