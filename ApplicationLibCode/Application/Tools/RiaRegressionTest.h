/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RiaRegressionTest : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RiaRegressionTest();
    ~RiaRegressionTest() override;

    void writeSettingsToApplicationStore() const;
    void readSettingsFromApplicationStore();

public:
    caf::PdmField<QString> folderContainingCompareTool;
    caf::PdmField<QString> folderContainingDiffTool;
    caf::PdmField<QString> folderContainingGitTool;
    caf::PdmField<QString> regressionTestFolder;
    caf::PdmField<QString> testFilter;
    caf::PdmField<bool>    showInteractiveDiffImages;
    caf::PdmField<bool>    useOpenMPForGeometryCreation;
    caf::PdmField<bool>    openReportInBrowser;
    caf::PdmField<bool>    appendTestsAfterTestFilter;
    caf::PdmField<bool>    invalidateExternalFilePaths;
    caf::PdmField<bool>    activateObjectsInPropertyEditor;

    caf::PdmField<bool> exportSnapshots3dViews;
    caf::PdmField<bool> exportSnapshotsPlots;

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
};
