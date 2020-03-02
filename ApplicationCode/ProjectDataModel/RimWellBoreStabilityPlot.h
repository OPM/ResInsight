/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimWellLogPlot.h"

#include "RigGeoMechWellLogExtractor.h"
#include "RigWbsParameter.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

class RimGeoMechCase;
class RimWellPath;
class RimWbsParameters;

class RimWellBoreStabilityPlot : public RimWellLogPlot
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellBoreStabilityPlot();
    void   applyWbsParametersToExtractor( RigGeoMechWellLogExtractor* extractor );
    double userDefinedValue( const RigWbsParameter& parameter ) const;
    void   copyWbsParameters( const RimWbsParameters* wbsParameters );
    void   setCaseWellPathAndTimeStep( RimGeoMechCase* geoMechCase, RimWellPath* wellPath, int timeStep );

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void onLoadDataAndUpdate() override;
    void childFieldChangedByUi( const caf::PdmFieldHandle* changedChildField ) override;
    void initAfterRead() override;

private:
    void applyDataSource();

private:
    caf::PdmChildField<RimWbsParameters*> m_wbsParameters;
};
