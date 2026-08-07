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
#pragma once

#include "cafPdmNestedCollection.h"

class RimSurface;
class RimEnsembleSurface;
class RimCase;

class RimSurfaceCollection : public caf::PdmNestedCollection<RimSurfaceCollection, RimSurface>
{
    CAF_PDM_HEADER_INIT;

public:
    RimSurfaceCollection();
    ~RimSurfaceCollection() override;

    // Construct the single top-level surface collection. Marks the instance as the topmost folder
    // and applies the branded surface icon. Sub-folders use the default folder icon from the
    // constructor.
    static RimSurfaceCollection* createTopmost();

    void addSurface( RimSurface* surface );

    void                             addEnsembleSurface( RimEnsembleSurface* ensembleSurface );
    std::vector<RimEnsembleSurface*> ensembleSurfaces() const;

    RimSurface* importSurfacesFromFiles( const QStringList& fileNames, bool showLegend = true );
    RimSurface* addGridCaseSurface( RimCase* sourceCase, int oneBasedSliceIndex = 1 );
    RimSurface* copySurfaces( std::vector<RimSurface*> surfaces );
    RimSurface* addSurfacesAtIndex( int index, std::vector<RimSurface*> surfaces );

    static RimSurface* createSurfaceFromFile( const QString& fileName );

    bool containsFileSurface( QString filename );
    bool containsSurface();

    void reloadSurfaces( std::vector<RimSurface*> surfaces, bool showLegend = true );
    void removeSurface( RimSurface* surface );
    void removeMissingFileSurfaces();

    virtual void loadData();

    void updateViews();
    void updateViews( const std::vector<RimSurface*>& surfsToReload, bool showLegend = true );

    void onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    std::vector<RimSurface*> surfaces() const;

    // Renames the surface if another surface in this folder already carries the same name.
    void ensureUniqueSurfaceName( RimSurface* surface );

protected:
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

    // Enforces name uniqueness among sibling folders when the folder is renamed.
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

private:
    void orderChanged( const caf::SignalEmitter* emitter );
};
