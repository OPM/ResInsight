/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RicStackSelectedCurvesFeature.h"

#include "RicfCommandObject.h"
#include "RimStackablePlotCurve.h"

#include "cafCmdFeature.h"
#include "cafPdmPtrArrayField.h"

//==================================================================================================
///
//==================================================================================================
class RicUnstackSelectedCurvesFeature : public caf::CmdFeature, public RicfCommandObject
{
    RICF_HEADER_INIT;

public:
    RicUnstackSelectedCurvesFeature();
    caf::PdmScriptResponse execute() override;

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    caf::PdmPtrArrayField<RimStackablePlotCurve*> m_curves;
};
