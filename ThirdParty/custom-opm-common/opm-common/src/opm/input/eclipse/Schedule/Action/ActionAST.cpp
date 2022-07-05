/*
  Copyright 2018 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdlib>

#include <opm/input/eclipse/Schedule/Action/ActionAST.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionContext.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionValue.hpp>
#include <opm/input/eclipse/Schedule/Action/ASTNode.hpp>

#include "ActionParser.hpp"

namespace Opm {
namespace Action {

AST::AST(const std::vector<std::string>& tokens) {
    auto condition_node = Action::Parser::parse(tokens);
    this->condition.reset( new Action::ASTNode(condition_node) );
}

AST AST::serializeObject()
{
    AST result;
    result.condition = std::make_shared<ASTNode>(ASTNode::serializeObject());

    return result;
}

Action::Result AST::eval(const Action::Context& context) const {
    if (!this->condition || this->condition->empty())
        return Action::Result(false);
    else
        return this->condition->eval(context);
}


bool AST::operator==(const AST& data) const {
    if ((condition && !data.condition) ||
        (!condition && data.condition))
        return false;

    return !condition || (*condition == *data.condition);
}

void AST::required_summary(std::unordered_set<std::string>& required_summary) const {
    this->condition->required_summary(required_summary);
}


}
}
