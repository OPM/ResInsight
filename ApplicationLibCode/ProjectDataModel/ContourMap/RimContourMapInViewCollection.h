/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include <vector>

class RimContourMapInView;
class RimEclipseContourMapView;
class RimRegularLegendConfig;
class RiuViewer;

namespace cvf
{
class Camera;
class ModelBasicList;
} // namespace cvf

namespace caf
{
class DisplayCoordTransform;
}

//==================================================================================================
///
/// The contour maps of the project, mirrored into a single 3d view.
///
/// The project level collection of contour map views is the owner of the contour maps. This
/// collection holds one wrapper per contour map, carrying the presentation state that belongs to
/// this view only. Wrappers start out unchecked, so a view shows nothing until asked to.
///
//==================================================================================================
class RimContourMapInViewCollection : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimContourMapInViewCollection();
    ~RimContourMapInViewCollection() override;

    void updateFromContourMapCollection();

    std::vector<RimContourMapInView*> allContourMapsInView() const;
    std::vector<RimContourMapInView*> visibleContourMapsInView() const;

    void appendPartsToModel( cvf::ModelBasicList* model, const caf::DisplayCoordTransform* displayCoordTransform, const cvf::Camera* camera );

    std::vector<RimRegularLegendConfig*> legendConfigs() const;
    void updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer );

    // Redraw every 3d view mirroring the given contour map. Reading a project loads the ordinary 3d
    // views before the contour map views, so a 3d view can draw a contour map whose own view has not
    // been loaded yet, and reads it as empty. Calling this once the contour map view is ready lets
    // those views rebuild against real data.
    static void scheduleRedrawOfViewsShowing( const RimEclipseContourMapView* contourMapView );

    // Let every 3d view pick up contour maps that have been added or removed
    static void updateViewTreeItemsInAllViews();

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void initAfterRead() override;

private:
    RimContourMapInView* findContourMapInViewForSource( const RimEclipseContourMapView* contourMapView ) const;

    // Every contour map of the project, in a stable order. Includes the ensemble statistics contour
    // maps, which live on their ensemble rather than in the oil field collection.
    static std::vector<RimEclipseContourMapView*> allContourMapViewsInProject();

private:
    caf::PdmChildArrayField<RimContourMapInView*> m_contourMapsInView;
};
