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

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicEclipseCellResultToFileImpl.h"
#include "RicExportFeatureImpl.h"

#include "RigCaseCellResultsData.h"

#include "RimBinaryExportSettings.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafUtils.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicSaveEclipseResultAsInputPropertyExec::RicSaveEclipseResultAsInputPropertyExec( RimEclipseCellColors* cellColors )
    : CmdExecuteCommand( nullptr )
{
    CVF_ASSERT( cellColors );
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
    CVF_ASSERT( m_cellColors );

    if ( !m_cellColors->reservoirView() ) return;
    if ( !m_cellColors->reservoirView()->eclipseCase() ) return;
    if ( !m_cellColors->reservoirView()->eclipseCase()->eclipseCaseData() ) return;

    RimBinaryExportSettings exportSettings;
    exportSettings.eclipseKeyword = m_cellColors->resultVariable();
    {
        RiaApplication* app           = RiaApplication::instance();
        QString         projectFolder = app->currentProjectPath();
        if ( projectFolder.isEmpty() )
        {
            projectFolder = m_cellColors->reservoirView()->eclipseCase()->locationOnDisc();
        }

        QString outputFileName = projectFolder + "/" +
                                 caf::Utils::makeValidFileBasename( m_cellColors->resultVariableUiShortName() );

        exportSettings.fileName = outputFileName;
    }

    caf::PdmUiPropertyViewDialog propertyDialog( Riu3DMainWindowTools::mainWindowWidget(),
                                                 &exportSettings,
                                                 "Export Binary Eclipse Data to Text File",
                                                 "" );
    RicExportFeatureImpl::configureForExport( propertyDialog.dialogButtonBox() );

    if ( propertyDialog.exec() == QDialog::Accepted )
    {
        size_t timeStep = m_cellColors->reservoirView()->currentTimeStep();

        QString errMsg;
        bool    isOk = RicEclipseCellResultToFileImpl::writeBinaryResultToTextFile( exportSettings.fileName,
                                                                                 m_cellColors->reservoirView()
                                                                                     ->eclipseCase()
                                                                                     ->eclipseCaseData(),
                                                                                 timeStep,
                                                                                 m_cellColors,
                                                                                 exportSettings.eclipseKeyword,
                                                                                 exportSettings.undefinedValue,
                                                                                 "saveEclipseResultAsInputPropertyExec",
                                                                                 &errMsg );
        if ( !isOk )
        {
            QString fullError = QString( "Failed to exported current result to %1. Error was: %2" )
                                    .arg( exportSettings.fileName )
                                    .arg( errMsg );
            RiaLogging::error( fullError );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseResultAsInputPropertyExec::undo()
{
    // TODO
    CVF_ASSERT( 0 );
}
