/*
  Copyright 2013 Statoil ASA.

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


#ifndef ActionX_HPP_
#define ActionX_HPP_

#include <string>
#include <vector>
#include <ctime>

#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionAST.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionResult.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/Condition.hpp>



namespace Opm {
class DeckKeyword;

namespace Action {
class State;

/*
  The ActionX class internalizes the ACTIONX keyword. This keyword represents a
  small in-deck programming language for the SCHEDULE section. In the deck the
  ACTIONX keyword comes together with a 'ENDACTIO' kewyord and then a list of
  regular keywords in the between. The principle is then that ACTIONX represents
  a condition, and when that condition is satisfied the keywords are applied. In
  the example below the ACTIONX keyword defines a condition whether well OPX has
  watercut above 0.75, when the condition is met the WELOPEN keyword is applied
  - and the well is shut.

  ACTIONX
     'NAME'  /
     WWCT OPX > 0.50 /
  /

  WELOPEN
     'OPX'  OPEN /
  /

  ENDACTION


*/

class ActionX {
public:
    ActionX();
    ActionX(const std::string& name, size_t max_run, double max_wait, std::time_t start_time);
    ActionX(const DeckKeyword& kw, std::time_t start_time);
    ActionX(const DeckRecord& record, std::time_t start_time);

    static ActionX serializeObject();

    void addKeyword(const DeckKeyword& kw);
    bool ready(const State& state, std::time_t sim_time) const;
    Action::Result eval(const Action::Context& context) const;


    std::string name() const { return this->m_name; }
    size_t max_run() const { return this->m_max_run; }
    double min_wait() const { return this->m_min_wait; }
    std::size_t id() const;
    void update_id(std::size_t id);
    std::time_t start_time() const { return this->m_start_time; }
    std::vector<DeckKeyword>::const_iterator begin() const;
    std::vector<DeckKeyword>::const_iterator end() const;
    static bool valid_keyword(const std::string& keyword);

    /*
      The conditions() and keyword_strings() methods, and their underlying data
      members are only present to support writing formatted restart files.
    */
    const std::vector<Condition>& conditions() const;
    std::vector<std::string> keyword_strings() const;

    bool operator==(const ActionX& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_name);
        serializer(m_max_run);
        serializer(m_min_wait);
        serializer(m_start_time);
        serializer(m_id);
        serializer.vector(keywords);
        condition.serializeOp(serializer);
        serializer.vector(m_conditions);
    }

private:
    std::string m_name;
    size_t m_max_run = 0;
    double m_min_wait = 0.0;
    std::time_t m_start_time;
    std::size_t m_id = 0;

    std::vector<DeckKeyword> keywords;
    Action::AST condition;
    std::vector<Condition> m_conditions;
};

}
}
#endif /* WELL_HPP_ */
