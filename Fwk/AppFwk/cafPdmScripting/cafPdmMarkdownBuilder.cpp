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

#include "cafPdmMarkdownBuilder.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmObject.h"
#include "cafPdmObjectFactory.h"
#include "cafPdmObjectScriptingCapabilityRegister.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPythonGenerator.h"
#include "cafPdmXmlFieldHandle.h"

#include <QRegularExpression>
#include <QTextStream>

#include <algorithm>
#include <list>
#include <memory>
#include <set>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
;
QString caf::PdmMarkdownBuilder::generateDocDataModelObjects( std::vector<std::shared_ptr<const PdmObject>>& dataModelObjects )
{
    QString     generatedCode;
    QTextStream out( &generatedCode );

    // Sort to make sure super classes get created before sub classes
    std::sort( dataModelObjects.begin(),
               dataModelObjects.end(),
               []( std::shared_ptr<const PdmObject> lhs, std::shared_ptr<const PdmObject> rhs ) {
                   auto lhsStack = lhs->classInheritanceStack();
                   auto rhsStack = rhs->classInheritanceStack();

                   auto maxItems = std::min( lhsStack.size(), rhsStack.size() );
                   auto lhsIt    = lhsStack.begin();
                   auto rhsIt    = rhsStack.begin();
                   for ( size_t i = 0; i < maxItems; i++ )
                   {
                       if ( *lhsIt != *rhsIt )
                       {
                           return ( *lhsIt < *rhsIt );
                       }

                       lhsIt++;
                       rhsIt++;
                   }
                   return lhsStack.size() < rhsStack.size();
               } );

    std::map<QString, std::map<QString, std::pair<QString, QString>>> classAttributesGenerated;
    std::map<QString, std::map<QString, QString>>                     classMethodsGenerated;
    std::map<QString, QString>                                        classCommentsGenerated;

    // First generate all attributes and comments to go into each object
    for ( std::shared_ptr<const PdmObject> object : dataModelObjects )
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
                        QString snake_field_name = PdmPythonGenerator::camelToSnakeCase( scriptability->scriptFieldName() );

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
                                valueString = PdmPythonGenerator::pythonifyDataValue( valueString );

                                QString fieldCode =
                                    QString( "        self.%1 = %2\n" ).arg( snake_field_name ).arg( valueString );

                                QString fullComment =
                                    QString( "%1|%2|%3" ).arg( snake_field_name ).arg( dataType ).arg( comment );

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
                                QString fieldCode =
                                    QString( "    def %1(self):\n%2\n        return "
                                             "self.children(\"%3\", %4)[0] if len(self.children) > 0 else None\n" )
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
        }
    }

    // Write out classes
    std::set<QString> classesWritten;
    for ( std::shared_ptr<const caf::PdmObject> object : dataModelObjects )
    {
        const std::list<QString>& classInheritanceStack = object->classInheritanceStack();
        std::list<QString>        scriptSuperClassNames;

        size_t inheritanceLevel = 0;
        for ( auto it = classInheritanceStack.begin(); it != classInheritanceStack.end(); ++it )
        {
            const QString& classKeyword = *it;
            QString scriptClassName = PdmObjectScriptingCapabilityRegister::scriptClassNameFromClassKeyword( classKeyword );
            if ( scriptClassName.isEmpty() ) scriptClassName = classKeyword;

            if ( !classesWritten.count( scriptClassName ) )
            {
                QString classCode;

                {
                    size_t headingLevel    = inheritanceLevel + 1;
                    size_t maxHeadingLevel = 4;
                    size_t minHeadingLevel = 2;
                    headingLevel           = std::max( headingLevel, minHeadingLevel );
                    headingLevel           = std::min( headingLevel, maxHeadingLevel );

                    for ( size_t level = 0; level < headingLevel; level++ )
                    {
                        classCode += "#";
                    }
                }
                classCode += QString( " %1\n" ).arg( scriptClassName );

                if ( !scriptSuperClassNames.empty() )
                {
                    classCode += QString( "**Inherits %1**\n\n" ).arg( scriptSuperClassNames.back() );
                }

                if ( !classCommentsGenerated[classKeyword].isEmpty() || !classAttributesGenerated[classKeyword].empty() )
                {
                    if ( !classCommentsGenerated[classKeyword].isEmpty() )
                    {
                        classCode += QString( "%1\n\n" ).arg( classCommentsGenerated[classKeyword] );
                    }

                    if ( !classAttributesGenerated[classKeyword].empty() )
                    {
                        classCode += "Attribute | Type | Description\n";
                        classCode += "--------- | ---- | -----------\n";

                        for ( auto keyWordValuePair : classAttributesGenerated[classKeyword] )
                        {
                            QStringList items = keyWordValuePair.second.second.split( "|" );
                            if ( !items.empty() )
                            {
                                classCode += items[0];

                                for ( int i = 1; i < items.size(); i++ )
                                {
                                    classCode += " | ";
                                    classCode += items[i];
                                }
                                classCode += "\n";
                            }
                        }
                    }
                }

                out << classCode << "\n";
                classesWritten.insert( scriptClassName );
            }
            scriptSuperClassNames.push_back( scriptClassName );
            inheritanceLevel++;
        }
    }

    return generatedCode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<const PdmObject>> caf::PdmMarkdownBuilder::createAllObjects( caf::PdmObjectFactory* factory )
{
    std::vector<std::shared_ptr<const PdmObject>> objects;

    std::vector<QString> classKeywords = factory->classKeywords();
    for ( QString classKeyword : classKeywords )
    {
        auto       objectHandle = factory->create( classKeyword );
        PdmObject* object       = dynamic_cast<PdmObject*>( objectHandle );
        CAF_ASSERT( object );

        std::shared_ptr<PdmObject> sharedObject( object );
        objects.push_back( sharedObject );
    }

    return objects;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString caf::PdmMarkdownBuilder::generateDocCommandObjects( std::vector<std::shared_ptr<const PdmObject>>& commandObjects )
{
    QString     generatedText;
    QTextStream out( &generatedText );

    struct CommandDocData
    {
        QString                    snakeClassName;
        QString                    description;
        std::vector<AttributeItem> parameters;
    };

    std::vector<CommandDocData> objs;

    // First generate all attributes and comments to go into each object
    for ( std::shared_ptr<const PdmObject> object : commandObjects )
    {
        QString snakeCommandName = PdmPythonGenerator::camelToSnakeCase( object->classKeyword() );

        std::vector<AttributeItem>   attributes;
        std::vector<PdmFieldHandle*> fields;
        object->fields( fields );
        for ( auto field : fields )
        {
            QString snake_field_name = PdmPythonGenerator::camelToSnakeCase( field->keyword() );
            QString pythonDataType   = PdmPythonGenerator::dataTypeString( field, true );

            QString nativeDataType = field->xmlCapability()->dataTypeName();
            if ( nativeDataType.contains( "std::vector" ) )
            {
                pythonDataType = QString( "List of %1" ).arg( pythonDataType );
            }

            QString comment;
            {
                QStringList commentComponents;
                commentComponents << field->capability<PdmUiFieldHandle>()->uiName();
                commentComponents << field->capability<PdmUiFieldHandle>()->uiWhatsThis();
                commentComponents.removeAll( QString( "" ) );
                comment = commentComponents.join( ". " );
            }

            attributes.push_back( {snake_field_name, comment, pythonDataType} );
        }

        QString comment = caf::PdmObjectScriptingCapabilityRegister::scriptClassComment( object->classKeyword() );
        objs.push_back( {snakeCommandName, comment, attributes} );
        //        objectsAndAttributes[snakeCommandName] = attributes;
    }

    for ( auto object : objs )
    {
        out << QString( "## %1\n\n" ).arg( object.snakeClassName );

        if ( !object.description.isEmpty() )
        {
            out << object.description << "\n\n";
        }

        out << "Parameter | Type | Description\n";
        out << "--------- | ---- | -----------\n";

        for ( auto keyWordValuePair : object.parameters )
        {
            out << keyWordValuePair.name << " | ";
            out << keyWordValuePair.type << " | ";
            out << keyWordValuePair.description << "\n";
        }
        out << "\n";
    }

    return generatedText;
}
