/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "RimFormationNames.h"

#include "RigFormationNames.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimTools.h"
#include "RimWellLogTrack.h"

#include "RifColorLegendData.h"

#include "RiuPlotMainWindowTools.h"

#include "cafAssert.h"
#include "cafPdmUiFilePathEditor.h"

#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

CAF_PDM_SOURCE_INIT( RimFormationNames, "FormationNames" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames::RimFormationNames()
{
    CAF_PDM_InitObject( "Formation Names", ":/Formations16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_formationNamesFileName, "FormationNamesFileName", "File Name", "", "", "" );

    m_formationNamesFileName.uiCapability()->setUiEditorTypeName( caf::PdmUiFilePathEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFormationNames::~RimFormationNames()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                          const QVariant&            oldValue,
                                          const QVariant&            newValue )
{
    if ( &m_formationNamesFileName == changedField )
    {
        updateUiTreeName();
        QString errorMessage;
        readFormationNamesFile( &errorMessage );
        if ( !errorMessage.isEmpty() )
        {
            QMessageBox::warning( nullptr, "Formation Names", errorMessage );
        }
        updateConnectedViews();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    updateUiTreeName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::setFileName( const QString& fileName )
{
    m_formationNamesFileName = fileName;

    updateUiTreeName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFormationNames::fileName()
{
    return m_formationNamesFileName().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFormationNames::fileNameWoPath()
{
    QFileInfo fnameFileInfo( m_formationNamesFileName().path() );
    return fnameFileInfo.fileName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::updateConnectedViews()
{
    std::vector<RimCase*> objects;
    this->objectsWithReferringPtrFieldsOfType( objects );

    for ( RimCase* caseObj : objects )
    {
        if ( caseObj )
        {
            caseObj->updateFormationNamesData();

            std::vector<RimWellLogTrack*> tracks;
            caseObj->objectsWithReferringPtrFieldsOfType( tracks );
            for ( RimWellLogTrack* track : tracks )
            {
                // The track may be referring to the case for other reasons than formations.
                if ( track->formationNamesCase() == caseObj )
                {
                    track->loadDataAndUpdate();
                }
            }
        }
    }
    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::readFormationNamesFile( QString* errorMessage )
{
    m_formationNamesData = RifColorLegendData::readFormationNamesFile( m_formationNamesFileName().path(), errorMessage );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath )
{
    // m_formationNamesFileName =
    //     RimTools::relocateFile( m_formationNamesFileName(), newProjectPath, oldProjectPath, nullptr, nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimFormationNames::layerZoneTableFileName()
{
    return "layer_zone_table.txt";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimFormationNames::updateUiTreeName()
{
    this->uiCapability()->setUiName( fileNameWoPath() );
}
