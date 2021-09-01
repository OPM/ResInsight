/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include <QString>
#include <QStringList>

class RigEclipseCaseData;

//==================================================================================================
///
//==================================================================================================
class RicExportMultipleSurfacesUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicExportMultipleSurfacesUi();
    ~RicExportMultipleSurfacesUi() override;
    const QStringList& tabNames() const;

    void setLayersMinMax( int minLayer, int maxLayer );

    std::vector<int> layers() const;

    bool autoCreateEnsembleSurfaces() const;

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    caf::PdmField<std::vector<int>> m_layers;
    caf::PdmField<bool>             m_autoCreateEnsembleSurfaces;
    caf::PdmField<int>              m_minLayer;
    caf::PdmField<int>              m_maxLayer;

    QStringList m_tabNames;
};
