#include "../cli_args.hh"
#include "expr.hh"
#include "stmt.hh"
#include <algorithm>
#include <stack>

namespace engine {

class selectstmt : public expr::expr {
  public:
    selectstmt(const cli_args &args) : expr(args) {}
};

} // namespace engine
