/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

class RimMeasurement;
class Rim3dView;
class QObject;
class RiuMeasurementViewEventFilter;


//==================================================================================================
/// 
//==================================================================================================
class RicToggleMeasurementModeFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook(QAction* actionToSetup) override;
    bool isCommandChecked() override;

protected:
    void refreshActionLook();
    RimMeasurement* measurement() const;
    Rim3dView* activeView() const;
};

//==================================================================================================
///
//==================================================================================================
class RicTogglePolyMeasurementModeFeature : public RicToggleMeasurementModeFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    void onActionTriggered(bool isChecked) override;
    void setupActionLook(QAction* actionToSetup) override;
    bool isCommandChecked() override;

};
