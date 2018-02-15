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

#include "RimFractureTemplate.h"

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"
#include "cafPdmFieldCvfVec3d.h"

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <vector>

class RigFractureGrid;

//==================================================================================================
///  
///  
//==================================================================================================
class RimEllipseFractureTemplate : public RimFractureTemplate 
{
     CAF_PDM_HEADER_INIT;

public:
    RimEllipseFractureTemplate(void);
    virtual ~RimEllipseFractureTemplate(void);
    
    void                            loadDataAndUpdate();
    
    void                            fractureTriangleGeometry(std::vector<cvf::Vec3f>* nodeCoords, 
                                                     std::vector<cvf::uint>* polygonIndices, 
                                                     RiaEclipseUnitTools::UnitSystem neededUnit);
    std::vector<cvf::Vec3f>         fractureBorderPolygon(RiaEclipseUnitTools::UnitSystem  neededUnit);
    void                            changeUnits();
    
    const RigFractureGrid*          fractureGrid() const;

    void                            setDefaultValuesFromUnit();
    double                          conductivity() const;

    virtual void appendDataToResultStatistics(const QString& uiResultName, const QString& unit,
                                               MinMaxAccumulator& minMaxAccumulator,
                                               PosNegAccumulator& posNegAccumulator) const override;


    virtual std::vector<std::pair<QString, QString>> uiResultNamesWithUnit() const override;

protected:
    virtual void                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);
    virtual void                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    void                            setupFractureGridCells();


    virtual FractureWidthAndConductivity widthAndConductivityAtWellPathIntersection() const override;

private:
    cvf::ref<RigFractureGrid>   m_fractureGrid;

    caf::PdmField<float>        m_halfLength;
    caf::PdmField<float>        m_height;
    caf::PdmField<float>        m_width;
    caf::PdmField<float>        m_userDefinedEffectivePermeability;
};
