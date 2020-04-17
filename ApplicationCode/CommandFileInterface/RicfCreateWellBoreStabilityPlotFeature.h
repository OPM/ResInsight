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

#include "RicfCommandObject.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"

class RimWbsParameters;

//==================================================================================================
//
//
//
//==================================================================================================
class RicfCreateWbsPlotResult : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicfCreateWbsPlotResult( int viewId = -1 );

public:
    caf::PdmField<int> viewId;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RicfCreateWellBoreStabilityPlotFeature : public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicfCreateWellBoreStabilityPlotFeature();
    caf::PdmScriptResponse execute() override;

private:
    caf::PdmField<int>                    m_caseId;
    caf::PdmField<QString>                m_wellPath;
    caf::PdmField<int>                    m_timeStep;
    caf::PdmChildField<RimWbsParameters*> m_wbsParameters;
};
