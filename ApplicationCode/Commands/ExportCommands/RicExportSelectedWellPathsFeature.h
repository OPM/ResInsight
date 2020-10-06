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

#include <QFile>

#include <gsl/gsl>

#include <memory>

class RigWellPath;
class RigWellPathGeom;
class RimWellPath;
class RicExportWellPathsUi;
class QTextStream;

//==================================================================================================
///
//==================================================================================================
typedef std::shared_ptr<QFile>       QFilePtr;
typedef std::shared_ptr<QTextStream> QTextStreamPtr;

//==================================================================================================
///
//==================================================================================================
class RicExportSelectedWellPathsFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

    static void handleAction( const std::vector<RimWellPath*>& wellPaths );
    static void exportWellPath( gsl::not_null<const RimWellPath*> wellPath,
                                double                            mdStepSize,
                                const QString&                    folder,
                                bool                              writeProjectInfo = true );

    static RicExportWellPathsUi* openDialog();
    static QFilePtr              openFileForExport( const QString& folderName, const QString& fileName );
    static QTextStreamPtr        createOutputFileStream( QFile& file );

    static void writeWellPathGeometryToStream( QTextStream&                      stream,
                                               gsl::not_null<const RimWellPath*> wellPath,
                                               const QString&                    exportName,
                                               double                            mdStepSize,
                                               bool                              writeProjectInfo = true );

    static void writeWellPathGeometryToStream( QTextStream&       stream,
                                               const RigWellPath& wellPath,
                                               const QString&     exportName,
                                               double             mdStepSize,
                                               bool               useMdRkb,
                                               double             rkbOffset,
                                               bool               writeProjectInfo );

private:
    static void writeWellPathGeometryToStream( QTextStream&               stream,
                                               const QString&             exportName,
                                               const std::vector<double>& xValues,
                                               const std::vector<double>& yValues,
                                               const std::vector<double>& tvdValues,
                                               const std::vector<double>& mdValues,
                                               bool                       useMdRkb,
                                               bool                       writeProjectInfo );

    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};
