/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

class RimColorLegend;
class RimFormationNames;
class Rim3dView;

//==================================================================================================
///
//==================================================================================================
class RicImportFormationNamesFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

    static RimFormationNames* importFormationFiles( const QStringList& fileNames );

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static void addCustomColorLegend( QString& name, RimFormationNames* formationNames );

    void setFormationCellResultAndLegend( Rim3dView* activeView, QString& legendName );

private:
    RimColorLegend* m_lastCustomColorLegendCreated;
};
