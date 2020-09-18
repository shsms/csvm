#include "../cli_args.hh"
#include "expr.hh"
#include "stmt.hh"
#include <algorithm>
#include <stack>

namespace engine {

using namespace std::string_literals;

class selectstmt : public expr::expr {
  public:
    selectstmt(const cli_args &args) : expr(args) {}

    std::string string() override {
	return "select:\n\t" + expr::string();
    }
};

} // namespace engine
