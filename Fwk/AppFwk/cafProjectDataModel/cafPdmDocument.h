//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#pragma once

#include "cafPdmDeprecation.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class QXmlStreamWriter;

namespace caf
{
//==================================================================================================
/// The PdmDocument class is the main class to do file based IO,
/// and is also supposed to act as the overall container of the objects read.
//==================================================================================================
class PdmDocument : public PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    PdmDocument();

    QString fileName() const;
    void    setFileName( const QString& fileName );

    std::vector<QString> readFile( const std::vector<PdmDeprecation>& deprecations = {} );
    bool                 writeFile();

    static void updateUiIconStateRecursively( PdmObjectHandle* root );

protected:
    QString               documentAsString();
    const PdmFieldHandle* fileNameHandle() const;

private:
    void                 writeDocumentToXmlStream( QXmlStreamWriter& xmlStream );
    std::vector<QString> readFile( QIODevice* device, const std::vector<PdmDeprecation>& deprecations );
    void                 writeFile( QIODevice* device );

private:
    PdmField<QString> m_fileName;
};

} // End of namespace caf
