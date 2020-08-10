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

#include "RicfCommandObject.h"
#include "RimPlotCurve.h"

#include "cafCmdFeature.h"
#include "cafPdmPtrArrayField.h"

#include <vector>
//==================================================================================================
///
//==================================================================================================
class RicStackSelectedCurvesFeature : public caf::CmdFeature, public RicfCommandObject
{
    RICF_HEADER_INIT;

public:
    RicStackSelectedCurvesFeature();
    caf::PdmScriptResponse execute() override;

    static std::vector<RimPlotCurve*> plotCurvesFromSelection( const std::vector<caf::PdmUiItem*>& selectedItems );
    static std::vector<RimPlotCurve*> subsetOfPlotCurvesFromStacking( const std::vector<RimPlotCurve*>& plotCurves,
                                                                      bool                              isStacked );

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    caf::PdmPtrArrayField<RimPlotCurve*> m_curves;
};
