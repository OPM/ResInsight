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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafPdmChildField.h"
#include "cafAppEnum.h"

// Include to make Pdm work for cvf::Color
#include "cafPdmFieldCvfColor.h"    
#include "cafPdmChildArrayField.h"
#include "cafPdmFieldCvfVec3d.h"
#include "cafPdmProxyValueField.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellPathCompletion : public RimCheckableNamedObject
{
    CAF_PDM_HEADER_INIT;
public:

    RimWellPathCompletion();
    virtual ~RimWellPathCompletion();

    virtual void                        defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    virtual void                        fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void                                setCoordinates(std::vector< cvf::Vec3d > coordinates);
    void                                setMeasuredDepths(std::vector< double > measuredDepths);

    std::vector< cvf::Vec3d >           coordinates() { return m_coordinates(); }
    std::vector< double >               measuredDepths() { return m_measuredDepths(); }
private:
    std::vector<QString>                displayCoordinates() const;

    caf::PdmField< std::vector< cvf::Vec3d> >  m_coordinates;
    caf::PdmField< std::vector< double > >     m_measuredDepths;
    caf::PdmProxyValueField< std::vector<QString> > m_displayCoordinates;
};
