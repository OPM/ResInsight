/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021     Equinor ASA
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

class RimFaultRAPreprocSettings;
class RimEclipseView;
class RimEclipseInputCase;

//==================================================================================================
///
//==================================================================================================
class RicNewFaultReactAssessmentFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    void prepareDirectory( QString dirname, bool deleteExistingContent ) const;
    bool showSettingsGUI( RimFaultRAPreprocSettings& settings );
    bool runPreProc( RimFaultRAPreprocSettings& settings );

    void addParameterFileForCleanUp( QString filename );
    void cleanUpParameterFiles();

    RimEclipseView* getView( RimEclipseInputCase* eCase );

    std::list<QString> m_parameterFilesToCleanUp;
};
