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
#include "cafPdmPythonGenerator.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmObjectFactory.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmObjectScriptingCapabilityRegister.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmXmlFieldHandle.h"

#ifndef CAF_EXCLUDE_CVF
#include "cafPdmFieldScriptingCapabilityCvfColor3.h"
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"
#endif

#include <QRegularExpression>
#include <QTextStream>

#include <list>
#include <memory>
#include <set>

using namespace caf;

CAF_PDM_CODE_GENERATOR_SOURCE_INIT( PdmPythonGenerator, "py" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString caf::PdmPythonGenerator::generate( PdmObjectFactory* factory, std::vector<QString>& errorMessages ) const
{
    QString     generatedCode;
    QTextStream out( &generatedCode );

    std::vector<QString> classKeywords = factory->classKeywords();

    std::vector<std::shared_ptr<PdmObject>> dummyObjects;
    for ( QString classKeyword : classKeywords )
    {
        auto       objectHandle = factory->create( classKeyword );
        PdmObject* object       = dynamic_cast<PdmObject*>( objectHandle );
        CAF_ASSERT( object );

        std::shared_ptr<PdmObject> sharedObject( object );
        if ( PdmObjectScriptingCapabilityRegister::isScriptable( sharedObject.get() ) )
        {
            dummyObjects.push_back( sharedObject );
        }
    }

    // Sort to make sure super classes get created before sub classes
    std::sort( dummyObjects.begin(),
               dummyObjects.end(),
               []( std::shared_ptr<PdmObject> lhs, std::shared_ptr<PdmObject> rhs )
               {
                   if ( lhs->inheritsClassWithKeyword( rhs->classKeyword() ) )
                   {
                       return false;
                   }
                   return lhs->classKeyword() < rhs->classKeyword();
               } );

    std::map<QString, std::map<QString, std::pair<QString, QString>>> classAttributesGenerated;
    std::map<QString, std::map<QString, QString>>                     classMethodsGenerated;
    std::map<QString, QString>                                        classCommentsGenerated;
    std::set<QString>                                                 dataTypesInChildFields;

    // First generate all attributes and comments to go into each object
    for ( std::shared_ptr<PdmObject> object : dummyObjects )
    {
        const std::list<QString>& classInheritanceStack = object->classInheritanceStack();

        for ( auto it = classInheritanceStack.begin(); it != classInheritanceStack.end(); ++it )
        {
            const QString& classKeyword = *it;
            QString scriptClassComment  = PdmObjectScriptingCapabilityRegister::scriptClassComment( classKeyword );

            std::map<QString, QString> attributesGenerated;

            if ( !scriptClassComment.isEmpty() ) classCommentsGenerated[classKeyword] = scriptClassComment;

            if ( classKeyword == object->classKeyword() )
            {
                std::vector<PdmFieldHandle*> fields = object->fields();
                for ( auto field : fields )
                {
                    auto scriptability = field->template capability<PdmAbstractFieldScriptingCapability>();
                    if ( scriptability != nullptr )
                    {
                        QString snake_field_name = camelToSnakeCase( scriptability->scriptFieldName() );

                        QString comment;
                        {
                            QStringList commentComponents;
                            commentComponents << field->capability<PdmUiFieldHandle>()->uiName();
                            commentComponents << field->capability<PdmUiFieldHandle>()->uiWhatsThis();
                            commentComponents.removeAll( QString( "" ) );
                            comment = commentComponents.join( ". " );
                        }

                        auto pdmValueField      = dynamic_cast<const PdmValueField*>( field );
                        auto pdmChildField      = dynamic_cast<const PdmChildFieldHandle*>( field );
                        auto pdmChildArrayField = dynamic_cast<const PdmChildArrayFieldHandle*>( field );
                        if ( pdmValueField )
                        {
                            QString dataType = scriptability->dataType();
                            if ( dataType.isEmpty() )
                            {
                                dataType = PdmPythonGenerator::dataTypeString( field, false );
                            }

                            if ( field->xmlCapability()->isVectorField() )
                            {
                                dataType = QString( "List[%1]" ).arg( dataType );
                            }

                            bool shouldBeMethod = false;
                            auto proxyField     = dynamic_cast<const PdmProxyFieldHandle*>( field );
                            if ( proxyField && proxyField->isStreamingField() ) shouldBeMethod = true;

                            if ( classAttributesGenerated[field->ownerClass()].count( snake_field_name ) ) continue;
                            if ( classMethodsGenerated[field->ownerClass()].count( snake_field_name ) ) continue;

                            QVariant valueVariant = pdmValueField->toQVariant();

                            if ( shouldBeMethod )
                            {
                                if ( proxyField->hasGetter() )
                                {
                                    QString fullComment = QString( "        \"\"\"%1\n\n        Returns:\n             "
                                                                   "%2\n        \"\"\"" )
                                                              .arg( comment )
                                                              .arg( dataType );

                                    QString fieldCode = QString( "    def %1(self) -> %2:\n%3\n        return "
                                                                 "self._call_get_method(\"%4\")\n" )
                                                            .arg( snake_field_name )
                                                            .arg( dataType )
                                                            .arg( fullComment )
                                                            .arg( scriptability->scriptFieldName() );
                                    classMethodsGenerated[field->ownerClass()][snake_field_name] = fieldCode;
                                }
                                if ( proxyField->hasSetter() )
                                {
                                    QString fullComment = QString( "        \"\"\"Set %1\n\n        Arguments:\n"
                                                                   "            values (%2): data\n        \"\"\"" )
                                                              .arg( comment )
                                                              .arg( dataType );

                                    QString fieldCode =
                                        QString( "    def set_%1(self, values : %2) -> None:\n%3\n        "
                                                 "self._call_set_method(\"%4\", values)\n" )
                                            .arg( snake_field_name )
                                            .arg( dataType )
                                            .arg( fullComment )
                                            .arg( scriptability->scriptFieldName() );
                                    classMethodsGenerated[field->ownerClass()][QString( "set_%1" ).arg( snake_field_name )] =
                                        fieldCode;
                                }
                            }
                            else
                            {
                                QString defaultValue = getDefaultValue( field );

                                if ( dataType.toLower().contains( "optional" ) )
                                {
                                    auto strippedValue = defaultValue;
                                    strippedValue.remove( "\"" );

                                    if ( strippedValue.isEmpty() )
                                    {
                                        // An empty string as default value for an optional field is set to the None
                                        defaultValue = "None";
                                    }
                                }
                                else if ( defaultValue == "None" )
                                {
                                    // TODO: Consider using PdmField<std::optional<T>> for optional fields instead of
                                    // using the default value to manipulate the type
                                    dataType = QString( "Optional[%1]" ).arg( dataType );
                                }

                                QString fieldCode =
                                    QString( "        self.%1: %2 = %3\n" ).arg( snake_field_name ).arg( dataType ).arg( defaultValue );

                                QString fullComment;
                                {
                                    QString commentAndEnum = comment;

                                    QStringList enumTexts = scriptability->enumScriptTexts();
                                    if ( !enumTexts.empty() )
                                    {
                                        // Replace the comment text with enum values
                                        // The space is limited for the generation of documentation
                                        commentAndEnum = "One of [" + enumTexts.join( ", " ) + "]";
                                    }

                                    fullComment =
                                        QString( "%1 (%2): %3\n" ).arg( snake_field_name ).arg( dataType ).arg( commentAndEnum );
                                }

                                classAttributesGenerated[field->ownerClass()][snake_field_name].first  = fieldCode;
                                classAttributesGenerated[field->ownerClass()][snake_field_name].second = fullComment;
                            }
                        }
                        else if ( pdmChildField || pdmChildArrayField )
                        {
                            QString dataType = PdmPythonGenerator::dataTypeString( field, false );
                            QString scriptDataType =
                                PdmObjectScriptingCapabilityRegister::scriptClassNameFromClassKeyword( dataType );

                            dataTypesInChildFields.insert( scriptDataType );

                            QString commentDataType = field->xmlCapability()->isVectorField()
                                                          ? QString( "List[%1]" ).arg( scriptDataType )
                                                          : scriptDataType;

                            QString fullComment =
                                QString( "        \"\"\"%1\n\n        Returns:\n             %2\n        \"\"\"" )
                                    .arg( comment )
                                    .arg( commentDataType );

                            if ( pdmChildField )
                            {
                                QString fieldCode =
                                    QString( "    def %1(self) -> Optional[%4]:\n%2\n        children = "
                                             "self.children(\"%3\", %4)\n        return children[0] if "
                                             "len(children) > 0 else None\n" )
                                        .arg( snake_field_name )
                                        .arg( fullComment )
                                        .arg( scriptability->scriptFieldName() )
                                        .arg( scriptDataType );
                                classMethodsGenerated[field->ownerClass()][snake_field_name] = fieldCode;
                            }
                            else
                            {
                                QString fieldCode = QString( "    def %1(self) -> List[%2]:\n%3\n        return "
                                                             "self.children(\"%4\", %5)\n" )
                                                        .arg( snake_field_name )
                                                        .arg( scriptDataType )
                                                        .arg( fullComment )
                                                        .arg( scriptability->scriptFieldName() )
                                                        .arg( scriptDataType );
                                classMethodsGenerated[field->ownerClass()][snake_field_name] = fieldCode;
                            }
                        }
                    }
                }
            }

            for ( QString methodName : PdmObjectMethodFactory::instance()->registeredMethodNames( classKeyword ) )
            {
                std::shared_ptr<PdmObjectMethod> method =
                    PdmObjectMethodFactory::instance()->createMethod( object.get(), methodName );
                std::vector<PdmFieldHandle*> arguments = method->fields();

                QString methodComment = method->uiCapability()->uiWhatsThis();

                QString snake_method_name = camelToSnakeCase( methodName );

                if ( classMethodsGenerated[classKeyword][snake_method_name].length() ) continue;

                QStringList inputArgumentStrings;
                QStringList outputArgumentStrings;
                QStringList argumentComments;

                outputArgumentStrings.push_back( QString( "\"%1\"" ).arg( methodName ) );

                QString returnDataType = "None";
                QString returnComment;
                if ( !method->classKeywordReturnedType().isEmpty() )
                {
                    QString classKeywordReturnedType = method->classKeywordReturnedType();
                    returnComment                    = classKeywordReturnedType;
                    returnDataType =
                        PdmObjectScriptingCapabilityRegister::scriptClassNameFromClassKeyword( classKeywordReturnedType );

                    outputArgumentStrings.push_back( QString( "%1" ).arg( returnDataType ) );

                    if ( method->isNullptrValidResult() )
                    {
                        returnDataType = QString( "Optional[%1]" ).arg( returnDataType );
                    }
                }

                for ( auto field : arguments )
                {
                    auto scriptability = field->capability<PdmAbstractFieldScriptingCapability>();
                    auto argumentName  = camelToSnakeCase( scriptability->scriptFieldName() );
                    auto dataType      = dataTypeString( field, false );

                    bool isList = field->xmlCapability()->isVectorField();
                    if ( isList ) dataType = QString( "List[%1]" ).arg( dataType );

                    QString defaultValue = getDefaultValue( field );
                    if ( defaultValue == "None" )
                    {
                        // TODO: Consider using PdmField<std::optional<T>> for optional fields instead of using
                        // the default value to manipulate the type

                        dataType = QString( "Optional[%1]" ).arg( dataType );
                    }

                    QString commentOrEnumDescription = field->uiCapability()->uiWhatsThis();

                    QStringList enumTexts = scriptability->enumScriptTexts();
                    if ( !enumTexts.empty() )
                    {
                        // Replace the comment text with enum values
                        // The space is limited for the generation of documentation
                        commentOrEnumDescription = "One of [" + enumTexts.join( ", " ) + "]";
                    }

                    inputArgumentStrings.push_back(
                        QString( "%1: %2=%3" ).arg( argumentName ).arg( dataType ).arg( defaultValue ) );
                    outputArgumentStrings.push_back( QString( "%1=%1" ).arg( argumentName ) );
                    argumentComments.push_back(
                        QString( "%1 (%2): %3" ).arg( argumentName ).arg( dataType ).arg( commentOrEnumDescription ) );
                }
                QString fullComment = QString( "        \"\"\"\n        %1\n\n        Arguments:\n            "
                                               "%2\n        Returns:\n            %3\n        \"\"\"" )
                                          .arg( methodComment )
                                          .arg( argumentComments.join( "\n            " ) )
                                          .arg( returnComment );

                QString methodBody = QString( "self._call_pdm_method_void(%1)" ).arg( outputArgumentStrings.join( ", " ) );
                if ( returnDataType != "None" )
                {
                    if ( method->isNullptrValidResult() )
                    {
                        methodBody = QString( "return self._call_pdm_method_return_optional_value(%1)" )
                                         .arg( outputArgumentStrings.join( ", " ) );
                    }
                    else
                    {
                        methodBody =
                            QString( "return self._call_pdm_method_return_value(%1)" ).arg( outputArgumentStrings.join( ", " ) );
                    }
                }

                QString methodCode = QString( "    def %1(self, %2) -> %3:\n%4\n        %5\n" )
                                         .arg( snake_method_name )
                                         .arg( inputArgumentStrings.join( ", " ) )
                                         .arg( returnDataType )
                                         .arg( fullComment )
                                         .arg( methodBody );

                classMethodsGenerated[classKeyword][snake_method_name] = methodCode;
            }
        }
    }

    // Write out classes
    std::set<QString> classesWritten;
    classesWritten.insert( "PdmObjectBase" );

    out << "from __future__ import annotations\n";
    out << "from rips.pdmobject import PdmObjectBase\n";
    out << "import PdmObject_pb2\n";
    out << "import grpc\n";
    out << "from typing import Optional, Dict, List, Type\n";
    out << "\n";

    for ( std::shared_ptr<PdmObject> object : dummyObjects )
    {
        const std::list<QString>& classInheritanceStack = object->classInheritanceStack();
        std::list<QString>        scriptSuperClassNames;

        for ( auto it = classInheritanceStack.begin(); it != classInheritanceStack.end(); ++it )
        {
            const QString& classKeyword = *it;
            QString scriptClassName = PdmObjectScriptingCapabilityRegister::scriptClassNameFromClassKeyword( classKeyword );
            if ( scriptClassName.isEmpty() ) scriptClassName = classKeyword;

            if ( !classesWritten.count( scriptClassName ) )
            {
                QString classCode;
                if ( scriptSuperClassNames.empty() )
                {
                    classCode = QString( "class %1:\n" ).arg( scriptClassName );
                }
                else
                {
                    classCode = QString( "class %1(%2):\n" ).arg( scriptClassName ).arg( scriptSuperClassNames.back() );
                }
                if ( !classCommentsGenerated[classKeyword].isEmpty() || !classAttributesGenerated[classKeyword].empty() )
                {
                    classCode += "    \"\"\"\n";
                    if ( !classCommentsGenerated[classKeyword].isEmpty() )
                    {
                        if ( !classCommentsGenerated[classKeyword].isEmpty() )
                        {
                            classCode += QString( "    %1\n\n" ).arg( classCommentsGenerated[classKeyword] );
                        }
                    }
                    if ( !classAttributesGenerated[classKeyword].empty() )
                    {
                        classCode += "    Attributes:\n";
                        for ( auto keyWordValuePair : classAttributesGenerated[classKeyword] )
                        {
                            classCode += "        " + keyWordValuePair.second.second;
                        }
                    }
                    classCode += "    \"\"\"\n";
                }
                classCode +=
                    QString( "    __custom_init__ = None #: Assign a custom init routine to be run at __init__\n\n" );

                classCode +=
                    QString( "    def __init__(self, pb2_object: Optional[PdmObject_pb2.PdmObject]=None, channel: "
                             "Optional[grpc.Channel]=None) -> None:\n" );
                if ( !scriptSuperClassNames.empty() )
                {
                    // Own attributes. This initializes a lot of attributes to None.
                    // This means it has to be done before we set any values.
                    for ( auto keyWordValuePair : classAttributesGenerated[classKeyword] )
                    {
                        classCode += keyWordValuePair.second.first;
                    }
                    // Parent constructor
                    classCode +=
                        QString( "        %1.__init__(self, pb2_object, channel)\n" ).arg( scriptSuperClassNames.back() );
                }

                classCode += QString( "        if %1.__custom_init__ is not None:\n" ).arg( scriptClassName );
                classCode += QString( "            %1.__custom_init__(self, pb2_object=pb2_object, channel=channel)\n" )
                                 .arg( scriptClassName );

                for ( auto keyWordValuePair : classMethodsGenerated[classKeyword] )
                {
                    classCode += "\n";
                    classCode += keyWordValuePair.second;
                    classCode += "\n";
                }

                out << classCode << "\n";
                classesWritten.insert( scriptClassName );
            }
            scriptSuperClassNames.push_back( scriptClassName );
        }
    }

    out << "def class_dict() -> Dict[str, Type[PdmObjectBase]]:\n";
    out << "    classes : Dict[str, Type[PdmObjectBase]] = {}\n";
    for ( QString classKeyword : classesWritten )
    {
        out << QString( "    classes['%1'] = %1\n" ).arg( classKeyword );
    }
    out << "    return classes\n\n";

    out << "def class_from_keyword(class_keyword : str) -> Optional[Type[PdmObjectBase]]:\n";
    out << "    all_classes = class_dict()\n";
    out << "    if class_keyword in all_classes.keys():\n";
    out << "        return all_classes[class_keyword]\n";
    out << "    return None\n";

    // Check if all referenced data types are exported as classes
    for ( const auto& scriptDataType : dataTypesInChildFields )
    {
        if ( classesWritten.count( scriptDataType ) == 0 )
        {
            QString errorText = "No export for data type " + scriptDataType;
            errorMessages.push_back( errorText );
        }
    }

    return generatedCode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmPythonGenerator::getDefaultValue( PdmFieldHandle* field )
{
    QString defaultValue = "None";

    bool isList = field->xmlCapability()->isVectorField();
    if ( isList )
    {
        defaultValue = "[]";
    }
    else
    {
        QString valueString;

        // Always make sure the default value for a ptrField is empty string
        if ( !field->hasPtrReferencedObjects() )
        {
            auto scriptability = field->template capability<PdmAbstractFieldScriptingCapability>();

            QTextStream valueStream( &valueString );
            scriptability->readFromField( valueStream, true, true );
        }

        if ( valueString.isEmpty() )
        {
            valueString = defaultValue;
        }

        valueString = pythonifyDataValue( valueString );

        defaultValue = valueString;
    }

    return defaultValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmPythonGenerator::camelToSnakeCase( const QString& camelString )
{
    static QRegularExpression re1( "(.)([A-Z][a-z]+)" );
    static QRegularExpression re2( "([a-z0-9])([A-Z])" );

    QString snake_case = camelString;
    snake_case.replace( re1, "\\1_\\2" );
    snake_case.replace( re2, "\\1_\\2" );
    return snake_case.toLower();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmPythonGenerator::dataTypeString( const PdmFieldHandle* field, bool useStrForUnknownDataTypes )
{
    auto xmlObj = field->capability<PdmXmlFieldHandle>();

    auto scriptability = field->capability<PdmAbstractFieldScriptingCapability>();
    if ( scriptability && !scriptability->enumScriptTexts().empty() ) return "str";

    QString dataType = PdmObjectScriptingCapabilityRegister::scriptClassNameFromClassKeyword( xmlObj->dataTypeName() );

    std::map<QString, QString> builtins = {
        { QString::fromStdString( typeid( double ).name() ), "float" },
        { QString::fromStdString( typeid( float ).name() ), "float" },
        { QString::fromStdString( typeid( int ).name() ), "int" },
        { QString::fromStdString( typeid( bool ).name() ), "bool" },
        { QString::fromStdString( typeid( time_t ).name() ), "int" },
        { QString::fromStdString( typeid( QString ).name() ), "str" },
        { QString::fromStdString( typeid( caf::FilePath ).name() ), "str" },
        { QString::fromStdString( typeid( std::vector<double> ).name() ), "List[float]" },
        { QString::fromStdString( typeid( std::optional<double> ).name() ), "Optional[float]" },
        { QString::fromStdString( typeid( std::optional<float> ).name() ), "Optional[float]" },
        { QString::fromStdString( typeid( std::optional<int> ).name() ), "Optional[int]" },
        { QString::fromStdString( typeid( std::optional<bool> ).name() ), "Optional[bool]" },
        { QString::fromStdString( typeid( std::optional<QString> ).name() ), "Optional[str]" },
    };

#ifndef CAF_EXCLUDE_CVF
    builtins[QString::fromStdString( typeid( cvf::Vec3d ).name() )]   = "List[float]";
    builtins[QString::fromStdString( typeid( cvf::Color3f ).name() )] = "str";
    builtins[QString::fromStdString( typeid( cvf::Mat4d ).name() )]   = "List[float]";
#endif

    bool foundBuiltin = false;
    for ( auto builtin : builtins )
    {
        if ( dataType == builtin.first )
        {
            dataType.replace( builtin.first, builtin.second );
            foundBuiltin = true;
        }
    }

    if ( !foundBuiltin && useStrForUnknownDataTypes )
    {
        dataType = "str";
    }

    return dataType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmPythonGenerator::pythonifyDataValue( const QString& dataValue )
{
    QString outValue = dataValue;
    outValue.replace( "false", "False" );
    outValue.replace( "true", "True" );
    return outValue;
}
