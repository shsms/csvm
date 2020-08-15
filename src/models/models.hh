#ifndef CSVQ_MODELS_H
#define CSVQ_MODELS_H

#include <any>
#include <fmt/format.h>
#include <functional>
#include <string>
#include <typeindex>
#include <vector>

namespace models {
    enum value_t { double_t, string_t, bool_t };
struct value {
    value_t type;
    std::string string_v;
    double double_v;
    bool bool_v;
};

    struct bool_resp {
	bool bool_v;
	bool is_bool;
    }; 
    
inline bool_resp get_bool_value(const value&v) {
    return bool_resp{.bool_v=v.bool_v,.is_bool=v.type==bool_t};
}

inline value bool_value(bool v) {
    return value{.type = bool_t, .bool_v = v};
}

using row = std::vector<value>;

inline bool string_equal(const std::string &a, const value &b) {
    return a == b.string_v;
}

inline bool value_equal(const value&a, const value&b) {
    switch(a.type) {
    case double_t:
	return a.double_v == b.double_v;
    case bool_t:
	return a.bool_v == b.bool_v;
    default:
	return a.string_v == b.string_v;
    }
}

inline bool value_lt(const value&a, const value&b) {
    switch(a.type) {
    case double_t:
	return a.double_v < b.double_v;
    default:
	return a.string_v < b.string_v;
    }
}

inline bool value_gt(const value&a, const value&b) {
    switch(a.type) {
    case double_t:
	return a.double_v > b.double_v;
    default:
	return a.string_v > b.string_v;
    }
}

} // namespace models

#endif
