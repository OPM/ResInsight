/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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
#include "cafPdmPtrField.h"
#include "cafAppEnum.h"

class RimCase;
class RimView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimMultiSnapshotDefinition : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimMultiSnapshotDefinition();
    virtual ~RimMultiSnapshotDefinition();

    caf::PdmPtrField<RimCase*>  caseObject;
    caf::PdmPtrField<RimView*>  viewObject;

    caf::PdmField<int>       timeStepStart;
    caf::PdmField<int>       timeStepEnd;

    enum SnapShotDirectionEnum
    {
        RANGEFILTER_I,
        RANGEFILTER_J,
        RANGEFILTER_K
    };

    caf::PdmField< caf::AppEnum< SnapShotDirectionEnum > > sliceDirection;
    caf::PdmField<int>       startSliceIndex;
    caf::PdmField<int>       endSliceIndex;


    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

    void getTimeStepStrings(QList<caf::PdmOptionItemInfo> &options);

};
