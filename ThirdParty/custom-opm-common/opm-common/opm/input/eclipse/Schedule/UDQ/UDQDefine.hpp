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


#ifndef UDQ_DEFINE_HPP
#define UDQ_DEFINE_HPP

#include <optional>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQSet.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQContext.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQFunctionTable.hpp>
#include <opm/common/OpmLog/KeywordLocation.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQToken.hpp>

namespace Opm {

class UDQASTNode;
class ParseContext;
class ErrorGuard;

class UDQDefine{
public:
    UDQDefine();

    UDQDefine(const UDQParams& udq_params,
              const std::string& keyword,
              std::size_t report_step,
              const KeywordLocation& location,
              const std::vector<std::string>& deck_data);

    UDQDefine(const UDQParams& udq_params,
              const std::string& keyword,
              std::size_t report_step,
              const KeywordLocation& location,
              const std::vector<std::string>& deck_data,
              const ParseContext& parseContext,
              ErrorGuard& errors);

    template <typename T>
    UDQDefine(const UDQParams& udq_params,
              const std::string& keyword,
              std::size_t report_step,
              const KeywordLocation& location,
              const std::vector<std::string>& deck_data,
              const ParseContext& parseContext,
              T&& errors);

    static UDQDefine serializeObject();

    UDQSet eval(const UDQContext& context) const;
    const std::string& keyword() const;
    const std::string& input_string() const;
    const KeywordLocation& location() const;
    UDQVarType  var_type() const;
    std::set<UDQTokenType> func_tokens() const;
    void required_summary(std::unordered_set<std::string>& summary_keys) const;
    void update_status(UDQUpdate update_status, std::size_t report_step);
    std::pair<UDQUpdate, std::size_t> status() const;
    const std::vector<Opm::UDQToken> tokens() const;

    bool operator==(const UDQDefine& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_keyword);
        serializer(ast);
        serializer(m_var_type);
        m_location.serializeOp(serializer);
        serializer(string_data);
        serializer(m_update_status);
        serializer(m_report_step);
    }

private:
    std::string m_keyword;
    std::vector<Opm::UDQToken> m_tokens;
    std::shared_ptr<UDQASTNode> ast;
    UDQVarType m_var_type;
    KeywordLocation m_location;
    std::size_t m_report_step;
    UDQUpdate m_update_status;
    mutable std::optional<std::string> string_data;
};
}



#endif
