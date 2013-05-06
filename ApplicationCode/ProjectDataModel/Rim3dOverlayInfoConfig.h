/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmField.h"
#include "cafAppEnum.h"

#include "cvfAssert.h"

#include "cvfVector2.h"

class RimReservoirView;
//==================================================================================================
///  
///  
//==================================================================================================
class Rim3dOverlayInfoConfig : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    Rim3dOverlayInfoConfig();
    virtual ~Rim3dOverlayInfoConfig();

    void update3DInfo();

    void setReservoirView(RimReservoirView* ownerReservoirView) {m_reservoirView = ownerReservoirView; }

    void                                        setPosition(cvf::Vec2ui position);
    caf::PdmField<bool>                         active;
    caf::PdmField<bool>                         showInfoText;
    caf::PdmField<bool>                         showAnimProgress;
    caf::PdmField<bool>                         showHistogram;
    
protected:
    virtual void                                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual caf::PdmFieldHandle*                objectToggleField();
private:
    caf::PdmPointer<RimReservoirView>           m_reservoirView;

    cvf::Vec2ui                                 m_position;
};
