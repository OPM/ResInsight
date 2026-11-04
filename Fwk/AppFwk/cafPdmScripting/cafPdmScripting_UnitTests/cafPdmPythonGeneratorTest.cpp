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
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmPythonGenerator.h"

namespace caf
{
//==================================================================================================
/// A method registered on a base class whose return type narrows to the concrete class of self.
//==================================================================================================
class NarrowingReturnBaseObj : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    NarrowingReturnBaseObj() { CAF_PDM_InitScriptableObject( "Narrowing Return Base" ); }
};
CAF_PDM_SOURCE_INIT( NarrowingReturnBaseObj, "NarrowingReturnBaseObj" );

class NarrowingReturnDerivedObj : public NarrowingReturnBaseObj
{
    CAF_PDM_HEADER_INIT;

public:
    NarrowingReturnDerivedObj() { CAF_PDM_InitScriptableObject( "Narrowing Return Derived" ); }
};
CAF_PDM_SOURCE_INIT( NarrowingReturnDerivedObj, "NarrowingReturnDerivedObj" );

// Third level: the base method is generated again through this class's inheritance stack after the
// derived class already carries its override.
class NarrowingReturnGrandchildObj : public NarrowingReturnDerivedObj
{
    CAF_PDM_HEADER_INIT;

public:
    NarrowingReturnGrandchildObj() { CAF_PDM_InitScriptableObject( "Narrowing Return Grandchild" ); }
};
CAF_PDM_SOURCE_INIT( NarrowingReturnGrandchildObj, "NarrowingReturnGrandchildObj" );

class NarrowingReturnBaseObj_clone : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    NarrowingReturnBaseObj_clone( caf::PdmObjectHandle* self )
        : caf::PdmObjectCreationMethod( self )
    {
        CAF_PDM_InitObject( "Clone", "", "", "Clone" );
    }

    std::expected<caf::PdmObjectHandle*, QString> execute() override { return std::unexpected( "Not used" ); }
    QString classKeywordReturnedType() const override { return NarrowingReturnBaseObj::classKeywordStatic(); }
    QString classKeywordReturnedTypeForSelf() const override { return selfClassKeyword(); }
};
CAF_PDM_OBJECT_METHOD_SOURCE_INIT( NarrowingReturnBaseObj, NarrowingReturnBaseObj_clone, "Clone" );

//==================================================================================================
/// A folder hierarchy shaped like the nested collections: the method is registered on the root and
/// returns the class named by subFolderClassKeyword(), or the root when that is empty.
///
///   FolderRootObj                        AddSubFolder() -> FolderRootObj
///     FolderTypedObj                     narrows to FolderTypedObj (SurfaceCollection)
///       FolderSameTypeObj                same type as its parent (EnsembleSurface)
///       FolderAbstractObj                not instantiated, like RimPolygonContainer
///         FolderBelowAbstractObj         same type as FolderTypedObj
//==================================================================================================
class FolderRootObj : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    FolderRootObj() { CAF_PDM_InitScriptableObject( "Folder Root" ); }

    virtual QString subFolderClassKeyword() const { return {}; }
};
CAF_PDM_SOURCE_INIT( FolderRootObj, "FolderRootObj" );

class FolderTypedObj : public FolderRootObj
{
    CAF_PDM_HEADER_INIT;

public:
    FolderTypedObj() { CAF_PDM_InitScriptableObject( "Folder Typed" ); }

    QString subFolderClassKeyword() const override { return FolderTypedObj::classKeywordStatic(); }
};
CAF_PDM_SOURCE_INIT( FolderTypedObj, "FolderTypedObj" );

class FolderAbstractObj : public FolderTypedObj
{
    CAF_PDM_HEADER_INIT;

public:
    FolderAbstractObj() { CAF_PDM_InitObject( "Folder Abstract" ); }
};
CAF_PDM_ABSTRACT_SOURCE_INIT( FolderAbstractObj, "FolderAbstractObj" );

