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

#include <vector>

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

        operator bool() const
        {
            return !(eclipseCaseFiles.empty() && eclipseInputFiles.empty() && eclipseSummaryFiles.empty());
        }
    };

    static OpenCaseResults openEclipseFilesFromFileNames(const QStringList& fileNames);

protected:

    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered(bool isChecked) override;
    void setupActionLook(QAction* actionToSetup) override;

    static void openFileDialog(RiaDefines::ImportFileType fileTypes);

    static bool openEclipseCaseFromFileNames(const QStringList& fileNames);
    static bool openInputEclipseCaseFromFileNames(const QStringList& fileNames);
    static bool openSummaryCaseFromFileNames(const QStringList& fileNames);

};
