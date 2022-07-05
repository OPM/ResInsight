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

#ifndef UDQINPUT_HPP_
#define UDQINPUT_HPP_

#include <string>
#include <unordered_map>
#include <map>
#include <unordered_set>

#include <opm/input/eclipse/Schedule/UDQ/UDQInput.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQDefine.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQAssign.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQParams.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQFunctionTable.hpp>
#include <opm/input/eclipse/EclipseState/Util/OrderedMap.hpp>
#include <opm/input/eclipse/EclipseState/Util/IOrderSet.hpp>


namespace Opm {

    class DeckRecord;
    class SummaryState;
    class UDQState;
    class KeywordLocation;
    class WellMatcher;

    namespace RestartIO {
        struct RstState;
    }


    class UDQConfig {
    public:
        UDQConfig() = default;
        explicit UDQConfig(const UDQParams& params);
        UDQConfig(const UDQParams& params, const RestartIO::RstState& rst_state);

        static UDQConfig serializeObject();

        const std::string& unit(const std::string& key) const;
        bool has_unit(const std::string& keyword) const;
        bool has_keyword(const std::string& keyword) const;
        void add_record(const DeckRecord& record, const KeywordLocation& location, std::size_t report_step);

        void add_unit(const std::string& keyword, const std::string& unit);
        void add_update(const std::string& keyword, std::size_t report_step, const KeywordLocation& location, const std::vector<std::string>& data);
        void add_assign(const std::string& quantity, const std::vector<std::string>& selector, double value, std::size_t report_step);
        void add_assign(const std::string& quantity, const std::unordered_set<std::string>& selector, double value, std::size_t report_step);
        void add_define(const std::string& quantity, const KeywordLocation& location, const std::vector<std::string>& expression, std::size_t report_step);

        void eval_assign(std::size_t report_step, const WellMatcher& wm, SummaryState& st, UDQState& udq_state) const;
        void eval(std::size_t report_step, const WellMatcher& wm, SummaryState& st, UDQState& udq_state) const;
        const UDQDefine& define(const std::string& key) const;
        const UDQAssign& assign(const std::string& key) const;
        std::vector<UDQDefine> definitions() const;
        std::vector<UDQDefine> definitions(UDQVarType var_type) const;
        std::vector<UDQInput> input() const;

        // The size() method will return the number of active DEFINE and ASSIGN
        // statements; this will correspond to the length of the vector returned
        // from input().
        size_t size() const;

        UDQInput operator[](const std::string& keyword) const;
        UDQInput operator[](std::size_t insert_index) const;

        std::vector<UDQAssign> assignments() const;
        std::vector<UDQAssign> assignments(UDQVarType var_type) const;
        const UDQParams& params() const;
        const UDQFunctionTable& function_table() const;

        bool operator==(const UDQConfig& config) const;
        void required_summary(std::unordered_set<std::string>& summary_keys) const;


        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            udq_params.serializeOp(serializer);
            serializer.map(m_definitions);
            serializer.map(m_assignments);
            serializer(units);
            input_index.serializeOp(serializer);
            serializer.template map<decltype(type_count),false>(type_count);
            // The UDQFunction table is constant up to udq_params.
            // So we can just construct a new instance here.
            if (!serializer.isSerializing())
                udqft = UDQFunctionTable(udq_params);
        }

    private:
        void add_node(const std::string& quantity, UDQAction action);
        UDQAction action_type(const std::string& udq_key) const;
        void eval_assign(std::size_t report_step, SummaryState& st, UDQState& udq_state, UDQContext& context) const;
        void eval_define(std::size_t report_step, UDQState& udq_state, UDQContext& context) const;


        UDQParams udq_params;
        UDQFunctionTable udqft;


        /*
          The choices of datastructures are strongly motivated by the
          constraints imposed by the Eclipse formatted restart files; for
          writing restart files it is essential to keep meticolous control over
          the ordering of the keywords. In this class the ordering is mainly
          maintained by the input_index map which keeps track of the insert
          order of each keyword, and whether the keyword is currently DEFINE'ed
          or ASSIGN'ed.
        */
        std::unordered_map<std::string, UDQDefine> m_definitions;
        std::unordered_map<std::string, UDQAssign> m_assignments;
        std::unordered_map<std::string, std::string> units;

        IOrderSet<std::string> define_order;
        OrderedMap<UDQIndex> input_index;
        std::map<UDQVarType, std::size_t> type_count;
    };
}



#endif
