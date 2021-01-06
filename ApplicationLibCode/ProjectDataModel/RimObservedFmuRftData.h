/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RifReaderFmuRft.h"
#include "RimNamedObject.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

class RimObservedFmuRftData : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimObservedFmuRftData();

    void                   setDirectoryPath( const QString& path );
    void                   createRftReaderInterface();
    RifReaderRftInterface* rftReader();

    bool                 hasWell( const QString& wellPathName ) const;
    std::vector<QString> wells() const;
    std::vector<QString> labels( const RifEclipseRftAddress& rftAddress );

private:
    cvf::ref<RifReaderFmuRft> m_fmuRftReader;

    caf::PdmField<QString>                        m_directoryPath;
    caf::PdmProxyValueField<std::vector<QString>> m_wells;
};
