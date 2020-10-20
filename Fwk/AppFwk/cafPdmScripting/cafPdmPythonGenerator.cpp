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
QString PdmPythonGenerator::generate( PdmObjectFactory* factory ) const
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
    std::sort( dummyObjects.begin(), dummyObjects.end(), []( std::shared_ptr<PdmObject> lhs, std::shared_ptr<PdmObject> rhs ) {
        if ( lhs->inheritsClassWithKeyword( rhs->classKeyword() ) )
        {
            return false;
        }
        return lhs->classKeyword() < rhs->classKeyword();
    } );

    std::map<QString, std::map<QString, std::pair<QString, QString>>> classAttributesGenerated;
    std::map<QString, std::map<QString, QString>>                     classMethodsGenerated;
    std::map<QString, QString>                                        classCommentsGenerated;

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
                std::vector<PdmFieldHandle*> fields;
                object->fields( fields );
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
                            QString dataType = PdmPythonGenerator::dataTypeString( field, true );
                            if ( field->xmlCapability()->isVectorField() )
                            {
                                dataType = QString( "List of %1" ).arg( dataType );
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
                                    QString fullComment =
                                        QString( "        \"\"\"%1\n        Returns:\n             %2\n        \"\"\"" )
                                            .arg( comment )
                                            .arg( dataType );

                                    QString fieldCode = QString( "    def %1(self):\n%2\n        return "
                                                                 "self._call_get_method(\"%3\")\n" )
                                                            .arg( snake_field_name )
                                                            .arg( fullComment )
                                                            .arg( scriptability->scriptFieldName() );
                                    classMethodsGenerated[field->ownerClass()][snake_field_name] = fieldCode;
                                }
                                if ( proxyField->hasSetter() )
                                {
                                    QString fullComment = QString( "        \"\"\"Set %1\n        Arguments:\n"
                                                                   "            values (%2): data\n        \"\"\"" )
                                                              .arg( comment )
                                                              .arg( dataType );

                                    QString fieldCode = QString( "    def set_%1(self, values):\n%2\n        "
                                                                 "self._call_set_method(\"%3\", values)\n" )
                                                            .arg( snake_field_name )
                                                            .arg( fullComment )
                                                            .arg( scriptability->scriptFieldName() );
                                    classMethodsGenerated[field->ownerClass()][QString( "set_%1" ).arg( snake_field_name )] =
                                        fieldCode;
                                }
                            }
                            else
                            {
                                QString     valueString;
                                QTextStream valueStream( &valueString );
                                scriptability->readFromField( valueStream, true, true );
                                if ( valueString.isEmpty() ) valueString = QString( "\"\"" );
                                valueString = pythonifyDataValue( valueString );

                                QString fieldCode =
                                    QString( "        self.%1 = %2\n" ).arg( snake_field_name ).arg( valueString );

                                QString fullComment =
                                    QString( "%1 (%2): %3\n" ).arg( snake_field_name ).arg( dataType ).arg( comment );

                                classAttributesGenerated[field->ownerClass()][snake_field_name].first  = fieldCode;
                                classAttributesGenerated[field->ownerClass()][snake_field_name].second = fullComment;
                            }
                        }
                        else if ( pdmChildField || pdmChildArrayField )
                        {
                            QString dataType = PdmPythonGenerator::dataTypeString( field, false );
                            QString scriptDataType =
                                PdmObjectScriptingCapabilityRegister::scriptClassNameFromClassKeyword( dataType );

                            QString commentDataType = field->xmlCapability()->isVectorField()
                                                          ? QString( "List of %1" ).arg( scriptDataType )
                                                          : scriptDataType;

                            QString fullComment =
                                QString( "        \"\"\"%1\n        Returns:\n             %2\n        \"\"\"" )
                                    .arg( comment )
                                    .arg( commentDataType );

                            if ( pdmChildField )
                            {
                                QString fieldCode = QString( "    def %1(self):\n%2\n        children = "
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
                                QString fieldCode = QString( "    def %1(self):\n%2\n        return "
                                                             "self.children(\"%3\", %4)\n" )
                                                        .arg( snake_field_name )
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
                std::vector<PdmFieldHandle*> arguments;
                method->fields( arguments );

                QString methodComment = method->uiCapability()->uiWhatsThis();

                QString snake_method_name = camelToSnakeCase( methodName );

                if ( classMethodsGenerated[classKeyword][snake_method_name].count() ) continue;

                QStringList inputArgumentStrings;
                QStringList outputArgumentStrings;
                QStringList argumentComments;

                outputArgumentStrings.push_back( QString( "\"%1\"" ).arg( methodName ) );
                QString returnComment = method->defaultResult()->xmlCapability()->classKeyword();

                for ( auto field : arguments )
                {
                    bool    isList        = field->xmlCapability()->isVectorField();
                    QString defaultValue  = isList ? "[]" : "None";
                    auto    scriptability = field->capability<PdmAbstractFieldScriptingCapability>();
                    auto    argumentName  = camelToSnakeCase( scriptability->scriptFieldName() );
                    auto    dataType      = dataTypeString( field, false );
                    if ( isList ) dataType = "List of " + dataType;
                    inputArgumentStrings.push_back( QString( "%1=%2" ).arg( argumentName ).arg( defaultValue ) );
                    outputArgumentStrings.push_back( QString( "%1=%1" ).arg( argumentName ) );
                    argumentComments.push_back(
                        QString( "%1 (%2): %3" ).arg( argumentName ).arg( dataType ).arg( field->uiCapability()->uiWhatsThis() ) );
                }
                QString fullComment = QString( "        \"\"\"\n        %1\n        Arguments:\n            "
                                               "%2\n        Returns:\n            %3\n        \"\"\"" )
                                          .arg( methodComment )
                                          .arg( argumentComments.join( "\n            " ) )
                                          .arg( returnComment );

                QString methodCode = QString( "    def %1(self, %2):\n%3\n        return "
                                              "self._call_pdm_method(%4)\n" )
                                         .arg( snake_method_name )
                                         .arg( inputArgumentStrings.join( ", " ) )
                                         .arg( fullComment )
                                         .arg( outputArgumentStrings.join( ", " ) );

                classMethodsGenerated[classKeyword][snake_method_name] = methodCode;
            }
        }
    }

    // Write out classes
    std::set<QString> classesWritten;
    classesWritten.insert( "PdmObjectBase" );

    out << "from rips.pdmobject import PdmObjectBase\n";

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

                classCode += QString( "    def __init__(self, pb2_object=None, channel=None):\n" );
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
    out << "def class_dict():\n";
    out << "    classes = {}\n";
    for ( QString classKeyword : classesWritten )
    {
        out << QString( "    classes['%1'] = %1\n" ).arg( classKeyword );
    }
    out << "    return classes\n\n";

    out << "def class_from_keyword(class_keyword):\n";
    out << "    all_classes = class_dict()\n";
    out << "    if class_keyword in all_classes.keys():\n";
    out << "        return all_classes[class_keyword]\n";
    out << "    return None\n";

    return generatedCode;
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

    QString dataType = xmlObj->dataTypeName();

    std::map<QString, QString> builtins = {{QString::fromStdString( typeid( double ).name() ), "float"},
                                           {QString::fromStdString( typeid( float ).name() ), "float"},
                                           {QString::fromStdString( typeid( int ).name() ), "int"},
                                           {QString::fromStdString( typeid( time_t ).name() ), "time"},
                                           {QString::fromStdString( typeid( QString ).name() ), "str"}};

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
