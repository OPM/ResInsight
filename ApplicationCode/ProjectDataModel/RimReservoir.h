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

#include "RimWellCollection.h"

class QString;
class RigReservoir;
class RifReaderInterface;
class RimWellCollection;
class RigGridBase;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimReservoir : public caf::PdmObject, public cvf::Object
{
    CAF_PDM_HEADER_INIT;

public:
    RimReservoir();
    RimReservoir(RigReservoir* reservoir);
    virtual ~RimReservoir();

    bool                        openEclipseGridFile();
                                      
    RigReservoir*               reservoirData();
    const RigReservoir*         reservoirData() const;
                                      
    RifReaderInterface*         fileInterface();
    const RifReaderInterface*   fileInterface() const;

    RimReservoirView*           createAndAddReservoirView();
    void                        removeReservoirView(RimReservoirView* reservoirView);
                                      
    // Fields:                        
    caf::PdmField<QString>      caseName;
    caf::PdmField<QString>      caseDirectory;

    caf::PdmPointersField<RimReservoirView*> reservoirViews;

    virtual caf::PdmFieldHandle*    userDescriptionField()  { return &caseName;}

protected:
    // Overridden methods
    virtual void                    initAfterRead();

private:
    void                            createMockModel(QString modelName);

private:
    cvf::ref<RigReservoir>            m_rigReservoir;
    cvf::ref<RifReaderInterface>      m_fileInterface;
    
};

