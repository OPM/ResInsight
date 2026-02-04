//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "gtest/gtest.h"

#include "cafMockObjects.h"
#include "cafPdmPythonGenerator.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, CamelToSnakeCase_SimpleCamel )
{
    EXPECT_STREQ( "my_variable", caf::PdmPythonGenerator::camelToSnakeCase( "MyVariable" ).toStdString().c_str() );
    EXPECT_STREQ( "some_field_name", caf::PdmPythonGenerator::camelToSnakeCase( "SomeFieldName" ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, CamelToSnakeCase_AlreadySnake )
{
    EXPECT_STREQ( "already_snake", caf::PdmPythonGenerator::camelToSnakeCase( "already_snake" ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, CamelToSnakeCase_SingleWord )
{
    EXPECT_STREQ( "hello", caf::PdmPythonGenerator::camelToSnakeCase( "hello" ).toStdString().c_str() );
    EXPECT_STREQ( "hello", caf::PdmPythonGenerator::camelToSnakeCase( "Hello" ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, CamelToSnakeCase_AcronymPrefix )
{
    // Consecutive uppercase letters followed by a capitalised word
    EXPECT_STREQ( "xml_parser", caf::PdmPythonGenerator::camelToSnakeCase( "XMLParser" ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, CamelToSnakeCase_NumberInName )
{
    // A digit followed by an uppercase letter should produce an underscore
    EXPECT_STREQ( "my_var2_name", caf::PdmPythonGenerator::camelToSnakeCase( "myVar2Name" ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, CamelToSnakeCase_EmptyString )
{
    EXPECT_STREQ( "", caf::PdmPythonGenerator::camelToSnakeCase( "" ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, PythonifyDataValue_BooleanLiterals )
{
    EXPECT_STREQ( "True", caf::PdmPythonGenerator::pythonifyDataValue( "true" ).toStdString().c_str() );
    EXPECT_STREQ( "False", caf::PdmPythonGenerator::pythonifyDataValue( "false" ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, PythonifyDataValue_NoChange )
{
    EXPECT_STREQ( "hello", caf::PdmPythonGenerator::pythonifyDataValue( "hello" ).toStdString().c_str() );
    EXPECT_STREQ( "42", caf::PdmPythonGenerator::pythonifyDataValue( "42" ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, PythonifyDataValue_MixedBooleans )
{
    EXPECT_STREQ( "True and False", caf::PdmPythonGenerator::pythonifyDataValue( "true and false" ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
// dataTypeString maps PdmFieldHandle types to Python type strings via the builtins table.
// The builtins keys are typeid().name() strings, which is exactly what PdmXmlFieldHandle::dataTypeName()
// returns for value fields.  Child fields return their class keyword instead.
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, DataTypeString_Double )
{
    DemoPdmObject obj;
    EXPECT_STREQ( "float", caf::PdmPythonGenerator::dataTypeString( &obj.m_doubleField, false ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, DataTypeString_Int )
{
    DemoPdmObject obj;
    EXPECT_STREQ( "int", caf::PdmPythonGenerator::dataTypeString( &obj.m_intField, false ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, DataTypeString_QString )
{
    DemoPdmObject obj;
    EXPECT_STREQ( "str", caf::PdmPythonGenerator::dataTypeString( &obj.m_textField, false ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, DataTypeString_OptionalDouble )
{
    InheritedDemoObj obj;
    EXPECT_STREQ( "Optional[float]",
                  caf::PdmPythonGenerator::dataTypeString( &obj.m_optionalNumber, false ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
// Vector fields: dataTypeName() returns the element type; the List[] wrapper is added by the
// caller (generate()) after checking isVectorField().
TEST( PdmPythonGenerator, DataTypeString_VectorDouble )
{
    InheritedDemoObj obj;
    EXPECT_STREQ( "float", caf::PdmPythonGenerator::dataTypeString( &obj.m_numbers, false ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
// A scriptable AppEnum field triggers the early-return path: enumScriptTexts() is non-empty, so
// the function returns "str" without consulting the builtins table.
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, DataTypeString_ScriptableEnum )
{
    InheritedDemoObj obj;
    EXPECT_STREQ( "str", caf::PdmPythonGenerator::dataTypeString( &obj.m_myAppEnum, false ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, DataTypeString_PairBoolDouble )
{
    InheritedDemoObj obj;
    EXPECT_STREQ( "Tuple[bool, float]",
                  caf::PdmPythonGenerator::dataTypeString( &obj.m_pairField, false ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, DataTypeString_PairBoolFloat )
{
    InheritedDemoObj obj;
    EXPECT_STREQ( "Tuple[bool, float]",
                  caf::PdmPythonGenerator::dataTypeString( &obj.m_pairFloatField, false ).toStdString().c_str() );
}

//--------------------------------------------------------------------------------------------------
// Child-field dataTypeName() returns the class keyword of the referenced type.  That keyword is
// not in the builtins table, so useStrForUnknownDataTypes controls whether it is kept or replaced
// with "str".
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, DataTypeString_ChildField )
{
    DemoPdmObject obj;
    EXPECT_STREQ( "SimpleObj",
                  caf::PdmPythonGenerator::dataTypeString( &obj.m_simpleObjPtrField, false ).toStdString().c_str() );
    EXPECT_STREQ( "str", caf::PdmPythonGenerator::dataTypeString( &obj.m_simpleObjPtrField, true ).toStdString().c_str() );
}
