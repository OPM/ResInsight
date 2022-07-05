#ifndef ACTION_VALUE_HPP
#define ACTION_VALUE_HPP

#include <opm/input/eclipse/Schedule/Action/ActionResult.hpp>

enum TokenType {
  number,        //  0
  ecl_expr,      //  1
  open_paren,    //  2
  close_paren,   //  3
  op_gt,         //  4
  op_ge,         //  5
  op_lt,         //  6
  op_le,         //  7
  op_eq,         //  8
  op_ne,         //  9
  op_and,        // 10
  op_or,         // 11
  end,           // 12
  error          // 13
};

enum class FuncType {
  none,
  time,
  time_month,
  region,
  field,
  group,
  well,
  well_segment,
  well_connection,
  Well_lgr,
  aquifer,
  block
};



namespace Opm {
namespace Action {

class Value {
public:
    explicit Value(double value);
    Value(const std::string& wname, double value);
    Value() = default;

    Result eval_cmp(TokenType op, const Value& rhs) const;
    void add_well(const std::string& well, double value);
    double scalar() const;

private:
    Action::Result eval_cmp_wells(TokenType op, double rhs) const;

    double scalar_value;
    double is_scalar = false;
    std::vector<std::pair<std::string, double>> well_values;
};


}
}
#endif
