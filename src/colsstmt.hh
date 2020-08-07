#include "stmt.hh"
#include <algorithm>

namespace engine {
class colsstmt : public stmt {
  private:
    std::vector<std::string> columns;
    bool exclude;

  public:
    colsstmt() {
	exclude = false;
    }
    
    virtual ~colsstmt() {};
    
    void add_ident(const std::string &col) { columns.push_back(col); }

    std::string string() {
	std::string excl = "keep:";
	if (exclude == true) excl = "exclude:";
        std::string ret = "cols:" + excl + "\n";
        std::ranges::for_each(columns,
                              [&ret](auto &a) { ret += "\t" + a + "\n"; });
        return ret;
    }

    void add_bang() {
	exclude = true;
    }
};
} // namespace engine
