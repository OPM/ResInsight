/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#include "RiaEclipseUnitTools.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmChildField.h"
#include "cafPdmFieldCvfVec3d.h"

#include "cvfBase.h"
#include "cvfVector3.h"

#include <vector>

class RigEclipseCaseData;
class RigFractureGrid;
class RimFractureContainment;

//==================================================================================================
///  
///  
//==================================================================================================
class RimFractureTemplate : public caf::PdmObject 
{
     CAF_PDM_HEADER_INIT;

public:
    RimFractureTemplate(void);
    virtual ~RimFractureTemplate(void);
    
    caf::PdmField<QString>   name;
    caf::PdmField<float>     azimuthAngle;
    caf::PdmField<float>     skinFactor;

    caf::PdmField<double>           perforationLength;
    caf::PdmField<double>           perforationEfficiency;
    caf::PdmField<double>           wellDiameter;

    enum FracOrientationEnum
    {
        AZIMUTH,
        ALONG_WELL_PATH,
        TRANSVERSE_WELL_PATH
    };
    caf::PdmField< caf::AppEnum< FracOrientationEnum > > orientationType;

    enum FracConductivityEnum
    {
        INFINITE_CONDUCTIVITY,
        FINITE_CONDUCTIVITY,
    };
    caf::PdmField< caf::AppEnum< FracConductivityEnum > >  conductivityType;

    caf::PdmField< RiaEclipseUnitTools::UnitSystemType >  fractureTemplateUnit;

    void                            setDefaultWellDiameterFromUnit();
    double                          wellDiameterInFractureUnit(RiaEclipseUnitTools::UnitSystemType fractureUnit);
    double                          perforationLengthInFractureUnit(RiaEclipseUnitTools::UnitSystemType fractureUnit);
    
    virtual void                    fractureTriangleGeometry(std::vector<cvf::Vec3f>* nodeCoords, 
                                                             std::vector<cvf::uint>*  triangleIndices, 
                                                             RiaEclipseUnitTools::UnitSystem neededUnit) = 0;
    virtual std::vector<cvf::Vec3f> fractureBorderPolygon(RiaEclipseUnitTools::UnitSystem neededUnit) = 0;

    virtual const RigFractureGrid*  fractureGrid() const = 0;

    const RimFractureContainment *  fractureContainment();

protected:
    caf::PdmChildField<RimFractureContainment*> m_fractureContainment;

    virtual caf::PdmFieldHandle*    userDescriptionField() override;
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

    virtual void                    defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

};
