/*
  Copyright 2018 Statoil ASA.

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

#include <optional>
#include <sstream>
#include <unordered_set>
#include <fmt/format.h>

#include <opm/input/eclipse/Utility/Typetools.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckOutput.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionValue.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionX.hpp>
#include <opm/input/eclipse/Schedule/Action/Actdims.hpp>
#include <opm/input/eclipse/Schedule/Action/State.hpp>
#include <opm/input/eclipse/Schedule/Well/WellMatcher.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/W.hpp>
#include <opm/io/eclipse/rst/action.hpp>
#include <opm/common/utility/OpmInputError.hpp>

#include "ActionParser.hpp"

namespace Opm {
namespace Action {
namespace {

std::string dequote(const std::string& token, const std::optional<KeywordLocation>& location) {
    if (token[0] == '\'') {
        if (token.back() == '\'')
            return token.substr(1, token.size() - 2);
        else {
            auto msg = fmt::format("Unbalanced quote for token: {}", token);
            if (location.has_value())
                throw OpmInputError(msg, location.value());
            else
                throw std::logic_error(msg);
        }
    } else
        return token;
}

}


bool ActionX::valid_keyword(const std::string& keyword) {
    static std::unordered_set<std::string> actionx_allowed_list = {
        "BOX",
        "COMPLUMP", "COMPDAT", "COMPSEGS",
        "ENDBOX", "EXIT",
        "GCONINJE", "GCONPROD", "GCONSUMP", "GLIFTOPT", "GRUPNET", "GRUPTARG", "GRUPTREE", "GSATINJE", "GSATPROD",
        "MULTX", "MULTX-", "MULTY", "MULTY-", "MULTZ", "MULTZ-",
        "NEXT", "NEXTSTEP",
        "UDQ",
        "WCONINJE", "WCONPROD", "WECON", "WEFAC", "WELOPEN", "WELPI", "WELTARG", "WGRUPCON", "WPIMULT", "WELSEGS", "WELSPECS", "WSEGVALV", "WTEST", "WTMULT",
        "TEST"
    };
    return (actionx_allowed_list.find(keyword) != actionx_allowed_list.end());
}


ActionX::ActionX() :
    m_start_time(0)
{
}


ActionX::ActionX(const std::string& name, size_t max_run, double min_wait, std::time_t start_time) :
    m_name(name),
    m_max_run(max_run),
    m_min_wait(min_wait),
    m_start_time(start_time)
{}

ActionX::ActionX(const RestartIO::RstAction& rst_action)
    : m_name(rst_action.name)
    , m_max_run(rst_action.max_run)
    , m_min_wait(rst_action.min_wait),
      m_start_time(rst_action.start_time)
{
    std::vector<std::string> tokens;
    for (const auto& rst_condition : rst_action.conditions) {
        this->m_conditions.emplace_back(rst_condition);

        for (const auto& token : rst_condition.tokens())
            tokens.push_back(dequote(token, {}));
    }
    this->condition = Action::AST(tokens);
    for (const auto& keyword : rst_action.keywords)
        this->addKeyword(keyword);
}



ActionX::ActionX(const DeckRecord& record, std::time_t start_time) :
    ActionX( record.getItem("NAME").getTrimmedString(0),
             record.getItem("NUM").get<int>(0),
             record.getItem("MIN_WAIT").getSIDouble(0),
             start_time )

{}





ActionX::ActionX(const DeckKeyword& kw, const Actdims& actdims, std::time_t start_time) :
    ActionX(kw.getRecord(0), start_time)
{
    std::vector<std::string> tokens;
    for (size_t record_index = 1; record_index < kw.size(); record_index++) {
        const auto& record = kw.getRecord(record_index);
        const auto& cond_tokens = RawString::strings( record.getItem("CONDITION").getData<RawString>() );

        for (const auto& token : cond_tokens)
            tokens.push_back(dequote(token, kw.location()));

        this->m_conditions.emplace_back(cond_tokens, kw.location());
    }
    if (this->m_conditions.size() > actdims.max_conditions())
        throw OpmInputError(fmt::format("Action {} has too many conditions - adjust item 4 of ACTDIMS to at least {}", this->name(), this->m_conditions.size()), kw.location());

    this->condition = Action::AST(tokens);
}


ActionX ActionX::serializeObject()
{
    ActionX result;
    result.m_name = "test";
    result.m_max_run = 1;
    result.m_min_wait = 2;
    result.m_start_time = 3;
    result.keywords = {DeckKeyword::serializeObject()};
    result.condition = Action::AST::serializeObject();
    Quantity quant;
    quant.quantity = "test1";
    quant.args = {"test2"};
    Condition cond;
    cond.lhs = quant;
    cond.rhs = quant;
    cond.logic = Logical::AND;
    cond.cmp = Comparator::GREATER_EQUAL;
    cond.cmp_string = "test3";
    result.m_conditions = {cond};

    return result;
}


void ActionX::addKeyword(const DeckKeyword& kw) {
    this->keywords.push_back(kw);
}



Action::Result ActionX::eval(const Action::Context& context) const {
    return this->condition.eval(context);
}


bool ActionX::ready(const State& state, std::time_t sim_time) const {
    auto run_count = state.run_count(*this);
    if (run_count >= this->max_run())
        return false;

    if (sim_time < this->start_time())
        return false;

    if (run_count == 0)
        return true;

    if (this->min_wait() <= 0)
        return true;

    auto last_run = state.run_time(*this);
    return std::difftime(sim_time, last_run) >= this->min_wait();
}


std::vector<DeckKeyword>::const_iterator ActionX::begin() const {
    return this->keywords.begin();
}

std::vector<DeckKeyword>::const_iterator ActionX::end() const {
    return this->keywords.end();
}


std::vector<std::string> ActionX::keyword_strings() const {
    std::vector<std::string> keyword_strings;
    std::string keyword_string;
    {
        std::stringstream ss;
        DeckOutput::format fmt;
        for (const auto& kw : this->keywords) {
            ss << kw;
            ss << fmt.keyword_sep;
        }

        keyword_string = ss.str();
    }

    std::size_t offset = 0;
    while (true) {
        auto eol_pos = keyword_string.find('\n', offset);
        if (eol_pos == std::string::npos)
            break;

        if (eol_pos > offset)
            keyword_strings.push_back(keyword_string.substr(offset, eol_pos - offset));

        offset = eol_pos + 1;
    }
    keyword_strings.push_back("ENDACTIO");

    return keyword_strings;
}


const std::vector<Condition>& ActionX::conditions() const {
    return this->m_conditions;
}

std::size_t ActionX::id() const {
    return this->m_id;
}

void ActionX::update_id(std::size_t id) {
    this->m_id = id;
}


bool ActionX::operator==(const ActionX& data) const {
    return this->name() == data.name() &&
           this->max_run() == data.max_run() &&
           this->min_wait() == data.min_wait() &&
           this->start_time() == data.start_time() &&
           this->id() == data.id() &&
           this->keywords == data.keywords &&
           this->condition == data.condition &&
           this->conditions() == data.conditions();
}


void ActionX::required_summary(std::unordered_set<std::string>& required_summary) const {
    this->condition.required_summary(required_summary);
}

std::vector<std::string> ActionX::wellpi_wells(const WellMatcher& well_matcher, const std::vector<std::string>& matching_wells) const {
    std::unordered_set<std::string> wells;
    for (const auto& kw : this->keywords) {
        if (kw.name() == "WELPI") {
            for (const auto& record : kw) {
                std::vector<std::string> record_wells;
                const auto& wname_arg = record.getItem<ParserKeywords::WELPI::WELL_NAME>().get<std::string>(0);

                if (wname_arg == "?")
                    record_wells = matching_wells;
                else
                    record_wells = well_matcher.wells( wname_arg );

                for (const auto& well : record_wells)
                    wells.insert( well );
            }
        }
    }
    return std::vector<std::string>{ std::move_iterator(wells.begin()), std::move_iterator(wells.end()) };
}

}
}
