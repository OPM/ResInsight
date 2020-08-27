/*
  Copyright 2019 Equinor ASA.

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

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQEnums.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQInput.hpp>

namespace Opm {

    namespace {
        std::string strip_quotes(const std::string& s) {
            if (s[0] == '\'')
                return s.substr(1, s.size() - 2);
            else
                return s;
        }

    }

    UDQConfig::UDQConfig(const UDQParams& params) :
        udq_params(params),
        udqft(this->udq_params)
    {}


    UDQConfig::UDQConfig(const Deck& deck) :
        udq_params(deck),
        udqft(this->udq_params)
    {}

    UDQConfig UDQConfig::serializeObject()
    {
        UDQConfig result;
        result.udq_params = UDQParams::serializeObject();
        result.udqft = UDQFunctionTable(result.udq_params);
        result.m_definitions = {{"test1", UDQDefine::serializeObject()}};
        result.m_assignments = {{"test2", UDQAssign::serializeObject()}};
        result.units = {{"test3", "test4"}};
        result.input_index.insert({"test5", UDQIndex::serializeObject()});
        result.type_count = {{UDQVarType::SCALAR, 5}};

        return result;
    }

    const UDQParams& UDQConfig::params() const {
        return this->udq_params;
    }

    void UDQConfig::add_node( const std::string& quantity, UDQAction action) {
        auto index_iter = this->input_index.find(quantity);
        if (this->input_index.find(quantity) == this->input_index.end()) {
            auto var_type = UDQ::varType(quantity);
            auto insert_index = this->input_index.size();
            this->type_count[var_type] += 1;
            this->input_index[quantity] = UDQIndex(insert_index, this->type_count[var_type], action, var_type);
        } else
            index_iter->second.action = action;
    }

    void UDQConfig::add_assign(const std::string& quantity, const std::vector<std::string>& selector, double value) {
        this->add_node(quantity, UDQAction::ASSIGN);
        auto assignment = this->m_assignments.find(quantity);
        if (assignment == this->m_assignments.end())
            this->m_assignments.insert( std::make_pair(quantity, UDQAssign(quantity, selector, value )));
        else
            assignment->second.add_record(selector, value);
    }


    void UDQConfig::add_define(const std::string& quantity, const std::vector<std::string>& expression) {
        this->add_node(quantity, UDQAction::DEFINE);
        auto defined_iter = this->m_definitions.find( quantity );
        if (defined_iter != this->m_definitions.end())
            this->m_definitions.erase( defined_iter );

        this->m_definitions.insert( std::make_pair(quantity, UDQDefine(this->udq_params, quantity, expression)));
    }


    void UDQConfig::add_unit(const std::string& keyword, const std::string& quoted_unit) {
        const std::string unit = strip_quotes(quoted_unit);
        const auto pair_ptr = this->units.find(keyword);
        if (pair_ptr != this->units.end()) {
            if (pair_ptr->second != unit)
                throw std::invalid_argument("Illegal to change unit of UDQ keyword runtime");

            return;
        }
        /*
          A UNIT statement is sufficient to consider a UDQ keyword is defined;
          if it is not already defined with an ASSIGN / DEFINE keyword we
          default construct it with a scalar value of 0. The main purpose of
          doing this is to be able to really define the content in PYACTION
          keyword.
        */
        if (!this->has_keyword(keyword))
            this->add_assign(keyword, {}, 0);
        this->units[keyword] = unit;
    }


    void UDQConfig::add_record(const DeckRecord& record) {
        auto action = UDQ::actionType(record.getItem("ACTION").get<RawString>(0));
        const auto& quantity = record.getItem("QUANTITY").get<std::string>(0);
        const auto& data = RawString::strings( record.getItem("DATA").getData<RawString>() );

        if (action == UDQAction::UPDATE)
            throw std::invalid_argument("The UDQ action UPDATE is not yet implemented in opm/flow");

        if (action == UDQAction::UNITS)
            this->add_unit( quantity, data[0] );
        else {
            if (action == UDQAction::ASSIGN) {
                std::vector<std::string> selector(data.begin(), data.end() - 1);
                double value = std::stod(data.back());
                this->add_assign(quantity, selector, value);
            } else if (action == UDQAction::DEFINE)
                this->add_define(quantity, data);
            else
                throw std::runtime_error("Internal error - should not be here");
        }
    }


    std::vector<UDQDefine> UDQConfig::definitions() const {
        std::vector<UDQDefine> ret;
        for (const auto& index_pair : this->input_index) {
            if (index_pair.second.action == UDQAction::DEFINE) {
                const std::string& key = index_pair.first;
                ret.push_back(this->m_definitions.at(key));
            }
        }
        return ret;
    }


    std::vector<UDQDefine> UDQConfig::definitions(UDQVarType var_type) const {
        std::vector<UDQDefine> filtered_defines;
        for (const auto& index_pair : this->input_index) {
            if (index_pair.second.action == UDQAction::DEFINE) {
                const std::string& key = index_pair.first;
                const auto& udq_define = this->m_definitions.at(key);
                if (udq_define.var_type() == var_type)
                    filtered_defines.push_back(udq_define);
            }
        }
        return filtered_defines;
    }


    std::vector<UDQInput> UDQConfig::input() const {
        std::vector<UDQInput> res;
        for (const auto& index_pair : this->input_index) {
            const UDQIndex& index = index_pair.second;
            std::string u;
            if (this->has_unit(index_pair.first))
                u = this->unit(index_pair.first);

            if (index.action == UDQAction::DEFINE) {
                const std::string& key = index_pair.first;
                res.push_back(UDQInput(index, this->m_definitions.at(key), u));
            } else if (index_pair.second.action == UDQAction::ASSIGN) {
                const std::string& key = index_pair.first;
                res.push_back(UDQInput(index, this->m_assignments.at(key), u));
            }
        }
        return res;
    }

    std::size_t UDQConfig::size() const {
        std::size_t s = 0;
        for (const auto& index_pair : this->input_index) {
            if (index_pair.second.action == UDQAction::DEFINE)
                s += 1;
            else if (index_pair.second.action == UDQAction::ASSIGN)
                s += 1;
        }
        return s;
    }


    std::vector<UDQAssign> UDQConfig::assignments() const {
        std::vector<UDQAssign> ret;
        for (const auto& index_pair : this->input_index) {
            if (index_pair.second.action == UDQAction::ASSIGN) {
                const std::string& key = index_pair.first;
                ret.push_back(this->m_assignments.at(key));
            }
        }
        return ret;
    }


    std::vector<UDQAssign> UDQConfig::assignments(UDQVarType var_type) const {
        std::vector<UDQAssign> filtered_defines;
        for (const auto& index_pair : this->input_index) {
            if (index_pair.second.action == UDQAction::ASSIGN) {
                const std::string& key = index_pair.first;
                const auto& udq_define = this->m_assignments.at(key);
                if (udq_define.var_type() == var_type)
                    filtered_defines.push_back(udq_define);
            }
        }
        return filtered_defines;
    }



    const std::string& UDQConfig::unit(const std::string& key) const {
        const auto pair_ptr = this->units.find(key);
        if (pair_ptr == this->units.end())
            throw std::invalid_argument("No such UDQ quantity: " + key);

        return pair_ptr->second;
    }

    bool UDQConfig::has_unit(const std::string& keyword) const {
        return (this->units.count(keyword) > 0);
    }


    bool UDQConfig::has_keyword(const std::string& keyword) const {
        if (this->m_assignments.count(keyword) > 0)
            return true;

        if (this->m_definitions.count(keyword) > 0)
            return true;

        /*
          That a keyword is mentioned with UNITS is enough to consider it
          as a keyword which is present.
        */
        if (this->units.count(keyword) > 0)
            return true;

        return false;
    }


    const UDQInput UDQConfig::operator[](const std::string& keyword) const {
        const auto index_iter = this->input_index.find(keyword);
        if (index_iter == this->input_index.end())
            throw std::invalid_argument("Keyword: " + keyword + " not recognized as ASSIGN/DEFINE UDQ");

        std::string u;
        if (this->has_unit(keyword))
            u = this->unit(keyword);

        if (index_iter->second.action == UDQAction::ASSIGN)
            return UDQInput(this->input_index.at(keyword), this->m_assignments.at(keyword), u);

        if (index_iter->second.action == UDQAction::DEFINE)
            return UDQInput(this->input_index.at(keyword), this->m_definitions.at(keyword), u);

        throw std::logic_error("Internal error - should not be here");
    }


    const UDQFunctionTable& UDQConfig::function_table() const {
        return this->udqft;
    }


    bool UDQConfig::operator==(const UDQConfig& data) const {
        return this->params() == data.params() &&
               this->function_table() == data.function_table() &&
               this->m_definitions == data.m_definitions &&
               this->m_assignments == data.m_assignments &&
               this->units == data.units &&
               this->input_index == data.input_index &&
               this->type_count == data.type_count;
    }

    void UDQConfig::eval(SummaryState& st) const {
        const auto& func_table = this->function_table();
        UDQContext context(func_table, st);
        for (const auto& assign : this->assignments(UDQVarType::WELL_VAR)) {
            auto ws = assign.eval(st.wells());
            st.update_udq(ws);
        }

        for (const auto& def : this->definitions(UDQVarType::WELL_VAR)) {
            auto ws = def.eval(context);
            st.update_udq(ws);
        }

        for (const auto& assign : this->assignments(UDQVarType::GROUP_VAR)) {
            auto ws = assign.eval(st.groups());
            st.update_udq(ws);
        }

        for (const auto& def : this->definitions(UDQVarType::GROUP_VAR)) {
            auto ws = def.eval(context);
            st.update_udq(ws);
        }

        for (const auto& def : this->definitions(UDQVarType::FIELD_VAR)) {
            auto field_udq = def.eval(context);
            st.update_udq(field_udq);
        }
    }
}


