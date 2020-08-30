#ifndef CSVQ_PRINT_HH
#define CSVQ_PRINT_HH

#include "queue.hh"

#include <iostream>
#include <string>
#include <unordered_map>

namespace threading {

inline void print_worker(threading::queue &queue) {
    int next = 0;
    std::unordered_map<int, std::string> items;

    auto chunk = queue.dequeue();
    while (chunk.has_value()) {
        if (chunk->id == next) {
            std::cout << chunk->data;
            ++next;
            while (items.count(next) > 0) {
                std::cout << items.at(next);
                // TODO:  reusing strings helps avoid reallocation
                // probably has an impact only in super large files.
                items.erase(next);
                ++next;
            }
        } else {
            items.emplace(chunk->id, std::move(chunk->data));
        }
        chunk = queue.dequeue();
    }
}

} // namespace threading
#endif /* CSVQ_PRINT_HH */
