/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RiaLasDefines.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"
#include "cafPdmUiItem.h"

#include <QList>
#include <QString>
#include <QStringList>

class RimCase;
class RimWellPath;
class RimWellLogFile;

//==================================================================================================
///
//==================================================================================================
class RicCreateDepthAdjustedLasFilesUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicCreateDepthAdjustedLasFilesUi();
    ~RicCreateDepthAdjustedLasFilesUi() override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;

    void    setDefaultValues();
    bool    hasValidSelections() const;
    QString invalidSelectionsLogString() const;

public:
    caf::PdmField<QString>              exportFolder;
    caf::PdmPtrField<RimCase*>          selectedCase;
    caf::PdmPtrField<RimWellPath*>      sourceWell;
    caf::PdmPtrField<RimWellLogFile*>   wellLogFile;
    caf::PdmField<std::vector<QString>> selectedResultProperties;
    caf::PdmPtrArrayField<RimWellPath*> destinationWells;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    const QStringList m_depthProperties = QStringList( { RiaDefines::propertyNameMeasuredDepth(),
                                                         RiaDefines::propertyNameTvdMslDepth(),
                                                         RiaDefines::propertyNameTvdRkbDepth() } );
};
