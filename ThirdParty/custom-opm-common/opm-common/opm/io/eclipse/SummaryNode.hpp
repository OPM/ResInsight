/*
   Copyright 2020 Equinor ASA.

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

#ifndef OPM_IO_SUMMARYNODE_HPP
#define OPM_IO_SUMMARYNODE_HPP

#include <functional>
#include <optional>
#include <string>
#include <unordered_set>
#include <array>
#include <limits>

namespace Opm { namespace EclIO {

    struct lgr_info {
        std::string name;
        std::array<int, 3> ijk;
    };

struct SummaryNode {
    enum class Category {
        Well,
        Group,
        Field,
        Region,
        Block,
        Connection,
        Segment,
        Aquifer,
        Node,
        Miscellaneous,
    };

    enum class Type {
        Rate,
        Total,
        Ratio,
        Pressure,
        Count,
        Mode,
        ProdIndex,
        Undefined,
    };


    std::string keyword;
    Category    category;
    Type        type;
    std::string wgname;
    int         number;
    std::optional<std::string> fip_region;
    std::optional<lgr_info> lgr;

    constexpr static int default_number { std::numeric_limits<int>::min() };

    std::string unique_key() const;

    using number_renderer = std::function<std::string(const SummaryNode&)>;
    std::string unique_key(number_renderer) const;

    bool is_user_defined() const;

    static Category category_from_keyword(const std::string&);

    // Return true for keywords which should be Miscellaneous, although the
    // naive first-character-based classification suggests something else.
    static bool miscellaneous_exception(const std::string& keyword);

    std::optional<std::string> display_name() const;
    std::optional<std::string> display_number() const;
    std::optional<std::string> display_number(number_renderer) const;
};

}} // namespace Opm::EclIO

#endif // OPM_IO_SUMMARYNODE_HPP
