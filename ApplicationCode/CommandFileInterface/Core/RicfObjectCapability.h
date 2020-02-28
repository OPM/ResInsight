/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "cafPdmObjectCapability.h"

#include <QString>

#include <map>

namespace caf
{
class PdmObject;
class PdmObjectHandle;
class PdmObjectFactory;
} // namespace caf

class QTextStream;
class RicfMessages;

//==================================================================================================
//
//
//
//==================================================================================================
class RicfObjectCapability : public caf::PdmObjectCapability
{
public:
    RicfObjectCapability( caf::PdmObjectHandle* owner, bool giveOwnership );

    ~RicfObjectCapability() override;

    void readFields( QTextStream& inputStream, caf::PdmObjectFactory* objectFactory, RicfMessages* errorMessageContainer );
    void writeFields( QTextStream& outputStream ) const;

    static void    registerScriptClassNameAndComment( const QString& classKeyword,
                                                      const QString& scriptClassName,
                                                      const QString& scriptClassComment );
    static QString scriptClassNameFromClassKeyword( const QString& classKeyword );
    static QString classKeywordFromScriptClassName( const QString& scriptClassName );
    static QString scriptClassComment( const QString& classKeyword );

    static bool isScriptable( const caf::PdmObject* object );
    static void
        addCapabilityToObject( caf::PdmObject* object, const QString& scriptClassName, const QString& scriptClassComment );

private:
    caf::PdmObjectHandle* m_owner;

    static std::map<QString, QString> s_classKeywordToScriptClassName;
    static std::map<QString, QString> s_scriptClassNameToClassKeyword;
    static std::map<QString, QString> s_scriptClassComments;
};
