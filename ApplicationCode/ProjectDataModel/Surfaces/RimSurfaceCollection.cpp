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

#include "RimSurfaceCollection.h"
#include "QMessageBox"
#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RimProject.h"
#include "RimSurface.h"

CAF_PDM_SOURCE_INIT( RimSurfaceCollection, "SurfaceCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceCollection::RimSurfaceCollection()
{
    CAF_PDM_InitObject( "Surfaces", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_surfaces, "SurfacesField", "Surfaces", "", "", "" );
    m_surfaces.uiCapability()->setUiTreeHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceCollection::~RimSurfaceCollection() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::addSurface( RimSurface* surface )
{
    m_surfaces.push_back( surface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimSurfaceCollection::importSurfacesFromFiles( const QStringList& fileNames )
{
    QStringList              newFileNames;
    std::vector<RimSurface*> surfacesToReload;

    for ( const QString& newFileName : fileNames )
    {
        bool isFound = false;
        for ( RimSurface* surface : m_surfaces() )
        {
            if ( surface->surfaceFilePath() == newFileName )
            {
                surfacesToReload.push_back( surface );
                isFound = true;
                break;
            }
        }

        if ( !isFound )
        {
            newFileNames.push_back( newFileName );
        }
    }

    size_t  newSurfCount      = 0;
    size_t  existingSurfCount = m_surfaces().size();
    QString errorMessages;

    for ( const QString& newFileName : newFileNames )
    {
        RimSurface* newSurface = new RimSurface;

        auto newColor = RiaColorTables::categoryPaletteColors().cycledColor3f( existingSurfCount + newSurfCount );

        newSurface->setSurfaceFilePath( newFileName );
        newSurface->setColor( newColor );

        if ( !newSurface->updateDataFromFile() )
        {
            delete newSurface;
            errorMessages += newFileName + "\n";
        }
        else
        {
            this->addSurface( newSurface );
            surfacesToReload.push_back( newSurface );

            ++newSurfCount;
        }
    }

    if ( !errorMessages.isEmpty() )
    {
        QMessageBox::warning( nullptr, "Import Surfaces:", "Could not import the following files:\n" + errorMessages );
    }

    this->updateConnectedEditors();

    updateViews( surfacesToReload );

    if ( !newFileNames.empty() )
    {
        return m_surfaces[m_surfaces.size() - 1];
    }
    else
    {
        return nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSurfaceCollection::updateViews( const std::vector<RimSurface*>& surfsToReload )
{
    RimProject*               proj = RiaApplication::instance()->project();
    std::vector<RimGridView*> views;
    proj->allVisibleGridViews( views );

    for ( auto view : views )
    {
        // Update SurfaceInViewCollection
        // For each visible surfaceInView included in surfsToReload
        // LoadDataAndUpdate
    }
}
