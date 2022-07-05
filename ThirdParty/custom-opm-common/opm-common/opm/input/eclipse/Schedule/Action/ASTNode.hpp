#ifndef ASTNODE_HPP
#define ASTNODE_HPP

#include <unordered_set>

#include <opm/input/eclipse/Schedule/Action/ActionContext.hpp>

#include "ActionValue.hpp"

namespace Opm {
namespace Action {

class ActionContext;
class WellSet;
class ASTNode {
public:

    ASTNode();
    ASTNode(TokenType type_arg);
    ASTNode(double value);
    ASTNode(TokenType type_arg, FuncType func_type_arg, const std::string& func_arg, const std::vector<std::string>& arg_list_arg);

    static ASTNode serializeObject();

    Action::Result eval(const Action::Context& context) const;
    Action::Value value(const Action::Context& context) const;
    TokenType type;
    FuncType func_type;
    void add_child(const ASTNode& child);
    size_t size() const;
    bool empty() const;

    std::string func;
    void required_summary(std::unordered_set<std::string>& required_summary) const;

    bool operator==(const ASTNode& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(type);
        serializer(func_type);
        serializer(func);
        serializer(arg_list);
        serializer(number);
        serializer.vector(children);
    }

private:
    std::vector<std::string> arg_list;
    double number = 0.0;

    /*
      To have a member std::vector<ASTNode> inside the ASTNode class is
      supposedly borderline undefined behaviour; it compiles without warnings
      and works. Good for enough for me.
    */
    std::vector<ASTNode> children;
};
}
}
#endif
