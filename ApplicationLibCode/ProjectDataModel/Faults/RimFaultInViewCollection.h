/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"

#include <QString>

#include <vector>

class RimEclipseView;
class RimFaultInView;

//==================================================================================================
///
///
//==================================================================================================
class RimFaultInViewCollection : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum FaultFaceCullingMode
    {
        FAULT_BACK_FACE_CULLING,
        FAULT_FRONT_FACE_CULLING,
        FAULT_NO_FACE_CULLING
    };

public:
    RimFaultInViewCollection();
    ~RimFaultInViewCollection() override;

    bool isActive() const;
    void setActive( bool bActive );

    std::vector<RimFaultInView*>       faults() const;
    cvf::Color3f                       faultLabelColor() const;
    caf::AppEnum<FaultFaceCullingMode> faultResult() const;
    bool                               showFaultFaces() const;
    bool                               showFaultLabel() const;
    bool                               showOppositeFaultFaces() const;
    bool                               showNNCs() const;
    bool                               hideNNCsWhenNoResultIsAvailable() const;

    void setFaultResult( caf::AppEnum<FaultFaceCullingMode> resultType );
    void setShouldApplyCellFiltersToFaults( bool bEnabled );
    void setShowOppositeFaultFaces( bool bEnabled );
    void setShowFaultLabelWithFieldChanged( bool bEnabled );

    void synchronizeFaults();

    bool isGridVisualizationMode() const;
    bool shouldApplyCellFiltersToFaults() const;
    bool onlyShowFacesWithDefinedNeighbor() const;

    RimFaultInView* findFaultByName( QString name );

    void uiOrderingFaults( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    caf::PdmFieldHandle* objectToggleField() override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;
    void initAfterRead() override;

    RimEclipseView* parentView() const;

private:
    caf::PdmField<bool>         m_applyCellFilters;
    caf::PdmField<bool>         m_onlyShowWithNeighbor;
    caf::PdmField<bool>         m_showFaultFaces;
    caf::PdmField<bool>         m_showOppositeFaultFaces;
    caf::PdmField<bool>         m_showFaultLabel;
    caf::PdmField<cvf::Color3f> m_faultLabelColor;
    caf::PdmField<bool>         m_showFaultCollection;
    caf::PdmField<bool>         m_showNNCs;
    caf::PdmField<bool>         m_hideNNCsWhenNoResultIsAvailable;

    caf::PdmField<caf::AppEnum<FaultFaceCullingMode>> m_faultResult;

    caf::PdmChildArrayField<RimFaultInView*> m_faults;

    caf::PdmField<bool> m_showFaultsOutsideFilters_obsolete;
};
