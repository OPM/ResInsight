/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"
#include "cvfVector3.h"

#include <utility>
#include <vector>

class RimIntersectionResultDefinition;
class RivIntersectionHexGridInterface;
class RimIntersectionResultsDefinitionCollection;
class RimSurfaceIntersectionBand;
class RimSurfaceIntersectionCollection;
class RimSurfaceIntersectionCurve;
class RivIntersectionGeometryGeneratorInterface;

namespace caf
{
class CmdFeatureMenuBuilder;
class PdmUiTreeOrdering;
} // namespace caf

//==================================================================================================
/// The curtain of an intersection, described at a set of positions along it
//==================================================================================================
class RimIntersectionCurtain
{
public:
    /// The polyline the curve is resampled along, in 3D. Kept apart from the pillars so the samples
    /// are distributed along the intersection itself, also where the pillars are long.
    std::vector<cvf::Vec3d> trace;

    /// For each point of the trace, the segment the surface is looked up along, from the top to the
    /// bottom of the curtain. Tilted for an intersection that follows the grid pillars, which is
    /// what puts the curve on the curtain instead of on a vertical plane through it.
    std::vector<std::pair<cvf::Vec3d, cvf::Vec3d>> pillars;

    bool isValid() const { return trace.size() == pillars.size() && trace.size() > 1; }
};

class RimIntersection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimIntersection();
    ~RimIntersection() override;

    virtual QString name() const = 0;

    bool isActive() const;
    void setActive( bool isActive );
    bool isInactiveCellsVisible() const;

    RimIntersectionResultDefinition*          activeSeparateResultDefinition();
    cvf::ref<RivIntersectionHexGridInterface> createHexGridInterface();

    virtual const RivIntersectionGeometryGeneratorInterface* intersectionGeometryGenerator() const = 0;

    RimSurfaceIntersectionCollection*         surfaceIntersectionCollection() const;
    std::vector<RimSurfaceIntersectionCurve*> surfaceIntersectionCurves() const;
    std::vector<RimSurfaceIntersectionBand*>  surfaceIntersectionBands() const;
    RimSurfaceIntersectionCurve*              addIntersectionCurve();
    RimSurfaceIntersectionBand*               addIntersectionBand();

    /// True if the intersection is a vertical curtain, and therefore can display surface intersection
    /// curves and bands. Horizontal sections would require contouring instead of a vertical ray cast.
    virtual bool supportsSurfaceIntersectionCurves() const;

    virtual RimIntersectionCurtain surfaceCurtain() const;

    virtual void rebuildGeometryAndScheduleCreateDisplayModel();

protected:
    virtual RimIntersectionResultsDefinitionCollection* findSeparateResultsCollection();

    caf::PdmFieldHandle*          objectToggleField() final;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void defineSeparateDataSourceUi( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );
    void updateDefaultSeparateDataSource();

    void appendSurfaceIntersectionsToTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering );

    /// The context menu entries shared by all intersection types, ending with a separator so the
    /// caller can add its own entries before the trailing copy-to-all-views command
    void appendCommonMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const;

    /// A curtain that is a vertical extrusion of the trace, spanning [bottomZ, topZ]
    static RimIntersectionCurtain verticalCurtain( const std::vector<cvf::Vec3d>& trace, double topZ, double bottomZ );

    /// Reaches past any reservoir depth, used when the curtain has no defined vertical extent
    static double defaultCurtainExtent();

    caf::PdmField<bool>                                m_isActive;
    caf::PdmField<bool>                                m_showInactiveCells;
    caf::PdmField<bool>                                m_useSeparateDataSource;
    caf::PdmPtrField<RimIntersectionResultDefinition*> m_separateDataSource;

    caf::PdmChildField<RimSurfaceIntersectionCollection*> m_surfaceIntersections;

private:
    void onSurfaceIntersectionsChanged( const caf::SignalEmitter* emitter );
};
