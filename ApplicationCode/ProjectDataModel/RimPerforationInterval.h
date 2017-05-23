/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
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
#include "Rim3dPropertiesInterface.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"

//==================================================================================================
///  
///  
//==================================================================================================
class RimPerforationInterval : public RimCheckableNamedObject, public Rim3dPropertiesInterface
{
    CAF_PDM_HEADER_INIT;
public:

    RimPerforationInterval();
    virtual ~RimPerforationInterval();

    void                                setStartAndEndMD(double startMD, double endMD);
    double                              startMD() { return m_startMD(); }
    double                              endMD() { return m_endMD(); }

    virtual cvf::BoundingBox            boundingBoxInDomainCoords() override;

protected:
    virtual void                        defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    virtual void                        fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    virtual void                        defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;

private:
    caf::PdmField< double >             m_startMD;
    caf::PdmField< double >             m_endMD;
    caf::PdmField< double >             m_radius;
    caf::PdmField< double >             m_skinFactor;
};
