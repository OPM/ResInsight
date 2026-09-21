/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "Polygons/RimPolygonContainer.h"

#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <QPointer>

class RimSumoDataSource;
class RimPolygonCloudAddress;
class RiaSumoConnector;
class Rim3dView;

//==================================================================================================
///
/// Root of a browsable tree of Sumo polygon results for one (case, ensemble, base realization),
/// created once via "Add Cloud Polygon Source". buildDirectoryTree() does a single, cheap
/// polygon-result-directory metadata fetch and builds the *entire* folder/leaf structure (Field
/// Outline / Structure Depth Fault Lines + names / Fluid Contact Outline + names + contact types)
/// as RimPolygonCloudFolder/RimPolygonCloudAddress children immediately -- no coordinate data is
/// fetched at this point. Each RimPolygonCloudAddress leaf only fetches its own RimCloudPolygon
/// coordinate data lazily, the first time a 3D view's visibility checkbox for that leaf is checked
/// on (see RimPolygonInViewCollection).
///
/// This is also the only polygon container type offering a per-view "Auto-Follow View Realization"
/// choice (supportsRealizationOverride()) -- it governs every leaf beneath it in a given view, so
/// the choice is only ever shown/asked once per source, not once per leaf.
///
//==================================================================================================
class RimPolygonCloudSource : public RimPolygonContainer
{
    CAF_PDM_HEADER_INIT;

public:
    RimPolygonCloudSource();

    void setDataSource( RimSumoDataSource* dataSource );
    void setBaseRealization( int realization );

    RimSumoDataSource* dataSource() const;
    int                baseRealization() const;

    // Fetches the polygon result directory once (metadata only -- no coordinate arrays) and builds
    // the full nested RimPolygonCloudFolder/RimPolygonCloudAddress tree beneath this object. No-op
    // if already built (see m_directoryBuilt).
    void buildDirectoryTree();

    QString name() const;

    bool                 canAddSubCollection() const override;
    RimPolygonContainer* addNewSubCollection() override;

    bool             supportsRealizationOverride() const override;
    std::vector<int> availableRealizationIdsForOverride() const override;
    int              resolveViewMatchingRealization( const Rim3dView* view ) const override;

    // Evicts any fetched RimCloudPolygon data (base or per-realization-group) beneath this source
    // that is not currently shown checked in any open view. Called after the Auto-Follow checkbox
    // is turned off in a view, since realization groups created while it was on may otherwise
    // linger unused; safe to call at any time.
    void evictUnusedRealizationData();

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;

private:
    RiaSumoConnector*                    sumoConnector();
    void                                 updateName();
    std::vector<RimPolygonCloudAddress*> allAddresses() const;

private:
    caf::PdmPtrField<RimSumoDataSource*> m_dataSource;
    caf::PdmField<int>                   m_baseRealization;
    caf::PdmField<bool>                  m_directoryBuilt;

    QPointer<RiaSumoConnector> m_sumoConnector;
};
