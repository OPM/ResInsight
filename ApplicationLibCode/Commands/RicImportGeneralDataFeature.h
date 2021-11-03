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

#include "RiaDefines.h"

#include "cafCmdFeature.h"

#include <QStringList>

#include <memory>
#include <vector>

class RifReaderSettings;

//==================================================================================================
///
//==================================================================================================
class RicImportGeneralDataFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

public:
    struct OpenCaseResults
    {
        QStringList eclipseCaseFiles;
        QStringList eclipseInputFiles;
        QStringList eclipseSummaryFiles;

        std::vector<int> createdCaseIds;

        operator bool() const
        {
            return !( eclipseCaseFiles.empty() && eclipseInputFiles.empty() && eclipseSummaryFiles.empty() );
        }
    };

    static OpenCaseResults openEclipseFilesFromFileNames( const QStringList&                 fileNames,
                                                          bool                               doCreateDefaultPlot,
                                                          bool                               createDefaultView,
                                                          std::shared_ptr<RifReaderSettings> readerSettings = nullptr );
    static QStringList     fileNamesFromCaseNames( const QStringList& caseNames );
    static QStringList     getEclipseFileNamesWithDialog( RiaDefines::ImportFileType fileTypes );

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

    static void openFileDialog( RiaDefines::ImportFileType fileTypes );

    static bool openEclipseCaseFromFileNames( const QStringList&                 fileNames,
                                              bool                               createDefaultView,
                                              std::vector<int>&                  createdCaseIds,
                                              std::shared_ptr<RifReaderSettings> readerSettings );
    static bool openInputEclipseCaseFromFileNames( const QStringList& fileNames,
                                                   bool               createDefaultView,
                                                   std::vector<int>&  createdCaseIds );
    static bool openSummaryCaseFromFileNames( const QStringList& fileNames, bool doCreateDefaultPlot = true );
};
