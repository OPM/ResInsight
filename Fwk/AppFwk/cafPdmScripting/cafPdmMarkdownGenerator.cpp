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

#include "cafPdmMarkdownGenerator.h"

#include "cafPdmMarkdownBuilder.h"
#include "cafPdmObjectScriptabilityRegister.h"

using namespace caf;

CAF_PDM_CODE_GENERATOR_SOURCE_INIT( PdmMarkdownGenerator, "md" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString caf::PdmMarkdownGenerator::generate( PdmObjectFactory* factory ) const
{
    QString     generatedCode;
    QTextStream out( &generatedCode );

    std::vector<QString> classKeywords = factory->classKeywords();

    std::vector<std::shared_ptr<const PdmObject>> scriptableObjects;

    {
        std::vector<std::shared_ptr<const PdmObject>> allObjects = caf::PdmMarkdownBuilder::createAllObjects( factory );
        for ( auto obj : allObjects )
        {
            if ( PdmObjectScriptabilityRegister::isScriptable( obj.get() ) )
            {
                scriptableObjects.push_back( obj );
            }
        }
    }
    out << caf::PdmMarkdownBuilder::generateDocDataModelObjects( scriptableObjects );

    return generatedCode;
}
