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

#include "RimEllipseFractureTemplate.h"

#include "RiaLogging.h"

#include "RigCellGeometryTools.h"
#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigTesselatorTools.h"

#include "RimDefines.h"
#include "RimFracture.h"
#include "RimFractureContainment.h"
#include "RimFractureTemplate.h"
#include "RimProject.h"

#include "cafPdmObject.h"

#include "cvfVector3.h"
#include "cvfGeometryTools.h"



CAF_PDM_SOURCE_INIT(RimEllipseFractureTemplate, "RimEllipseFractureTemplate");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEllipseFractureTemplate::RimEllipseFractureTemplate(void)
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&halfLength,  "HalfLength",       650.0f,  "Halflength X<sub>f</sub>", "", "", "");
    CAF_PDM_InitField(&height,      "Height",           75.0f,   "Height", "", "", "");
    CAF_PDM_InitField(&width,       "Width",            1.0f,    "Width", "", "", "");

    CAF_PDM_InitField(&permeability,"Permeability",     22000.f, "Permeability [mD]", "", "", "");

    m_fractureGrid = new RigFractureGrid();
    setupFractureGridCells();
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEllipseFractureTemplate::~RimEllipseFractureTemplate()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimFractureTemplate::fieldChangedByUi(changedField, oldValue, newValue);

    if (changedField == &halfLength || changedField == &height)
    {
        //Changes to one of these parameters should change all fractures with this fracture template attached. 
        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj)
        {
            //Regenerate geometry
            std::vector<RimFracture*> fractures;
            proj->descendantsIncludingThisOfType(fractures);

            for (RimFracture* fracture : fractures)
            {
                if (fracture->attachedFractureDefinition() == this)
                {
                    fracture->setRecomputeGeometryFlag();
                }
            }

            proj->createDisplayModelAndRedrawAllViews();
            setupFractureGridCells();
        }
    }
    if (changedField == &width || changedField == &permeability)
    {
        setupFractureGridCells();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::fractureGeometry(std::vector<cvf::Vec3f>* nodeCoords, std::vector<cvf::uint>* triangleIndices, RimDefines::UnitSystem fractureUnit)
{
    RigEllipsisTesselator tesselator(20);

    float a = cvf::UNDEFINED_FLOAT;
    float b = cvf::UNDEFINED_FLOAT;

    if (fractureUnit == fractureTemplateUnit())
    {
        a = halfLength;
        b = height / 2.0f;

    }
    else if (fractureTemplateUnit() == RimDefines::UNITS_METRIC && fractureUnit == RimDefines::UNITS_FIELD)
    {
        RiaLogging::info(QString("Converting fracture template geometry from metric to field"));
        a = RimDefines::meterToFeet(halfLength);
        b = RimDefines::meterToFeet(height / 2.0f);
    }
    else if (fractureTemplateUnit() == RimDefines::UNITS_FIELD && fractureUnit == RimDefines::UNITS_METRIC)
    {
        RiaLogging::info(QString("Converting fracture template geometry from field to metric"));
        a = RimDefines::feetToMeter(halfLength);
        b = RimDefines::feetToMeter(height / 2.0f);
    }
    else
    {
        //Should never get here...
        RiaLogging::error(QString("Error: Could not convert units for fracture / fracture template"));
        return;
    }


    tesselator.tesselateEllipsis(a, b, triangleIndices, nodeCoords);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimEllipseFractureTemplate::fracturePolygon(RimDefines::UnitSystem fractureUnit)
{
    std::vector<cvf::Vec3f> polygon;

    std::vector<cvf::Vec3f> nodeCoords;
    std::vector<cvf::uint>  triangleIndices;

    fractureGeometry(&nodeCoords, &triangleIndices, fractureUnit);

    for (size_t i = 1; i < nodeCoords.size(); i++)
    {
        polygon.push_back(nodeCoords[i]);
    }

    return polygon;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::changeUnits()
{
    if (fractureTemplateUnit == RimDefines::UNITS_METRIC)
    {
        halfLength = RimDefines::meterToFeet(halfLength);
        height = RimDefines::meterToFeet(height);
        width = RimDefines::meterToInch(width);
        wellDiameter = RimDefines::meterToInch(wellDiameter);
        perforationLength = RimDefines::meterToFeet(perforationLength);
        fractureTemplateUnit = RimDefines::UNITS_FIELD;
    }
    else if (fractureTemplateUnit == RimDefines::UNITS_FIELD)
    {
        halfLength = RimDefines::feetToMeter(halfLength);
        height = RimDefines::feetToMeter(height);
        width = RimDefines::inchToMeter(width);
        wellDiameter = RimDefines::inchToMeter(wellDiameter);
        perforationLength = RimDefines::feetToMeter(perforationLength);
        fractureTemplateUnit = RimDefines::UNITS_METRIC;
    }

    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::setupFractureGridCells()
{

    std::vector<RigFractureCell> fractureCells;
    std::pair<size_t, size_t> wellCenterFractureCellIJ = std::make_pair(0, 0);

    bool wellCenterFractureCellFound = false;

    int numberOfCellsX = 25;
    int numberOfCellsY = 25;
    
    double cellSizeX = (halfLength * 2) / numberOfCellsX;
    double cellSizeZ = height / numberOfCellsY;

    double cellArea = cellSizeX * cellSizeZ;
    double areaTresholdForIncludingCell = 0.5 * cellArea;


    for (int i = 0; i < numberOfCellsX; i++)
    {
        for (int j = 0; j < numberOfCellsX; j++)
        {
            double X1 = - halfLength +  i    * cellSizeX;
            double X2 = - halfLength + (i+1) * cellSizeX;
            double Y1 = - height / 2 +  j    * cellSizeZ;
            double Y2 = - height / 2 + (j+1) * cellSizeZ;

            std::vector<cvf::Vec3d> cellPolygon;
            cellPolygon.push_back(cvf::Vec3d(X1, Y1, 0.0));
            cellPolygon.push_back(cvf::Vec3d(X2, Y1, 0.0));
            cellPolygon.push_back(cvf::Vec3d(X2, Y2, 0.0));
            cellPolygon.push_back(cvf::Vec3d(X1, Y2, 0.0));
            
            double cond = cvf::UNDEFINED_DOUBLE;
            if (fractureTemplateUnit == RimDefines::UNITS_METRIC)
            {
                //Conductivity should be md-m, width is in m
                cond = permeability * width;
            }
            else if(fractureTemplateUnit == RimDefines::UNITS_FIELD)
            {
                //Conductivity should be md-ft, but width is in inches 
                cond = permeability * RimDefines::inchToFeet(width);
            }

            std::vector<cvf::Vec3f> ellipseFracPolygon = fracturePolygon(fractureTemplateUnit());
            std::vector<cvf::Vec3d> ellipseFracPolygonDouble;
            for (auto v : ellipseFracPolygon) ellipseFracPolygonDouble.push_back(static_cast<cvf::Vec3d>(v));
            std::vector<std::vector<cvf::Vec3d> >clippedFracturePolygons = RigCellGeometryTools::intersectPolygons(cellPolygon, ellipseFracPolygonDouble);
            if (clippedFracturePolygons.size() > 0)
            {
                for (auto clippedFracturePolygon : clippedFracturePolygons)
                {
                    double areaCutPolygon = cvf::GeometryTools::polygonAreaNormal3D(clippedFracturePolygon).length();
                    if (areaCutPolygon < areaTresholdForIncludingCell) cond = 0.0; //Cell is excluded from calculation, cond is set to zero. Must be included for indexing to be correct
                }
            }
            else cond = 0.0;

            RigFractureCell fractureCell(cellPolygon, i, j);
            fractureCell.setConductivityValue(cond);

            if (cellPolygon[0].x() < 0.0 && cellPolygon[1].x() > 0.0)
            {
                if (cellPolygon[1].y() < 0.0 && cellPolygon[2].y() > 0.0)
                {
                    wellCenterFractureCellIJ = std::make_pair(fractureCell.getI(), fractureCell.getJ());
                    RiaLogging::debug(QString("Setting wellCenterFractureCell at cell %1, %2").
                                      arg(QString::number(fractureCell.getI()), QString::number(fractureCell.getJ())));

                    wellCenterFractureCellFound = true;
                }
            }

            fractureCells.push_back(fractureCell);

        }
    }

    
    m_fractureGrid->setFractureCells(fractureCells);
    m_fractureGrid->setWellCenterFractureCellIJ(wellCenterFractureCellIJ);
    m_fractureGrid->setICellCount(numberOfCellsX);
    m_fractureGrid->setJCellCount(numberOfCellsY);


}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigFractureGrid* RimEllipseFractureTemplate::fractureGrid() const
{
    return m_fractureGrid.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimFractureTemplate::defineUiOrdering(uiConfigName, uiOrdering);
    
    if (fractureTemplateUnit == RimDefines::UNITS_METRIC)
    {
        halfLength.uiCapability()->setUiName("Halflenght X<sub>f</sub> [m]");
        height.uiCapability()->setUiName("Height [m]");
        width.uiCapability()->setUiName("Width [m]");
        wellDiameter.uiCapability()->setUiName("Well Diameter [m]");
    }
    else if (fractureTemplateUnit == RimDefines::UNITS_FIELD)
    {
        halfLength.uiCapability()->setUiName("Halflenght X<sub>f</sub> [Ft]");
        height.uiCapability()->setUiName("Height [Ft]");
        width.uiCapability()->setUiName("Width [inches]");
        wellDiameter.uiCapability()->setUiName("Well Diameter [inches]");
    }

    
    uiOrdering.add(&name);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Geometry");
    geometryGroup->add(&halfLength);
    geometryGroup->add(&height);
    geometryGroup->add(&orientation);
    geometryGroup->add(&azimuthAngle);

    caf::PdmUiGroup* trGr = uiOrdering.addNewGroup("Fracture Truncation");
    m_fractureContainmentField()->defineUiOrdering(uiConfigName, *trGr);

    caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup("Properties");
    propertyGroup->add(&fractureConductivity);
    if (fractureConductivity == RimFractureTemplate::FINITE_CONDUCTIVITY)
    {
        propertyGroup->add(&permeability);
        propertyGroup->add(&width);
    }
    propertyGroup->add(&skinFactor);
    propertyGroup->add(&perforationLength);
    propertyGroup->add(&perforationEfficiency);
    propertyGroup->add(&wellDiameter);

    uiOrdering.add(&fractureTemplateUnit);
    uiOrdering.skipRemainingFields(true);
}

