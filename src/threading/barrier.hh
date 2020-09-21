#ifndef CSVM_BARRIER_HH
#define CSVM_BARRIER_HH

#include "locks.hh"
#include <atomic>
#include <condition_variable>

namespace threading {
class barrier {
    fake_lock flock;
    std::atomic<int> expected{};
    std::condition_variable_any cv;

  public:
    inline void expect(int e) { expected = e; }

    inline void arrive() {
        --expected;
        cv.notify_all();
    }

    inline void wait() {
        cv.wait(flock, [&]() { return expected <= 0; });
    }

    inline void arrive_and_wait() {
        arrive();
        wait();
    }
};
} // namespace threading

#endif /* CSVM_BARRIER_HH */
