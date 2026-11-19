/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RimOpmFlowJobFileHelper.h"

#include <QFileInfo>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOpmFlowJobFileHelper::RimOpmFlowJobFileHelper()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOpmFlowJobFileHelper::~RimOpmFlowJobFileHelper()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJobFileHelper::setWorkingDirectory( QString workingDir )
{
    m_workingDirectory = workingDir;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJobFileHelper::setName( QString name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJobFileHelper::setUseEnsembleNames( int currentEnsembleId )
{
    m_currentEnsembleId = currentEnsembleId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJobFileHelper::deckName() const
{
    auto deckName = baseDeckName();
    if ( m_currentEnsembleId >= 0 )
    {
        deckName = deckName + "-" + QString::number( m_currentEnsembleId );
    }

    return deckName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJobFileHelper::baseDeckName() const
{
    QString baseName = m_name;
    baseName.replace( ' ', '_' );
    baseName = baseName.toUpper();
    return baseName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJobFileHelper::restartDeckName( QString inputDeckName ) const
{
    return inputDeckName + "_RST";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJobFileHelper::fluxnumFileName( bool relativePath ) const
{
    QString filename = "FLUXNUM.grdecl";
    if ( !relativePath )
    {
        filename = m_workingDirectory + "/" + filename;
    }
    return filename;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJobFileHelper::fluxFileName() const
{
    return workingDirectory() + "/" + deckName() + ".FLUX";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJobFileHelper::deckExtension() const
{
    return ".DATA";
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimOpmFlowJobFileHelper::workingDirectory() const
{
    if ( m_currentEnsembleId < 0 )
    {
        return m_workingDirectory;
    }
    else
    {
        return QString( "%1/run-%2" ).arg( m_workingDirectory ).arg( m_currentEnsembleId );
    }
}
