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

#include "RimCheckableNamedObject.h"
#include "RimMswCompletionParameters.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmChildArrayField.h"

#include <vector>

class RimWellPathFracture;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellPathFractureCollection : public RimCheckableNamedObject
{
     CAF_PDM_HEADER_INIT;
public:
    enum ReferenceMDType
    {
        AUTO_REFERENCE_MD = 0,
        MANUAL_REFERENCE_MD
    };

    typedef caf::AppEnum<ReferenceMDType> ReferenceMDEnum;

    RimWellPathFractureCollection(void);
    virtual ~RimWellPathFractureCollection(void);
    
    const RimMswCompletionParameters* mswParameters() const;
    void                              addFracture(RimWellPathFracture* fracture);
    void                              deleteFractures();
    void                              setUnitSystemSpecificDefaults();
    ReferenceMDType                   referenceMDType() const;
    double                            manualReferenceMD() const;
    
    std::vector<RimWellPathFracture*> fractures() const;
    std::vector<RimWellPathFracture*> activeFractures() const;

protected:
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

private:
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    caf::PdmChildArrayField<RimWellPathFracture*>   m_fractures;
    caf::PdmField<ReferenceMDEnum>                  m_refMDType;
    caf::PdmField<double>                           m_refMD;
    caf::PdmChildField<RimMswCompletionParameters*> m_mswParameters;
};
