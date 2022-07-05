/*
  Copyright (c) 2020 Equinor ASA

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

#define BOOST_TEST_MODULE data_GuideRateValue

#include <boost/test/unit_test.hpp>

#include <opm/output/data/GuideRateValue.hpp>

#include "tests/MessageBuffer.cpp"

BOOST_AUTO_TEST_SUITE(GuideRate_Values)

BOOST_AUTO_TEST_CASE(Construct)
{
    using GRValue = ::Opm::data::GuideRateValue;
    auto grvalue = GRValue{};

    BOOST_CHECK_MESSAGE(! grvalue.has(GRValue::Item::Oil), "Default constructed GuideRateValue must not have Oil");
    BOOST_CHECK_MESSAGE(! grvalue.has(GRValue::Item::Gas), "Default constructed GuideRateValue must not have Gas");
    BOOST_CHECK_MESSAGE(! grvalue.has(GRValue::Item::Water), "Default constructed GuideRateValue must not have Water");
    BOOST_CHECK_MESSAGE(! grvalue.has(GRValue::Item::ResV), "Default constructed GuideRateValue must not have ResV");

    BOOST_CHECK_MESSAGE(! grvalue.has(static_cast<GRValue::Item>(1729)),
                        "Default constructed GuideRateValue must not have out-of-bounds phase 1729");

    BOOST_CHECK_MESSAGE(! grvalue.has(static_cast<GRValue::Item>(-1)),
                        "Default constructed GuideRateValue must not have out-of-bounds phase -1");
}

BOOST_AUTO_TEST_CASE(Set_and_Get)
{
    using GRValue = ::Opm::data::GuideRateValue;
    auto grvalue = GRValue{};

    BOOST_CHECK_THROW(grvalue.get(GRValue::Item::Oil), std::invalid_argument);
    BOOST_CHECK_THROW(grvalue.get(static_cast<GRValue::Item>(1729)), std::invalid_argument);
    BOOST_CHECK_THROW(grvalue.get(static_cast<GRValue::Item>(-1)), std::invalid_argument);

    grvalue.set(GRValue::Item::Oil, 123.456);
    grvalue.set(GRValue::Item::Water, -0.98765);
    grvalue.set(GRValue::Item::ResV, 567.89);

    BOOST_CHECK_THROW(grvalue.set(static_cast<GRValue::Item>(355113), 0.1234),
                      std::invalid_argument);

    BOOST_CHECK_CLOSE(grvalue.get(GRValue::Item::Oil), 123.456, 1.0e-10);
    BOOST_CHECK_CLOSE(grvalue.get(GRValue::Item::Water), -0.98765, 1.0e-10);
    BOOST_CHECK_CLOSE(grvalue.get(GRValue::Item::ResV), 567.89, 1.0e-10);

    BOOST_CHECK_THROW(grvalue.get(GRValue::Item::Gas), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(Copy_and_Assignment)
{
    using GRValue = ::Opm::data::GuideRateValue;
    auto grvalue1 = GRValue{};

    grvalue1.set(GRValue::Item::Oil, 123.456);
    grvalue1.set(GRValue::Item::Water, -0.98765);
    grvalue1.set(GRValue::Item::ResV, 567.89);

    const auto grvalue2{ grvalue1 };

    BOOST_CHECK_CLOSE(grvalue2.get(GRValue::Item::Oil), 123.456, 1.0e-10);
    BOOST_CHECK_CLOSE(grvalue2.get(GRValue::Item::Water), -0.98765, 1.0e-10);
    BOOST_CHECK_CLOSE(grvalue2.get(GRValue::Item::ResV), 567.89, 1.0e-10);

    BOOST_CHECK_THROW(grvalue2.get(GRValue::Item::Gas), std::invalid_argument);

    auto grvalue3 = GRValue{};
    grvalue3 = grvalue2;

    BOOST_CHECK_CLOSE(grvalue3.get(GRValue::Item::Oil), 123.456, 1.0e-10);
    BOOST_CHECK_CLOSE(grvalue3.get(GRValue::Item::Water), -0.98765, 1.0e-10);
    BOOST_CHECK_CLOSE(grvalue3.get(GRValue::Item::ResV), 567.89, 1.0e-10);

    BOOST_CHECK_THROW(grvalue3.get(GRValue::Item::Gas), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(Addition)
{
    using GRValue = ::Opm::data::GuideRateValue;
    auto grvalue1 = GRValue{};

    grvalue1.set(GRValue::Item::Oil, 123.456);
    grvalue1.set(GRValue::Item::ResV, 567.89);

    auto grvalue2 = GRValue{};
    grvalue2.set(GRValue::Item::Water, -0.98765);
    grvalue2.set(GRValue::Item::Oil, 123.321);

    grvalue1 += grvalue2;

    BOOST_CHECK_MESSAGE(grvalue1.has(GRValue::Item::Water),
                        "Operator '+=' must assign unset items");

    BOOST_CHECK_CLOSE(grvalue1.get(GRValue::Item::Oil), 246.777, 1.0e-10);
    BOOST_CHECK_CLOSE(grvalue1.get(GRValue::Item::Water), -0.98765, 1.0e-10);
    BOOST_CHECK_CLOSE(grvalue1.get(GRValue::Item::ResV), 567.89, 1.0e-10);
}

BOOST_AUTO_TEST_CASE(Clear)
{
    using GRValue = ::Opm::data::GuideRateValue;
    auto grvalue = GRValue{};

    grvalue.set(GRValue::Item::Oil, 123.456);
    grvalue.set(GRValue::Item::Water, -0.98765);
    grvalue.set(GRValue::Item::ResV, 567.89);

    grvalue.clear();

    BOOST_CHECK_MESSAGE(! grvalue.has(GRValue::Item::Oil), "Cleared GuideRateValue must not have Oil");
    BOOST_CHECK_MESSAGE(! grvalue.has(GRValue::Item::Gas), "Cleared GuideRateValue must not have Gas");
    BOOST_CHECK_MESSAGE(! grvalue.has(GRValue::Item::Water), "Cleared GuideRateValue must not have Water");
    BOOST_CHECK_MESSAGE(! grvalue.has(GRValue::Item::ResV), "Cleared GuideRateValue must not have ResV");
}

BOOST_AUTO_TEST_CASE(Serialize_ReadWrite)
{
    using GRValue = ::Opm::data::GuideRateValue;
    auto grvalue1 = GRValue{};

    grvalue1.set(GRValue::Item::Oil, 123.456);
    grvalue1.set(GRValue::Item::Water, -0.98765);
    grvalue1.set(GRValue::Item::ResV, 567.89);

    auto buffer = MessageBuffer{};
    grvalue1.write(buffer);

    auto grvalue2 = GRValue{};
    grvalue2.read(buffer);

    BOOST_CHECK_MESSAGE(  grvalue2.has(GRValue::Item::Oil), "Serialized GuideRateValue must have Oil");
    BOOST_CHECK_MESSAGE(! grvalue2.has(GRValue::Item::Gas), "Serialized GuideRateValue must NOT have Gas");
    BOOST_CHECK_MESSAGE(  grvalue2.has(GRValue::Item::Water), "Serialized GuideRateValue must have Water");
    BOOST_CHECK_MESSAGE(  grvalue2.has(GRValue::Item::ResV), "Serialized GuideRateValue must have Voidage");

    BOOST_CHECK_MESSAGE(! grvalue2.has(static_cast<GRValue::Item>(1729)),
                        "Serialized GuideRateValue must not have out-of-bounds phase 1729");

    BOOST_CHECK_MESSAGE(! grvalue2.has(static_cast<GRValue::Item>(-1)),
                        "Serialized GuideRateValue must not have out-of-bounds phase -1");

    BOOST_CHECK_CLOSE(grvalue2.get(GRValue::Item::Oil), 123.456, 1.0e-10);
    BOOST_CHECK_CLOSE(grvalue2.get(GRValue::Item::Water), -0.98765, 1.0e-10);
    BOOST_CHECK_CLOSE(grvalue2.get(GRValue::Item::ResV), 567.89, 1.0e-10);

    BOOST_CHECK_THROW(grvalue2.get(GRValue::Item::Gas), std::invalid_argument);

    BOOST_CHECK_MESSAGE(grvalue1 == grvalue2, "Serialized GuideRateValue must equal its original value");
}

BOOST_AUTO_TEST_SUITE_END()
