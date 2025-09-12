/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RiaDefines.h"

#include "RifReaderSettings.h"

#include <QString>

//==================================================================================================
///
///
//==================================================================================================
class RiaPreferencesGrid : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    using GridModelEnum = caf::AppEnum<RiaDefines::GridModelReader>;

public:
    RiaPreferencesGrid();

    static RiaPreferencesGrid* current();
    static RifReaderSettings   gridOnlyReaderSettings();
    RifReaderSettings          readerSettings();

    bool    importFaults() const;
    bool    importNNCs() const;
    bool    includeInactiveCellsInFaultGeometry() const;
    bool    importAdvancedMswData() const;
    QString includeFileAbsolutePathPrefix() const;
    bool    useResultIndexFile() const;
    bool    skipWellData() const;
    bool    loadAndShowSoil() const;
    bool    autoComputeDepthRelatedProperties() const;
    bool    onlyLoadActiveCells() const;
    bool    invalidateLongThinCells() const;
    bool    useCylindricalVisualization() const;

    RiaDefines::GridModelReader gridModelReader() const;

    void                        setGridModelReaderOverride( const std::string& readerName );
    void                        setGridModelReaderOverride( const RiaDefines::GridModelReader readerType );
    RiaDefines::GridModelReader gridModelReaderOverride() const;

    void appendItems( caf::PdmUiOrdering& uiOrdering );

private:
    caf::PdmField<GridModelEnum> m_gridModelReader;
    RiaDefines::GridModelReader  m_gridModelReaderOverride;

    caf::PdmField<bool>    m_importFaults;
    caf::PdmField<bool>    m_importNNCs;
    caf::PdmField<bool>    m_includeInactiveCellsInFaultGeometry;
    caf::PdmField<bool>    m_importAdvancedMswData;
    caf::PdmField<QString> m_includeFileAbsolutePathPrefix;
    caf::PdmField<bool>    m_useResultIndexFile;
    caf::PdmField<bool>    m_skipWellData;
    caf::PdmField<bool>    m_autoComputeDepthRelatedProperties;
    caf::PdmField<bool>    m_loadAndShowSoil;
    caf::PdmField<bool>    m_onlyLoadActiveCells;
    caf::PdmField<bool>    m_invalidateLongThinCells;
    caf::PdmField<bool>    m_useCylindricalVisualization;
};
