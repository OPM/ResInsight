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

#include <QString>

#include <vector>

class RimWellLogCurve;
class RimWellLogPlot;

//==================================================================================================
///
//==================================================================================================
class RicExportToLasFileFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    static std::vector<QString> exportToLasFiles( const QString&        exportFolder,
                                                  const QString&        filePrefix,
                                                  const RimWellLogPlot* plotWindow,
                                                  bool                  exportTvdRkb,
                                                  bool                  capitalizeFileNames,
                                                  bool                  alwaysOverwrite,
                                                  double                resampleInterval,
                                                  bool                  convertCurveUnits );

    static std::vector<QString> exportToLasFiles( const QString&                exportFolder,
                                                  const QString&                filePrefix,
                                                  std::vector<RimWellLogCurve*> curves,
                                                  const std::vector<QString>&   wellNames,
                                                  const std::vector<double>&    rkbDiffs,
                                                  bool                          capitalizeFileNames,
                                                  bool                          alwaysOverwrite,
                                                  double                        resampleInterval,
                                                  bool                          convertCurveUnits );

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};
