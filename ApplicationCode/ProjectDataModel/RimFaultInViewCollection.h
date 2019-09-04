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

class RimEclipseView;
class RimFaultInView;
class RimNoCommonAreaNncCollection;

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

    void                                syncronizeFaults();

    bool                                isGridVisualizationMode() const;
    
    bool                                isShowingFaultsAndFaultsOutsideFilters() const;
    void                                setShowFaultsOutsideFilter(bool show);

    caf::PdmField<bool>                 showFaultFaces;
    caf::PdmField<bool>                 showOppositeFaultFaces;
    
    caf::PdmField<caf::AppEnum< FaultFaceCullingMode > > faultResult;


    caf::PdmField<bool>                 showFaultLabel;
    caf::PdmField<cvf::Color3f>         faultLabelColor;

    caf::PdmField<bool>                 showFaultCollection;
    caf::PdmField<bool>                 showNNCs;
    caf::PdmField<bool>                 hideNncsWhenNoResultIsAvailable;

    caf::PdmChildArrayField<RimFaultInView*>    faults;
    RimFaultInView*                           findFaultByName(QString name);

    caf::PdmChildField<RimNoCommonAreaNncCollection*> noCommonAreaNnncCollection;


private:
    void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    caf::PdmFieldHandle*    objectToggleField() override;
    void                    defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    RimEclipseView*         parentView() const;

private:
    caf::PdmField<bool>     m_showFaultsOutsideFilters;
};
