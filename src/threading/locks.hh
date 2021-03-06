#ifndef CSVM_FAKE_LOCK_HH
#define CSVM_FAKE_LOCK_HH

#include <atomic>
#include <chrono>
#include <thread>

namespace threading {

using namespace std::chrono_literals;

struct fake_lock {
    inline void lock() {}
    inline void unlock() {}
};

class spin_lock {
    std::atomic_flag flag = ATOMIC_FLAG_INIT;

  public:
    inline void lock() {
        while (flag.test_and_set(std::memory_order_acquire)) {
            std::this_thread::sleep_for(100us);
        }
    }

    inline void unlock() { flag.clear(std::memory_order_release); }
};

} // namespace threading

#endif /* CSVM_FAKE_LOCK_HH */
