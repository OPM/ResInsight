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
#include <stdexcept>
#include <math.h>
#include <filesystem>
#include <iostream>

#define BOOST_TEST_MODULE jsonParserTests
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>


#include <opm/json/JsonObject.hpp>

namespace framework = boost::unit_test::framework;



BOOST_AUTO_TEST_CASE(ParseValidJson) {
    std::string inline_json = "{\"key\": \"value\"}";
    BOOST_CHECK_NO_THROW(Json::JsonObject parser(inline_json));
}


BOOST_AUTO_TEST_CASE(ParseValidJson_fromLiteral) {
    BOOST_CHECK_NO_THROW(Json::JsonObject parser("{\"key\": \"value\"}"));
}



BOOST_AUTO_TEST_CASE(ParseInvalidJSON_throw) {
    std::string inline_json = "{\"key\": \"value\"";
    BOOST_CHECK_THROW(Json::JsonObject parser(inline_json) , std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(ParsevalidJSON_getString) {
    std::string inline_json = "{\"key\": \"value\"}";
    Json::JsonObject parser(inline_json);

    BOOST_CHECK_EQUAL( "value" , parser.get_string("key") );
}


BOOST_AUTO_TEST_CASE(ParsevalidJSONString_asString) {
    std::string inline_json = "{\"key\": \"value\"}";
    Json::JsonObject parser(inline_json);
    Json::JsonObject value = parser.get_item("key");

    BOOST_CHECK_EQUAL( "value" , value.as_string() );
}


BOOST_AUTO_TEST_CASE(ParsevalidJSONnotString_asString_throws) {
    std::string inline_json = "{\"key\": 100}";
    Json::JsonObject parser(inline_json);
    Json::JsonObject value = parser.get_item("key");

    BOOST_CHECK_THROW( value.as_string() , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(ParsevalidJSONint_asNumber) {
    std::string inline_json = "{\"key1\": 100, \"key2\" : 100.100 }";
    Json::JsonObject parser(inline_json);
    Json::JsonObject value1 = parser.get_item("key1");
    Json::JsonObject value2 = parser.get_item("key2");

    BOOST_CHECK_EQUAL( 100 , value1.as_int() );
    BOOST_CHECK( fabs(100.100 - value2.as_double()) < 0.00001 );
}

BOOST_AUTO_TEST_CASE(ParsevalidJSONint_isNumber) {
    std::string inline_json = "{\"key1\": 100, \"key2\" : 100.100 , \"key3\": \"string\"}";
    Json::JsonObject parser(inline_json);
    Json::JsonObject value1 = parser.get_item("key1");
    Json::JsonObject value2 = parser.get_item("key2");
    Json::JsonObject value3 = parser.get_item("key3");

    BOOST_CHECK( value1.is_number()) ;
    BOOST_CHECK( value2.is_number()) ;
    BOOST_CHECK_EQUAL( false , value3.is_number()) ;
}


BOOST_AUTO_TEST_CASE(ParsevalidJSONnotNumber_asNumber_throws) {
    std::string inline_json = "{\"key\": \"100X\"}";
    Json::JsonObject parser(inline_json);
    Json::JsonObject value = parser.get_item("key");

    BOOST_CHECK_THROW( value.as_int()    , std::invalid_argument );
    BOOST_CHECK_THROW( value.as_double() , std::invalid_argument );
}



BOOST_AUTO_TEST_CASE(ParsevalidJSON_getInt_OK) {
    std::string inline_json = "{\"key1\": 100 , \"key2\" : 200}";
    Json::JsonObject parser(inline_json);
    BOOST_CHECK_EQUAL( 100 , parser.get_int("key1") );
    BOOST_CHECK_EQUAL( 200 , parser.get_int("key2") );
}



BOOST_AUTO_TEST_CASE(ParsevalidJSON_hasItem) {
    std::string inline_json = "{\"key\": \"value\"}";
    Json::JsonObject parser(inline_json);
    BOOST_CHECK( parser.has_item("key"));
    BOOST_CHECK_EQUAL( false , parser.has_item("keyX"));
}



BOOST_AUTO_TEST_CASE(ParsevalidJSON_getMissingValue) {
    std::string inline_json = "{\"key\": \"value\"}";
    Json::JsonObject parser(inline_json);
    BOOST_CHECK_THROW( parser.get_string("keyX") , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(ParsevalidJSON_getNotScalar_throws) {
    std::string inline_json = "{\"key\": \"value\", \"list\": [1,2,3]}";
    Json::JsonObject parser(inline_json);
    BOOST_CHECK_EQUAL( "value" , parser.get_string("key"));
    BOOST_CHECK_THROW( parser.get_string("list") , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(ParsevalidJSON_getObject) {
    std::string inline_json = "{\"key\": \"value\", \"list\": [1,2,3]}";
    Json::JsonObject parser(inline_json);
    BOOST_CHECK_NO_THROW( Json::JsonObject object = parser.get_item("list") );
    BOOST_CHECK_NO_THROW( Json::JsonObject object = parser.get_item("key") );
}


BOOST_AUTO_TEST_CASE(ParsevalidJSON_getObject_missing_throw) {
    std::string inline_json = "{\"key\": \"value\", \"list\": [1,2,3]}";
    Json::JsonObject parser(inline_json);
    BOOST_CHECK_THROW( parser.get_item("listX") , std::invalid_argument );
}



BOOST_AUTO_TEST_CASE(ParsevalidJSON_CheckArraySize) {
    std::string inline_json = "{\"key\": \"value\", \"list\": [1,2,3]}";
    Json::JsonObject parser(inline_json);
    Json::JsonObject object = parser.get_item("list");
    BOOST_CHECK_EQUAL( 3U , object.size() );
}


BOOST_AUTO_TEST_CASE(ParsevalidJSON_isArray){
    std::string inline_json = "{\"key\": \"value\", \"list\": [1,2,3]}";
    Json::JsonObject parser(inline_json);
    Json::JsonObject list = parser.get_item("list");
    Json::JsonObject key = parser.get_item("key");

    BOOST_CHECK( list.is_array() );
    BOOST_CHECK_EQUAL( false , key.is_array( ) );
}


BOOST_AUTO_TEST_CASE(ParsevalidJSON_arrayGet) {
    std::string inline_json = "{\"key\": \"value\", \"list\": [1,2,3]}";
    Json::JsonObject parser(inline_json);
    Json::JsonObject list = parser.get_item("list");
    Json::JsonObject key = parser.get_item("key");

    BOOST_CHECK_NO_THROW( list.get_array_item( 0U ));
    BOOST_CHECK_NO_THROW( list.get_array_item( 1U ));
    BOOST_CHECK_NO_THROW( list.get_array_item( 2U ));

    BOOST_CHECK_THROW( list.get_array_item( 3U ) , std::invalid_argument );
    BOOST_CHECK_THROW( key.get_array_item( 0U ) , std::invalid_argument );
}


BOOST_AUTO_TEST_CASE(parseJSONString_testType) {
    std::string inline_json = "{\"item\": \"string\"}";
    Json::JsonObject json(inline_json);
    Json::JsonObject item = json.get_item( "item" );

    BOOST_CHECK( item.is_string() );
    BOOST_CHECK_EQUAL( false , item.is_number( ) );
    BOOST_CHECK_EQUAL( false , item.is_array( ) );
    BOOST_CHECK_EQUAL( false , item.is_object( ) );
}


BOOST_AUTO_TEST_CASE(parseJSONNumber_testType) {
    std::string inline_json = "{\"item\": 100}";
    Json::JsonObject json(inline_json);
    Json::JsonObject item = json.get_item( "item" );

    BOOST_CHECK_EQUAL( true  , item.is_number( ) );
    BOOST_CHECK_EQUAL( false , item.is_string() );
    BOOST_CHECK_EQUAL( false , item.is_array( ) );
    BOOST_CHECK_EQUAL( false , item.is_object( ) );
}


BOOST_AUTO_TEST_CASE(parseJSONArray_testType) {
    std::string inline_json = "{\"item\": [1,2,3]}";
    Json::JsonObject json(inline_json);
    Json::JsonObject item = json.get_item( "item" );

    BOOST_CHECK_EQUAL( false , item.is_number( ) );
    BOOST_CHECK_EQUAL( false , item.is_string() );
    BOOST_CHECK_EQUAL( true  , item.is_array( ) );
    BOOST_CHECK_EQUAL( false , item.is_object( ) );
}


BOOST_AUTO_TEST_CASE(parseJSONObject_testType) {
    std::string inline_json = "{\"item\": {\"list\": [0,1,2]}}";
    Json::JsonObject json(inline_json);
    Json::JsonObject item = json.get_item( "item" );

    BOOST_CHECK_EQUAL( false , item.is_number( ) );
    BOOST_CHECK_EQUAL( false , item.is_string() );
    BOOST_CHECK_EQUAL( false , item.is_array( ) );
    BOOST_CHECK_EQUAL( true  , item.is_object( ) );
}



BOOST_AUTO_TEST_CASE(Parse_fileDoesNotExist_Throws) {
    std::filesystem::path jsonFile("file/does/not/exist");
    BOOST_CHECK_THROW( Json::JsonObject parser(jsonFile) , std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(Parse_fileExists_OK) {
    const auto arg = framework::master_test_suite().argv[1];
    std::filesystem::path jsonFile(arg);
    BOOST_CHECK_NO_THROW( Json::JsonObject parser(jsonFile) );
}


BOOST_AUTO_TEST_CASE(to_string_ok) {
    const auto arg = framework::master_test_suite().argv[1];
    std::filesystem::path jsonFile(arg);
    Json::JsonObject parser(jsonFile);
    std::string json_string =
        "{\n"
        "	\"keywords\":	[{\n"
        "			\"name\":	\"BPR\",\n"
        "			\"items\":	[{\n"
        "					\"name\":	\"ItemX\",\n"
        "					\"size_type\":	\"SINGLE\",\n"
        "					\"value_type\":	\"FLOAT\"\n"
        "				}]\n"
        "		}, {\n"
        "			\"name\":	\"WWCT\",\n"
        "			\"size\":	0\n"
        "		}]\n"
        "}";

    BOOST_CHECK_EQUAL( parser.to_string() , json_string);
}


BOOST_AUTO_TEST_CASE(create) {
    Json::JsonObject json;
    json.add_item("name", "Awesome 4D");
    json.add_item("size", 100);
    json.add_item("pi", 3.14159265);
    {
        auto list = json.add_array("array");
        list.add("String");
        list.add(100);
        list.add(2.7172);
    }
    {
        auto dict = json.add_object("object");
        dict.add_item("key", "String");
        dict.add_item("int", 100);
        dict.add_item("double", 2.7172);
    }

    std::string s = json.dump();
    Json::JsonObject json2(s);
    BOOST_CHECK_EQUAL(json2.get_string("name"), "Awesome 4D");
    BOOST_CHECK_EQUAL(json2.get_int("size"), 100);

    auto array = json2.get_item("array");
    BOOST_CHECK( array.is_array() );
    BOOST_CHECK_EQUAL( array.size(), 3 );
    BOOST_CHECK_EQUAL( array.get_array_item(2).as_double(), 2.7172 );

    auto dict = json2.get_item("object");
    BOOST_CHECK( dict.is_object() );
    BOOST_CHECK_EQUAL( dict.get_string("key"), "String");
}
