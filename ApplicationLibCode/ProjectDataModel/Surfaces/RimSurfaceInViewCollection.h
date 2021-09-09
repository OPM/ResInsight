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

#include "RimCheckableNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

namespace cvf
{
class ModelBasicList;
class Transform;
class ScalarMapper;
} // namespace cvf

class RimSurfaceInView;
class RimSurface;
class RimSurfaceCollection;
class RimEnsembleSurface;
class RimRegularLegendConfig;
class RiuViewer;
class RivIntersectionGeometryGeneratorIF;

class RimSurfaceInViewCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSurfaceInViewCollection();
    ~RimSurfaceInViewCollection() override;

    QString name() const override;

    RimSurfaceCollection* surfaceCollection() const;
    void                  setSurfaceCollection( RimSurfaceCollection* surfcoll );

    void updateFromSurfaceCollection();
    void loadData();
    void clearGeometry();

    void appendPartsToModel( cvf::ModelBasicList* surfaceVizModel, cvf::Transform* scaleTransform );
    void updateCellResultColor( bool hasGeneralCellResult, size_t timeStepIndex );
    void applySingleColorEffect();

    bool hasAnyActiveSeparateResults();
    void updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer );

    std::vector<RimRegularLegendConfig*> legendConfigs();

    std::vector<const RivIntersectionGeometryGeneratorIF*> intersectionGeometryGenerators() const;

protected:
    void                 initAfterRead() override;
    caf::PdmFieldHandle* userDescriptionField() override;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    RimSurfaceInView*           getSurfaceInViewForSurface( const RimSurface* surf ) const;
    RimSurfaceInViewCollection* getCollectionInViewForCollection( const RimSurfaceCollection* coll ) const;

    void updateAllViewItems();
    void syncCollectionsWithView();
    void syncSurfacesWithView();

private:
    caf::PdmProxyValueField<QString>                     m_collectionName;
    caf::PdmChildArrayField<RimSurfaceInView*>           m_surfacesInView;
    caf::PdmChildArrayField<RimSurfaceInViewCollection*> m_collectionsInView;

    caf::PdmPtrField<RimSurfaceCollection*> m_surfaceCollection;
};
