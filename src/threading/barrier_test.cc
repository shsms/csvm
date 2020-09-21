#include <catch2/catch.hpp>
#include "barrier.hh"

using namespace std::chrono_literals;

TEST_CASE("barrier should sync all threads", "[threading::barrier]") {
    threading::barrier b;
    b.expect(3);
    
    std::atomic<int> c{};

    const auto l = [&c, &b](){
	std::this_thread::sleep_for(1s);
	c+=1;
	b.arrive();
    };
    std::thread t0(l);
    std::thread t1(l);
    std::thread t2(l);
    
    REQUIRE(c == 0);
    b.wait();
    REQUIRE(c == 3);

    t0.join();
    t1.join();
    t2.join();
}