class FolderBelowAbstractObj : public FolderAbstractObj
{
    CAF_PDM_HEADER_INIT;

public:
    FolderBelowAbstractObj() { CAF_PDM_InitScriptableObject( "Folder Below Abstract" ); }
};
CAF_PDM_SOURCE_INIT( FolderBelowAbstractObj, "FolderBelowAbstractObj" );

class FolderRootObj_addSubFolder : public caf::PdmObjectCreationMethod
{
    CAF_PDM_HEADER_INIT;

public:
    FolderRootObj_addSubFolder( caf::PdmObjectHandle* self )
        : caf::PdmObjectCreationMethod( self )
    {
        CAF_PDM_InitObject( "Add Sub Folder", "", "", "Add Sub Folder" );
    }

    std::expected<caf::PdmObjectHandle*, QString> execute() override { return std::unexpected( "Not used" ); }
    QString classKeywordReturnedType() const override { return FolderRootObj::classKeywordStatic(); }
    QString classKeywordReturnedTypeForSelf() const override
    {
        QString keyword = self<FolderRootObj>()->subFolderClassKeyword();
        return keyword.isEmpty() ? classKeywordReturnedType() : keyword;
    }
};
CAF_PDM_OBJECT_METHOD_SOURCE_INIT( FolderRootObj, FolderRootObj_addSubFolder, "AddSubFolder" );
} // namespace caf

