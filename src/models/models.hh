#ifndef CSVQ_MODELS_H
#define CSVQ_MODELS_H

#include <any>
#include <fmt/format.h>
#include <functional>
#include <string>
#include <typeindex>
#include <vector>

namespace models {
enum value_t { double_t, string_t };
struct value {
    value_t type;
    std::string string_v;
    double double_v;
};
using row = std::vector<value>;

inline void print(const char *fmt, const value &a) {
    if (a.type == string_t)
        fmt::print(fmt, a.string_v);
    else {
        fmt::print(fmt, a.double_v);
    }
}

inline bool string_equal(const std::string &a, const value &b) {
    return a == b.string_v;
}
} // namespace models

#endif