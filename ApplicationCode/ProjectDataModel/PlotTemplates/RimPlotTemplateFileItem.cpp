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

#include "RimPlotTemplateFileItem.h"

#include "RiaFieldHandleTools.h"
#include "RiaLogging.h"

#include "cafPdmField.h"
#include "cafPdmUiFilePathEditor.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimPlotTemplateFileItem, "PlotTemplateFileItem" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotTemplateFileItem::RimPlotTemplateFileItem()
{
    CAF_PDM_InitObject( "PlotTemplateFileItem", ":/SummaryTemplate16x16.png", "Plot Template", "" );

    CAF_PDM_InitField( &m_absoluteFileName, "AbsolutePath", QString(), "Location", "", "", "" );
    m_absoluteFileName.uiCapability()->setUiReadOnly( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotTemplateFileItem::~RimPlotTemplateFileItem()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPlotTemplateFileItem::setFilePath( const QString& filePath )
{
    QFileInfo fi( filePath );
    this->uiCapability()->setUiName( fi.baseName() );

    m_absoluteFileName = filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimPlotTemplateFileItem::absoluteFilePath() const
{
    return m_absoluteFileName();
}
