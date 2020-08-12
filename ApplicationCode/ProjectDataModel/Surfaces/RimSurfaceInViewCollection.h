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

namespace cvf
{
class ModelBasicList;
class Transform;
class ScalarMapper;
} // namespace cvf

class RimSurfaceInView;
class RimSurface;
class RimRegularLegendConfig;
class RiuViewer;

class RimSurfaceInViewCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimSurfaceInViewCollection();
    ~RimSurfaceInViewCollection() override;

    void updateFromSurfaceCollection();
    void loadData();
    void clearGeometry();

    void appendPartsToModel( cvf::ModelBasicList* surfaceVizModel, cvf::Transform* scaleTransform );
    void updateCellResultColor( bool hasGeneralCellResult, size_t timeStepIndex );
    void applySingleColorEffect();

    bool hasAnyActiveSeparateResults();
    void updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer );

    std::vector<RimRegularLegendConfig*> legendConfigs();

protected:
    virtual void initAfterRead() override;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    bool              hasSurfaceInViewForSurface( const RimSurface* surf ) const;
    RimSurfaceInView* getSurfaceInViewForSurface( const RimSurface* surf ) const;

    caf::PdmChildArrayField<RimSurfaceInView*> m_surfacesInView;
};
