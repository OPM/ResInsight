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

#include "RicSaveEclipseResultAsInputPropertyExec.h"

#include "RimEclipseCellColors.h"
#include "RimBinaryExportSettings.h"
#include "RimEclipseView.h"
#include "RimEclipseCase.h"

#include "RigCaseCellResultsData.h"

#include "RifEclipseInputFileTools.h"
#include "RifReaderInterface.h"

#include "RiaApplication.h"
#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"

#include <QFileInfo>
#include <QMessageBox>
#include "cafUtils.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSaveEclipseResultAsInputPropertyExec::RicSaveEclipseResultAsInputPropertyExec(RimEclipseCellColors* cellColors)
    : CmdExecuteCommand(NULL)
{
    CVF_ASSERT(cellColors);
    m_cellColors = cellColors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSaveEclipseResultAsInputPropertyExec::~RicSaveEclipseResultAsInputPropertyExec()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicSaveEclipseResultAsInputPropertyExec::name()
{
    return "Export Property To File";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseResultAsInputPropertyExec::redo()
{
    CVF_ASSERT(m_cellColors);

    if (!m_cellColors->reservoirView()) return;
    if (!m_cellColors->reservoirView()->eclipseCase()) return;
    if (!m_cellColors->reservoirView()->eclipseCase()->eclipseCaseData()) return;

    RimBinaryExportSettings exportSettings;
    exportSettings.eclipseKeyword = m_cellColors->resultVariable();
    {
        RiaApplication* app = RiaApplication::instance();
        QString projectFolder = app->currentProjectPath();
        if (projectFolder.isEmpty())
        {
            projectFolder = m_cellColors->reservoirView()->eclipseCase()->locationOnDisc();
        }

        QString outputFileName = projectFolder + "/" + caf::Utils::makeValidFileBasename( m_cellColors->resultVariableUiShortName());

        exportSettings.fileName = outputFileName;
    }

    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &exportSettings, "Export Binary Eclipse Data to Text File", "");

    if (propertyDialog.exec() == QDialog::Accepted)
    {
        size_t timeStep = m_cellColors->reservoirView()->currentTimeStep();

        bool isOk = RifEclipseInputFileTools::writeBinaryResultToTextFile(exportSettings.fileName, m_cellColors->reservoirView()->eclipseCase()->eclipseCaseData(), timeStep, m_cellColors, exportSettings.eclipseKeyword, exportSettings.undefinedValue);
        if (!isOk)
        {
            QMessageBox::critical(NULL, "File export", "Failed to exported current result to " + exportSettings.fileName);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseResultAsInputPropertyExec::undo()
{
    // TODO
    CVF_ASSERT(0);
}
