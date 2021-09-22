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

#include "cafPdmField.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

class RimSurface;
class RimEnsembleSurface;
class RimCase;

class RimSurfaceCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSurfaceCollection();
    ~RimSurfaceCollection() override;

    void addSurface( RimSurface* surface );
    void setAsTopmostFolder();

    void                             addEnsembleSurface( RimEnsembleSurface* ensembleSurface );
    std::vector<RimEnsembleSurface*> ensembleSurfaces() const;

    RimSurface* importSurfacesFromFiles( const QStringList& fileNames, bool showLegend = true );
    RimSurface* addGridCaseSurface( RimCase* sourceCase, int oneBasedSliceIndex = 1 );
    RimSurface* copySurfaces( std::vector<RimSurface*> surfaces );
    RimSurface* addSurfacesAtIndex( int index, std::vector<RimSurface*> surfaces );

    void                  addSubCollection( RimSurfaceCollection* collection );
    RimSurfaceCollection* getSubCollection( const QString& name ) const;
    void                  deleteSubCollection( const QString& name );

    bool containsFileSurface( QString filename );
    bool containsSurface();

    void reloadSurfaces( std::vector<RimSurface*> surfaces, bool showLegend = true );
    void removeSurface( RimSurface* surface );
    void removeMissingFileSurfaces();

    virtual void loadData();

    void updateViews();
    void updateViews( const std::vector<RimSurface*>& surfsToReload, bool showLegend = true );

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    QString collectionName() const;
    void    setCollectionName( const QString name );

    std::vector<RimSurface*>           surfaces() const;
    std::vector<RimSurfaceCollection*> subCollections() const;

protected:
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    void orderChanged( const caf::SignalEmitter* emitter );

    caf::PdmField<QString>                         m_collectionName;
    caf::PdmChildArrayField<RimSurface*>           m_surfaces;
    caf::PdmChildArrayField<RimSurfaceCollection*> m_subCollections;
};
