/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafCmdFeature.h"

class RimEclipseWell;
class RimView;
class RimWellLogExtractionCurve;
class RimWellLogTrack;
class RimWellPath;
class RimRftPlotCollection;
class RimWellRftPlot;

//==================================================================================================
/// 
//==================================================================================================
class RicNewRftPlotFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    // Overrides
    virtual bool isCommandEnabled();
    virtual void onActionTriggered( bool isChecked );
    virtual void setupActionLook( QAction* actionToSetup );

private:
    RimWellLogTrack*    selectedWellLogPlotTrack() const;
    RimWellPath*        selectedWellPath() const;
    RimEclipseWell*     selectedSimulationWell(int * branchIndex) const;
    bool                caseAvailable() const;

    template<typename T>
    T                   selectedPdmObject() const;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename T>
T RicNewRftPlotFeature::selectedPdmObject() const
{
    T objToFind = nullptr;

    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(objToFind);
    }

    return objToFind;
}
