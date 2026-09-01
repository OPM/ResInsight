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

#include "RiaFontCache.h"

#include "cafAppEnum.h"
#include "cafPdmFieldCvfColor.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include "cvfArray.h"
#include "cvfObject.h"

#include <memory>
#include <optional>
#include <vector>

class RigContourMapGrid;
class RigContourMapTopography;
class RigMainGrid;
class RigSurface;
class RimEclipseContourMapView;
class RimContourMapProjection;
class RimRegularLegendConfig;
class RivContourMapProjectionPartMgr;
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
/// A contour map shown inside a 3d view.
///
/// This is a presentation wrapper only. The contour map itself, including the result, the
/// aggregation and the computed geometry, is owned by the contour map view this object points at.
/// The wrapper decides where the map is placed vertically in the 3d scene and what parts of it are
/// drawn.
///
//==================================================================================================
class RimContourMapInView : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    using SourceItemT = RimEclipseContourMapView;

    enum class MapPosition
    {
        TOP_OF_CASE,
        BOTTOM_OF_CASE,
        USER_DEFINED_DEPTH
    };

    enum class LineColorMode
    {
        CONTRAST_TO_MAP,
        SINGLE_COLOR
    };

    RimContourMapInView();
    ~RimContourMapInView() override;

    // Follows the name of the contour map this wrapper points at
    QString name() const override;

    RimEclipseContourMapView* contourMapView() const;
    RimEclipseContourMapView* sourceItem() const;
    void                      setContourMapView( RimEclipseContourMapView* contourMapView );

    void appendPartsToModel( cvf::ModelBasicList* model, const caf::DisplayCoordTransform* displayCoordTransform, const cvf::Camera* camera );

    RimRegularLegendConfig* legendConfig() const;
    void                    updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer );

    // Drops the drawables only. The topography raster depends on the host view, not on the contour map,
    // and is expensive enough to be worth keeping when only the contour map itself has changed.
    void clearPartMgr();
    void clearGeometry();

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void initAfterRead() override;

    caf::PdmFieldHandle* userDescriptionField() override;

private:
    RimContourMapProjection* contourMapProjection() const;

    // The domain z the contour map is placed on, including the depth offset.
    std::optional<double> mapElevation() const;

    double mapSampleSpacing() const;

    // Draped geometry is lifted clear of the geometry it follows, since coplanar geometry z-fights.
    // The map surface is lifted the least, and the contour lines sit above it.
    double surfaceDrapeLift() const;
    double contourLineLift() const;

    // Lazily built, and rebuilt whenever the geometry is cleared.
    std::shared_ptr<const RigContourMapTopography> topography();

    RivContourMapProjectionPartMgr* partMgr();

private:
    caf::PdmPtrField<RimEclipseContourMapView*> m_contourMapView;
    caf::PdmProxyValueField<QString>            m_nameProxy;

    caf::PdmField<caf::AppEnum<MapPosition>> m_mapPosition;
    caf::PdmField<double>                    m_depthOffset;
    caf::PdmField<double>                    m_userDefinedDepth;

    caf::PdmField<bool> m_showMapSurface;
    caf::PdmField<bool> m_showContourLines;
    caf::PdmField<bool> m_projectSurfaceOnGeometry;
    caf::PdmField<bool> m_projectLinesOnGeometry;

    caf::PdmField<bool>                       m_showContourLabels;
    caf::PdmField<RiaFontCache::FontSizeEnum> m_labelFontSize;

    caf::PdmField<caf::AppEnum<LineColorMode>> m_lineColorMode;
    caf::PdmField<cvf::Color3f>                m_lineColor;
    caf::PdmField<int>                         m_lineThickness;

    cvf::ref<RivContourMapProjectionPartMgr>       m_partMgr;
    std::shared_ptr<const RigContourMapTopography> m_topography;

    // What the cached topography was built from. The raster is expensive, so it is kept until one of
    // these changes. All are rebuilt or replaced as objects by their owners, so comparing identity is
    // enough.
    cvf::cref<cvf::UByteArray> m_topographyCellVisibility;
    const RigContourMapGrid*   m_topographyMapGrid  = nullptr;
    const RigMainGrid*         m_topographyMainGrid = nullptr;
    std::vector<RigSurface*>   m_topographySurfaces;
};
