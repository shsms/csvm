#ifndef CSVM_MODELS_H
#define CSVM_MODELS_H

#include <any>
#include <fmt/format.h>
#include <functional>
#include <string>
#include <typeindex>
#include <variant>
#include <vector>

namespace models {

using value = std::variant<std::string, double, bool>;

using row = std::vector<value>;

struct raw_chunk {
    int id;
    std::string data;
};

struct bin_chunk {
    int id;
    std::vector<row> data;
    int length;
};

struct col_header {
    std::string name;
    // TODO: add rename/alias info
};

using header_row = std::vector<col_header>;

struct bool_resp {
    bool bool_v;
    bool is_bool;
};

inline bool string_equal(const std::string &a, const value &b) {
    return a == std::get<std::string>(b);
}

inline bool value_equal(const value &a, const value &b) { return a == b; }

inline bool value_lt(const value &a, const value &b) { return a < b; }

inline bool value_gt(const value &a, const value &b) { return a > b; }

inline void to_num(value &a) {
    double vv = 0.0;
    auto str = std::get<std::string>(a);
    if (!str.empty()) {
        try {
            vv = std::stod(str);
        } catch (std::invalid_argument &) {
            throw std::invalid_argument(std::string("non-numeric value '") +
                                        str + "'");
        }
    }
    a = vv;
}

inline void to_str(value &a) {
    auto str = std::to_string(std::get<double>(a));
    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
    str.erase(str.find_last_not_of('.') + 1, std::string::npos);
    a = std::move(str);
}

inline void append_to_string(std::string &ret, const row &row) {
    static const std::string comma_str = ",";
    static const std::string newline = "\n";
    for (auto ii = 0; ii < row.size(); ii++) {
        if (ii == 0) {
            ret += std::get<std::string>(row[ii]);
        } else {
            ret += comma_str + std::get<std::string>(row[ii]);
        }
    }
    ret += newline;
}

} // namespace models

#endif
