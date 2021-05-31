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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include "cvfObject.h"

#include "RimIntersection.h"

class RimSurface;
class RimSurfaceResultDefinition;
class RivSurfacePartMgr;
class RivIntersectionHexGridInterface;
class RiuViewer;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RimSurfaceInView : public RimIntersection
{
    CAF_PDM_HEADER_INIT;

public:
    RimSurfaceInView();
    ~RimSurfaceInView() override;

    QString     name() const override;
    RimSurface* surface() const;
    void        setSurface( RimSurface* surf );

    bool                        isNativeSurfaceResultsActive() const;
    RimSurfaceResultDefinition* surfaceResultDefinition();

    void                                      clearGeometry();
    RivSurfacePartMgr*                        surfacePartMgr();
    const RivIntersectionGeometryGeneratorIF* intersectionGeometryGenerator() const override;

    void loadDataAndUpdate();

    void updateLegendRangesTextAndVisibility( RiuViewer* nativeOrOverrideViewer, bool isUsingOverrideViewer );

protected:
    void initAfterRead() override;

private:
    RimIntersectionResultsDefinitionCollection* findSeparateResultsCollection() override;

    caf::PdmFieldHandle* userDescriptionField() override;
    void                 fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                 defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    caf::PdmProxyValueField<QString> m_name;
    caf::PdmPtrField<RimSurface*>    m_surface;

    caf::PdmChildField<RimSurfaceResultDefinition*> m_resultDefinition;

    cvf::ref<RivSurfacePartMgr> m_surfacePartMgr;
};
