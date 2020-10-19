/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "cafFilePath.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QString>
#include <vector>

class RimEclipseResultDefinition;
class RimEclipseCase;
class RimColorLegend;
class RigEclipseCaseData;

//==================================================================================================
///
//==================================================================================================
class RimNonNetLayers : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimNonNetLayers();
    ~RimNonNetLayers() override;

    caf::Signal<> changed;

    void setEclipseCase( RimEclipseCase* eclipseCase );

    const RimEclipseResultDefinition* resultDefinition() const;
    double                            cutOff() const;
    const QString&                    formation() const;
    const QString&                    facies() const;

protected:
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    RimColorLegend*             getFaciesColorLegend();
    static std::vector<QString> getFormationNames();
    static RimEclipseCase*      getEclipseCase();
    static RigEclipseCaseData*  getEclipseCaseData();

private:
    caf::PdmField<double>                           m_cutOff;
    caf::PdmChildField<RimEclipseResultDefinition*> m_resultDefinition;
    caf::PdmField<QString>                          m_formation;
    caf::PdmField<QString>                          m_facies;
};