namespace
{
QString generatedClassCode( const QString& generatedCode, const QString& className )
{
    int start = generatedCode.indexOf( QString( "class %1(" ).arg( className ) );
    if ( start == -1 ) return {};

    int end = generatedCode.indexOf( "\nclass ", start );
    if ( end == -1 ) end = generatedCode.size();

    return generatedCode.mid( start, end - start );
}
} // namespace

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

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, ReportsAmbiguousEnumClassNames )
{
    InheritedDemoObj::setUseAmbiguousEnumField( true );

    caf::PdmPythonGenerator generator;
    std::vector<QString>    errorMessages;
    generator.generate( caf::PdmDefaultObjectFactory::instance(), errorMessages );

    InheritedDemoObj::setUseAmbiguousEnumField( false );

    ASSERT_EQ( 1u, errorMessages.size() );
    EXPECT_TRUE( errorMessages.front().contains( "MyAppEnumValue" ) );
    EXPECT_TRUE( errorMessages.front().contains( "AlternativeEnumValue" ) );
    EXPECT_TRUE( errorMessages.front().contains( "PdmScriptEnumNameRegistry" ) );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, RejectsDuplicateEnumClassNames )
{
    CollisionTestPdmObject::setUseCollisionEnumField( true );

    caf::PdmPythonGenerator generator;
    std::vector<QString>    errorMessages;
    QString generatedCode = generator.generate( caf::PdmDefaultObjectFactory::instance(), errorMessages );

    CollisionTestPdmObject::setUseCollisionEnumField( false );

    ASSERT_EQ( 1u, errorMessages.size() );
    EXPECT_TRUE( generatedCode.isEmpty() );
    EXPECT_TRUE( errorMessages.front().contains( "Duplicate Python enum class name 'MyAppEnumValue'" ) );
    EXPECT_TRUE( errorMessages.front().contains( "InheritedDemoObj.MyAppEnumValue" ) );
    EXPECT_TRUE( errorMessages.front().contains( "CollisionTestPdmObject.MyAppEnumValue" ) );
    EXPECT_TRUE( errorMessages.front().contains( "shared header" ) );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, RejectsEnumClassNamesThatConflictWithModuleNames )
{
    CollisionTestPdmObject::setUseReservedNameEnumField( true );

    caf::PdmPythonGenerator generator;
    std::vector<QString>    errorMessages;
    QString generatedCode = generator.generate( caf::PdmDefaultObjectFactory::instance(), errorMessages );

    CollisionTestPdmObject::setUseReservedNameEnumField( false );

    ASSERT_EQ( 1u, errorMessages.size() );
    EXPECT_TRUE( generatedCode.isEmpty() );
    EXPECT_TRUE( errorMessages.front().contains( "enum class name 'Type'" ) );
    EXPECT_TRUE( errorMessages.front().contains( "CollisionTestPdmObject.Type" ) );
    EXPECT_TRUE( errorMessages.front().contains( "PdmScriptEnumNameRegistry" ) );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, EmitsTypedOverrideWhenReturnTypeNarrowsOnSubclass )
{
    caf::PdmPythonGenerator generator;
    std::vector<QString>    errorMessages;
    QString generatedCode = generator.generate( caf::PdmDefaultObjectFactory::instance(), errorMessages );

    EXPECT_TRUE( errorMessages.empty() );

    int baseClassStart    = generatedCode.indexOf( "class NarrowingReturnBaseObj(" );
    int derivedClassStart = generatedCode.indexOf( "class NarrowingReturnDerivedObj(" );
    ASSERT_NE( -1, baseClassStart );
    ASSERT_NE( -1, derivedClassStart );
    ASSERT_LT( baseClassStart, derivedClassStart );

    int grandchildClassStart = generatedCode.indexOf( "class NarrowingReturnGrandchildObj(" );
    ASSERT_LT( derivedClassStart, grandchildClassStart );

    int grandchildClassEnd = generatedCode.indexOf( "\nclass ", grandchildClassStart );
    if ( grandchildClassEnd == -1 ) grandchildClassEnd = generatedCode.size();

    QString baseClassCode       = generatedCode.mid( baseClassStart, derivedClassStart - baseClassStart );
    QString derivedClassCode    = generatedCode.mid( derivedClassStart, grandchildClassStart - derivedClassStart );
    QString grandchildClassCode = generatedCode.mid( grandchildClassStart, grandchildClassEnd - grandchildClassStart );

    // The base class keeps the declared type, each subclass gets a typed override of the same method
    EXPECT_TRUE( baseClassCode.contains( "def clone(self, ) -> NarrowingReturnBaseObj:" ) );
    EXPECT_TRUE( baseClassCode.contains( "_call_pdm_method_return_value(\"Clone\", NarrowingReturnBaseObj)" ) );
    EXPECT_TRUE( derivedClassCode.contains( "def clone(self, ) -> NarrowingReturnDerivedObj:" ) );
    EXPECT_TRUE( derivedClassCode.contains( "_call_pdm_method_return_value(\"Clone\", NarrowingReturnDerivedObj)" ) );
    EXPECT_TRUE( grandchildClassCode.contains( "def clone(self, ) -> NarrowingReturnGrandchildObj:" ) );
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
TEST( PdmPythonGenerator, SkipsTypedOverrideInheritedThroughAbstractClass )
{
    caf::PdmPythonGenerator generator;
    std::vector<QString>    errorMessages;
    QString generatedCode = generator.generate( caf::PdmDefaultObjectFactory::instance(), errorMessages );

    EXPECT_TRUE( errorMessages.empty() );

    QString rootCode          = generatedClassCode( generatedCode, "FolderRootObj" );
    QString typedCode         = generatedClassCode( generatedCode, "FolderTypedObj" );
    QString abstractCode      = generatedClassCode( generatedCode, "FolderAbstractObj" );
    QString belowAbstractCode = generatedClassCode( generatedCode, "FolderBelowAbstractObj" );
    ASSERT_FALSE( rootCode.isEmpty() );
    ASSERT_FALSE( typedCode.isEmpty() );
    ASSERT_FALSE( abstractCode.isEmpty() );
    ASSERT_FALSE( belowAbstractCode.isEmpty() );

    EXPECT_TRUE( rootCode.contains( "def add_sub_folder(self, ) -> FolderRootObj:" ) );
    EXPECT_TRUE( typedCode.contains( "def add_sub_folder(self, ) -> FolderTypedObj:" ) );

    // FolderBelowAbstractObj inherits the FolderTypedObj override through the abstract class
    EXPECT_FALSE( abstractCode.contains( "def add_sub_folder" ) );
    EXPECT_FALSE( belowAbstractCode.contains( "def add_sub_folder" ) );
}
