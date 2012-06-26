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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"


class QString;
class RigReservoir;
class RigGridBase;
class RimReservoirView;

//==================================================================================================
// 
// Interface for reservoirs. 
// As this is a pure virtual class, the factory macros are not relevant (nor possible) to use
// CAF_PDM_HEADER_INIT and CAF_PDM_SOURCE_INIT
// 
//==================================================================================================
class RimReservoir : public caf::PdmObject
{

public:
    RimReservoir();
    virtual ~RimReservoir();

    virtual bool                openEclipseGridFile() = 0;
                                      
    RigReservoir*               reservoirData();
    const RigReservoir*         reservoirData() const;
                                      
    RimReservoirView*           createAndAddReservoirView();
    void                        removeReservoirView(RimReservoirView* reservoirView);

    void                        removeResult(const QString& resultName);
                                      
    // Fields:                        
    caf::PdmField<QString>      caseName;

    caf::PdmPointersField<RimReservoirView*> reservoirViews;

    virtual caf::PdmFieldHandle*    userDescriptionField()  { return &caseName; }
    
    virtual QString             locationOnDisc() const      { return QString(); }

protected:
    // Overridden methods
    virtual void                    initAfterRead();


protected:
    cvf::ref<RigReservoir>            m_rigReservoir;
};

