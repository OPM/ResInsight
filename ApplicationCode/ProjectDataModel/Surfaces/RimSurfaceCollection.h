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

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"

class RimSurface;
class RimCase;

class RimSurfaceCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSurfaceCollection();
    ~RimSurfaceCollection() override;

    void addSurface( RimSurface* surface );

    RimSurface* importSurfacesFromFiles( const QStringList& fileNames );
    RimSurface* addGridCaseSurface( RimCase* sourceCase );

    void reloadSurfaces( std::vector<RimSurface*> surfaces );
    void removeSurface( RimSurface* surface );
    void addSurfacesAtPosition( int position, std::vector<RimSurface*> surfaces );

    void loadData();
    void updateViews();
    void updateViews( const std::vector<RimSurface*>& surfsToReload );

    void onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                         std::vector<caf::PdmObjectHandle*>& referringObjects ) override;

    std::vector<RimSurface*> surfaces() const;

private:
    void orderChanged( const caf::SignalEmitter* emitter );

    caf::PdmChildArrayField<RimSurface*> m_surfaces;
};
