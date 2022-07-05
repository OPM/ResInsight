/*
  Copyright 2019 Equinor

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

#define BOOST_TEST_MODULE SummaryNode

#include <boost/test/unit_test.hpp>

#include <opm/io/eclipse/SummaryNode.hpp>

namespace {
    void expect_key(const Opm::EclIO::SummaryNode& node, const std::string& unique_key) {
        BOOST_CHECK_EQUAL(node.unique_key(), unique_key);
    }
}

BOOST_AUTO_TEST_SUITE(UniqueKey)

BOOST_AUTO_TEST_CASE(UniqueKey) {
    using Category = Opm::EclIO::SummaryNode::Category;
    using Type = Opm::EclIO::SummaryNode::Type;

    expect_key( { "KEYW", Category::Well,          Type::Rate, "NORA", 1, std::nullopt, std::nullopt}, "KEYW:NORA" );
    expect_key( { "KEYW", Category::Group,         Type::Rate, "NORA", 2, std::nullopt, std::nullopt}, "KEYW:NORA" );
    expect_key( { "KEYW", Category::Field,         Type::Rate, "NORA", 3, std::nullopt, std::nullopt}, "KEYW" );
    expect_key( { "KEYW", Category::Region,        Type::Rate, "NORA", 4, std::nullopt, std::nullopt}, "KEYW:4" );
    expect_key( { "KEYW", Category::Block,         Type::Rate, "NORA", 5, std::nullopt, std::nullopt}, "KEYW:5" );
    expect_key( { "KEYW", Category::Connection,    Type::Rate, "NORA", 6, std::nullopt, std::nullopt}, "KEYW:NORA:6" );
    expect_key( { "KEYW", Category::Segment,       Type::Rate, "NORA", 7, std::nullopt, std::nullopt}, "KEYW:NORA:7" );
    expect_key( { "KEYW", Category::Miscellaneous, Type::Rate, "NORA", 8, std::nullopt, std::nullopt}, "KEYW" );
}

BOOST_AUTO_TEST_CASE(InjectedNumberRenderer) {
    using Category = Opm::EclIO::SummaryNode::Category;
    using Type = Opm::EclIO::SummaryNode::Type;

    Opm::EclIO::SummaryNode positiveNode {
      "SIGN",
      Category::Region,
      Type::Undefined,
      "-",
      2,
      std::nullopt,
      std::nullopt
    };

    Opm::EclIO::SummaryNode negativeNode {
      "SIGN",
      Category::Region,
      Type::Undefined,
      "-",
      -2,
      std::nullopt,
      std::nullopt
    };

    auto chooseSign = [](const Opm::EclIO::SummaryNode& node) -> std::string {
        return node.number > 0 ? "+" : "-";
    };

    BOOST_CHECK_EQUAL(positiveNode.unique_key(chooseSign), "SIGN:+");
    BOOST_CHECK_EQUAL(negativeNode.unique_key(chooseSign), "SIGN:-");
}

BOOST_AUTO_TEST_CASE(user_defined) {
    auto summary_node = Opm::EclIO::SummaryNode{"FU_VAR1",
                                                Opm::EclIO::SummaryNode::Category::Field,
                                                Opm::EclIO::SummaryNode::Type::Undefined,
                                                "", -1 , std::nullopt, std::nullopt};
    BOOST_CHECK( summary_node.is_user_defined() );
}

BOOST_AUTO_TEST_SUITE_END() // UniqueKey
