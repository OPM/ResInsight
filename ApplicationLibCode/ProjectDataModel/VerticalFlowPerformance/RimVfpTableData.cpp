/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimVfpTableData.h"

#include "RiaOpmParserTools.h"

#include "RigVfpTables.h"

#include "cafCmdFeatureMenuBuilder.h"

#include <QFileInfo>

CAF_PDM_SOURCE_INIT( RimVfpTableData, "RimVfpTableData" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimVfpTableData::RimVfpTableData()
{
    CAF_PDM_InitObject( "VFP Plot", ":/VfpPlot.svg" );

    CAF_PDM_InitFieldNoDefault( &m_filePath, "FilePath", "File Path" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpTableData::setFileName( const QString& filename )
{
    m_filePath = filename;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimVfpTableData::baseFileName()
{
    QFileInfo fileInfo( m_filePath().path() );
    return fileInfo.baseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpTableData::ensureDataIsImported()
{
    if ( m_vfpTables ) return;

    updateObjectName();

    m_vfpTables = std::make_unique<RigVfpTables>();

    const auto [vfpProdTables, vfpInjTables] = RiaOpmParserTools::extractVfpTablesFromDataFile( m_filePath().path().toStdString() );
    for ( const auto& prod : vfpProdTables )
    {
        m_vfpTables->addProductionTable( prod );
    }

    for ( const auto& inj : vfpInjTables )
    {
        m_vfpTables->addInjectionTable( inj );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimVfpTableData::tableCount() const
{
    if ( m_vfpTables )
    {
        return m_vfpTables->injectionTableNumbers().size() + m_vfpTables->productionTableNumbers().size();
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigVfpTables* RimVfpTableData::vfpTables() const
{
    return m_vfpTables.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpTableData::updateObjectName()
{
    QString name = "VFP Plots";

    QFileInfo fileInfo( m_filePath().path() );
    auto      fileName = fileInfo.fileName();
    if ( !fileName.isEmpty() )
    {
        name = fileName;
    }
    setName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimVfpTableData::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicNewVfpPlotFeature";
}
