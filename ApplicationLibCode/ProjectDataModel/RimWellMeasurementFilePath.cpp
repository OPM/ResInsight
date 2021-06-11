/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimWellMeasurementFilePath.h"

#include "cafPdmUiLineEditor.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimWellMeasurementFilePath, "WellMeasurementFilePath" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementFilePath::RimWellMeasurementFilePath()
{
    CAF_PDM_InitObject( "RimWellMeasurementFilePath", ":/WellMeasurement16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_userDescription, "UserDecription", "Name", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_filePath, "FilePath", "File Path", "", "", "" );
    m_filePath.uiCapability()->setUiReadOnly( true );
    m_filePath.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementFilePath::~RimWellMeasurementFilePath()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellMeasurementFilePath::filePath() const
{
    return m_filePath.v().path();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellMeasurementFilePath::setFilePath( const QString& filePath )
{
    m_filePath = filePath;
    if ( m_userDescription().isEmpty() )
    {
        m_userDescription = QFileInfo( filePath ).fileName();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimWellMeasurementFilePath::userDescriptionField()
{
    return &m_userDescription;
}
