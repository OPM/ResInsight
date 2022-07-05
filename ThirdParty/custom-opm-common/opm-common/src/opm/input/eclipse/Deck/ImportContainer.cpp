/*
  Copyright 2021 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <opm/input/eclipse/Deck/ImportContainer.hpp>

#include <stdexcept>
#include <unordered_set>

#include <fmt/format.h>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/LogUtil.hpp>
#include <opm/io/eclipse/EclFile.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

namespace Opm {

ImportContainer::ImportContainer(const Parser& parser, const UnitSystem& unit_system, const std::string& fname, bool formatted, std::size_t deck_size) {
    EclIO::EclFile ecl_file(fname, EclIO::EclFile::Formatted{formatted});
    const auto& header = ecl_file.getList();
    for (std::size_t kw_index = 0; kw_index < header.size(); kw_index++) {
        const auto& [name, data_type, _] = header[kw_index];
        (void)_;
        if (!parser.isRecognizedKeyword(name)) {
            OpmLog::info(fmt::format("{:<5} Skipping {} from IMPORT file {}", "", name, fname));
            continue;
        }

        const auto& parser_kw = parser.getKeyword(name);
        if (!parser_kw.isDataKeyword()) {
            OpmLog::info(fmt::format("{:<5} Skipping {} from IMPORT file {}", "", name, fname));
            continue;
        }

        const auto& parser_item = parser_kw.getRecord(0).get(0);
        if (parser_item.dataType() == type_tag::fdouble) {
            if (data_type == EclIO::REAL) {
                auto& float_data = ecl_file.get<float>(kw_index);
                std::vector<double> double_data{ float_data.begin(), float_data.end() };
                this->keywords.emplace_back(parser_kw, double_data, unit_system, unit_system);
            } else if (data_type == EclIO::DOUB) {
                auto& double_data = ecl_file.get<double>(kw_index);
                this->keywords.emplace_back(parser_kw, double_data, unit_system, unit_system);
            }
        } else if (parser_item.dataType() == type_tag::integer) {
            const auto& data = ecl_file.get<int>(kw_index);
            this->keywords.emplace_back(parser_kw, data);
        } else
            throw std::logic_error(fmt::format("File: {} keyword:{}\nIMPORT keyword only supports integer and floating point data keywords", fname, name));

        deck_size += 1;
        auto msg = fmt::format("{:5} Loading {:<8} from IMPORT file {}", deck_size, name, fname);
        OpmLog::info(msg);
    }
}



}
