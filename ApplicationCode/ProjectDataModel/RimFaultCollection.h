/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafAppEnum.h"
#include <QString>

#include "cvfBase.h"
#include "cvfColor3.h"

#include "RimFault.h"

class RimReservoirView;


//==================================================================================================
///  
///  
//==================================================================================================
class RimFaultCollection : public caf::PdmObject
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
    RimFaultCollection();
    virtual ~RimFaultCollection();

    void                                setReservoirView(RimReservoirView* ownerReservoirView);
    void                                syncronizeFaults();

    bool                                isGridVisualizationMode() const;

    caf::PdmField<bool>                 showGeometryDetectedFaults; // Obsolete, to be removed

    caf::PdmField<bool>                 showFaultFaces;
    caf::PdmField<bool>                 showOppositeFaultFaces;
    caf::PdmField<bool>                 showFaultsOutsideFilters;
    caf::PdmField<bool>                 showNNCs;
    caf::PdmField<bool>                 forceFaultColor;
    
    caf::PdmField<caf::AppEnum< FaultFaceCullingMode > > faultFaceCulling;


    caf::PdmField<bool>                 showFaultLabel;
    caf::PdmField<cvf::Color3f>         faultLabelColor;

    caf::PdmField<bool>                 showFaultCollection;

    caf::PdmPointersField<RimFault*>    faults;

    virtual void                        fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle*        objectToggleField();

private:
    RimFault*                           findFaultByName(QString name);

private:
    RimReservoirView* m_reservoirView;

};
