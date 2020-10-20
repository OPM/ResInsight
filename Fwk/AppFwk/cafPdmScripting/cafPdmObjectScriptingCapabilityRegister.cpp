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
#include "cafPdmObjectScriptingCapabilityRegister.h"

#include "cafPdmObject.h"

using namespace caf;

std::map<QString, QString> PdmObjectScriptingCapabilityRegister::s_classKeywordToScriptClassName;
std::map<QString, QString> PdmObjectScriptingCapabilityRegister::s_scriptClassNameToClassKeyword;
std::map<QString, QString> PdmObjectScriptingCapabilityRegister::s_scriptClassComments;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectScriptingCapabilityRegister::registerScriptClassNameAndComment( const QString& classKeyword,
                                                                              const QString& scriptClassName,
                                                                              const QString& scriptClassComment )
{
    s_classKeywordToScriptClassName[classKeyword]    = scriptClassName;
    s_scriptClassNameToClassKeyword[scriptClassName] = classKeyword;
    s_scriptClassComments[classKeyword]              = scriptClassComment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmObjectScriptingCapabilityRegister::scriptClassNameFromClassKeyword( const QString& classKeyword )
{
    auto it = s_classKeywordToScriptClassName.find( classKeyword );
    if ( it != s_classKeywordToScriptClassName.end() )
    {
        return it->second;
    }
    return classKeyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmObjectScriptingCapabilityRegister::classKeywordFromScriptClassName( const QString& scriptClassName )
{
    auto it = s_scriptClassNameToClassKeyword.find( scriptClassName );
    if ( it != s_scriptClassNameToClassKeyword.end() )
    {
        return it->second;
    }
    return scriptClassName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmObjectScriptingCapabilityRegister::scriptClassComment( const QString& classKeyword )
{
    auto it = s_scriptClassComments.find( classKeyword );
    if ( it != s_scriptClassComments.end() )
    {
        return it->second;
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmObjectScriptingCapabilityRegister::isScriptable( const caf::PdmObject* object )
{
    return s_classKeywordToScriptClassName.find( object->classKeyword() ) != s_classKeywordToScriptClassName.end();
}
