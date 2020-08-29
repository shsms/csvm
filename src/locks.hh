#ifndef CSVQ_FAKE_LOCK_HH
#define CSVQ_FAKE_LOCK_HH

#include <atomic>
#include <chrono>
#include <thread>

namespace threading {

using namespace std::chrono_literals;

struct fake_lock {
    void lock() {}
    void unlock() {}
};

class spin_lock {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;

  public:
    void lock() {
        while (flag.test_and_set(std::memory_order_acquire)) {
            std::this_thread::sleep_for(1000ns);
        }
    }

    void unlock() { flag.clear(std::memory_order_release); }
};

} // namespace threading

#endif /* CSVQ_FAKE_LOCK_HH */
