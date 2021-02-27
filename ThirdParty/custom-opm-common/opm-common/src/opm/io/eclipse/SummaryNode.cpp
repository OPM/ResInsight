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

#include <opm/io/eclipse/SummaryNode.hpp>

#include <numeric>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

constexpr bool use_number(Opm::EclIO::SummaryNode::Category category) {
    switch (category) {
    case Opm::EclIO::SummaryNode::Category::Aquifer:       [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Block:         [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Connection:    [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Region:        [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Segment:
        return true;
    case Opm::EclIO::SummaryNode::Category::Field:         [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Group:         [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Node:          [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Miscellaneous: [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Well:
        return false;
    }

    static_assert("Unhandled category in use_number");
    return false; // Never reached, but quells compiler warning
}

constexpr bool use_name(Opm::EclIO::SummaryNode::Category category) {
    switch (category) {
    case Opm::EclIO::SummaryNode::Category::Connection:    [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Group:         [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Segment:       [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Node:          [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Well:
        return true;
    case Opm::EclIO::SummaryNode::Category::Aquifer:       [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Block:         [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Field:         [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Miscellaneous: [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Region:
        return false;
    }

    static_assert("Unhandled category in use_name");
    return false; // Never reached, but quells compiler warning
}

std::string default_number_renderer(const Opm::EclIO::SummaryNode& node) {
    return std::to_string(node.number);
}

bool is_node_keyword(const std::string& keyword)
{
    static const auto node_kw = std::unordered_set<std::string> {
        "GPR",
    };

    return node_kw.find(keyword) != node_kw.end();
}

Opm::EclIO::SummaryNode::Category
distinguish_group_from_node(const std::string& keyword)
{
    return is_node_keyword(keyword)
        ? Opm::EclIO::SummaryNode::Category::Node
        : Opm::EclIO::SummaryNode::Category::Group;
}
}

std::string Opm::EclIO::SummaryNode::unique_key(number_renderer render_number) const {
    std::vector<std::string> key_parts { keyword } ;

    if (auto opt = display_name())
        key_parts.emplace_back(opt.value());

    if (auto opt = display_number(render_number))
        key_parts.emplace_back(opt.value());

    auto compose_key = [](std::string& key, const std::string& key_part) -> std::string {
        constexpr auto delimiter { ':' } ;
        return key.empty() ? key_part : key + delimiter + key_part;
    };

    return std::accumulate(std::begin(key_parts), std::end(key_parts), std::string(), compose_key);
}

std::string Opm::EclIO::SummaryNode::unique_key() const {
    return unique_key(default_number_renderer);
}

bool Opm::EclIO::SummaryNode::is_user_defined() const {
    static const std::unordered_set<std::string> udq_blacklist {
        "AUTOCOAR",
        "AUTOREF",
        "FULLIMP",
        "GUIDECAL",
        "GUIDERAT",
        "GUPFREQ",
        "RUNSPEC",
        "RUNSUM",
        "SUMMARY",
        "SUMTHIN",
        "SURF",
        "SURFACT",
        "SURFACTW",
        "SURFADDW",
        "SURFADS",
        "SURFCAPD",
        "SURFESAL",
        "SURFNUM",
        "SURFOPTS",
        "SURFROCK",
        "SURFST",
        "SURFSTES",
        "SURFVISC",
        "SURFWNUM",
    } ;

    static const std::regex user_defined_regex { "[ABCFGRSW]U[A-Z0-9_]+" } ;

    const bool matched     { std::regex_match(keyword, user_defined_regex) } ;
    const bool blacklisted { udq_blacklist.find(keyword) != udq_blacklist.end() } ;

    return matched && !blacklisted;
}

Opm::EclIO::SummaryNode::Category Opm::EclIO::SummaryNode::category_from_keyword(
    const std::string& keyword,
    const std::unordered_set<std::string>& miscellaneous_keywords
) {
    if (keyword.length() == 0) {
        return Category::Miscellaneous;
    }

    if (miscellaneous_keywords.find(keyword) != miscellaneous_keywords.end()) {
        return Category::Miscellaneous;
    }

    switch (keyword[0]) {
    case 'A': return Category::Aquifer;
    case 'B': return Category::Block;
    case 'C': return Category::Connection;
    case 'F': return Category::Field;
    case 'G': return distinguish_group_from_node(keyword);
    case 'R': return Category::Region;
    case 'S': return Category::Segment;
    case 'W': return Category::Well;
    default:  return Category::Miscellaneous;
    }
}

std::optional<std::string> Opm::EclIO::SummaryNode::display_name() const {
    if (use_name(category)) {
        return wgname;
    } else {
        return std::nullopt;
    }
}

std::optional<std::string> Opm::EclIO::SummaryNode::display_number() const {
    return display_number(default_number_renderer);
}

std::optional<std::string> Opm::EclIO::SummaryNode::display_number(number_renderer render_number) const {
    if (use_number(category)) {
        return render_number(*this);
    } else {
        return std::nullopt;
    }
}
