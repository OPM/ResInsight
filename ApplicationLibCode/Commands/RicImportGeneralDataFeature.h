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

struct RifReaderSettings;

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
        QStringList roffFiles;

        std::vector<int> createdCaseIds;

        operator bool() const
        {
            return !( eclipseCaseFiles.empty() && eclipseInputFiles.empty() && eclipseSummaryFiles.empty() && roffFiles.empty() );
        }
    };

    static OpenCaseResults openEclipseFilesFromFileNames( const QStringList& fileNames, bool doCreateDefaultPlot, bool createDefaultView );
    static OpenCaseResults openEclipseFilesFromFileNames( const QStringList& fileNames,
                                                          bool               doCreateDefaultPlot,
                                                          bool               createDefaultView,
                                                          RifReaderSettings& readerSettings );
    static QStringList     fileNamesFromCaseNames( const QStringList& caseNames );
    static QStringList     getEclipseFileNamesWithDialog( RiaDefines::ImportFileType fileTypes );

    static QString getFilePattern( const std::vector<RiaDefines::ImportFileType>& fileTypes, bool allowWildcard );

protected:
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;

    static QString getFilePattern( RiaDefines::ImportFileType fileType );

    static void openFileDialog( RiaDefines::ImportFileType fileTypes );

    static bool openEclipseCaseFromFileNames( const QStringList& fileNames,
                                              bool               createDefaultView,
                                              std::vector<int>&  createdCaseIds,
                                              RifReaderSettings& readerSettings );

    static bool openSummaryCaseFromFileNames( const QStringList& fileNames, bool doCreateDefaultPlot = true );

    static bool openEclipseInputFilesFromFileNames( const QStringList& fileNames, bool createDefaultView, std::vector<int>& createdCaseIds );
    static bool openGrdeclCasesFromFileNames( const QStringList& fileNames, bool createDefaultView, std::vector<int>& createdCaseIds );
    static bool
        openGrdeclCaseAndPropertiesFromFileNames( const QStringList& fileNames, bool createDefaultView, std::vector<int>& createdCaseIds );

    static bool openRoffFilesFromFileNames( const QStringList& fileNames, bool createDefaultView, std::vector<int>& createdCaseIds );
    static bool openRoffCasesFromFileNames( const QStringList& fileNames, bool createDefaultView, std::vector<int>& createdCaseIds );
    static bool openRoffCaseAndPropertiesFromFileNames( const QStringList& fileNames, bool createDefaultView, std::vector<int>& createdCaseIds );
};
