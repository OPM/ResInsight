/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RiaQDateTimeTools.h"

#include "cafCmdFeature.h"

class RimSummaryPlot;

//==================================================================================================
///
//==================================================================================================
class RicAsciiExportSummaryPlotFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static QString defaultExportDir();
    static QString getFileNameFromUserDialog( const QString& fileNameCandidate, const QString& defaultDir );
    static bool    exportTextToFile( const QString& fileName, const QString& text );

protected:
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

private:
    static bool exportAsciiForSummaryPlot( const QString&        fileName,
                                           const RimSummaryPlot* selectedSummaryPlots,
                                           DateTimePeriod        resamplingPeriod,
                                           bool                  showTimeAsLongString );
};
