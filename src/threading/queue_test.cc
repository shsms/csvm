#include <catch2/catch.hpp>
#include "queue.hh"
#include <chrono>

using namespace std::chrono_literals;

TEST_CASE("thread-safe queue", "[threading::queue]") {
    threading::queue<int> t;
    threading::queue<int> q(std::move(t));

    static const int limit = 10;
    static const int num_items = 1000;
    const int sum = num_items * (num_items - 1) / 2;

    q.set_limit(limit);
    
    std::thread t0([&q](){
	std::this_thread::sleep_for(10ms);	
	for (int ii = 0; ii < num_items; ii ++) {
	    q.enqueue(std::move(ii));
	}
    });

    auto vv = q.dequeue();
    REQUIRE(vv.value() == 0);

    std::this_thread::sleep_for(100ms);

    REQUIRE(q.size() == limit);

    int t1_sum{};
    std::thread t1([&q, &t1_sum]() {
	auto item = q.dequeue();
	while(item.has_value()) {
	    t1_sum+=item.value();
	    item = q.dequeue();
	    std::this_thread::sleep_for(1us);
	}
    });

    int t2_sum{};
    std::thread t2([&q, &t2_sum]() {
	auto item = q.dequeue();
	while(item.has_value()) {
	    t2_sum+=item.value();
	    item = q.dequeue();
	    std::this_thread::sleep_for(1us);
	}
    });

    t0.join();

    q.set_eof();
    
    t1.join();
    t2.join();

    REQUIRE(q.size() == 0);
    REQUIRE(0 < t1_sum);
    REQUIRE(t1_sum < sum);
    REQUIRE(t1_sum + t2_sum == sum);
}
