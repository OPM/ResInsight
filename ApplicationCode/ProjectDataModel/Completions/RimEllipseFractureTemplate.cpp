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

#include "RiaApplication.h"
#include "RiaEclipseUnitTools.h"
#include "RiaFractureDefines.h"
#include "RiaLogging.h"

#include "RigCellGeometryTools.h"
#include "RigFractureCell.h"
#include "RigFractureGrid.h"
#include "RigStatisticsMath.h"
#include "RigTesselatorTools.h"

#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureContainment.h"
#include "RimFractureTemplate.h"
#include "RimProject.h"
#include "RimStimPlanColors.h"

#include "cafPdmObject.h"

#include "cvfVector3.h"
#include "cvfGeometryTools.h"



CAF_PDM_SOURCE_INIT(RimEllipseFractureTemplate, "RimEllipseFractureTemplate");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEllipseFractureTemplate::RimEllipseFractureTemplate()
{
    CAF_PDM_InitObject("Fracture Template", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&m_halfLength,  "HalfLength",       0.0,   "Halflength X<sub>f</sub>", "", "", "");
    CAF_PDM_InitField(&m_height,      "Height",           0.0,   "Height", "", "", "");
    CAF_PDM_InitField(&m_width,       "Width",            0.0,   "Width", "", "", "");

    CAF_PDM_InitField(&m_userDefinedEffectivePermeability,"Permeability",     0.0,   "Permeability [mD]", "", "", "");

    m_fractureGrid = new RigFractureGrid();
    assignConductivityToCellsInsideEllipse();
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
void RimEllipseFractureTemplate::loadDataAndUpdate()
{
    assignConductivityToCellsInsideEllipse();

    RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
    if (activeView) activeView->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimFractureTemplate::fieldChangedByUi(changedField, oldValue, newValue);

    if (   changedField == &m_halfLength
        || changedField == &m_height
        || changedField == &m_width
        || changedField == &m_userDefinedEffectivePermeability
        || changedField == &m_scaleApplyButton)
    {
        m_scaleApplyButton = false;

        //Changes to one of these parameters should change all fractures with this fracture template attached. 
        reload();
    }

    if (changedField == &m_width || changedField == &m_userDefinedEffectivePermeability)
    {
        assignConductivityToCellsInsideEllipse();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::fractureTriangleGeometry(std::vector<cvf::Vec3f>* nodeCoords, 
                                                          std::vector<cvf::uint>* triangleIndices)
{
    RigEllipsisTesselator tesselator(20);

    float a = m_halfLength * m_widthScaleFactor;
    float b = m_height / 2.0f * m_heightScaleFactor;

    tesselator.tesselateEllipsis(a, b, triangleIndices, nodeCoords);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RimEllipseFractureTemplate::fractureBorderPolygon()
{
    std::vector<cvf::Vec3f> polygon;

    std::vector<cvf::Vec3f> nodeCoords;
    std::vector<cvf::uint>  triangleIndices;

    fractureTriangleGeometry(&nodeCoords, &triangleIndices);

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
    if (fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_METRIC)
    {
        convertToUnitSystem(RiaEclipseUnitTools::UNITS_FIELD);
    }
    else if (fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_FIELD)
    {
        convertToUnitSystem(RiaEclipseUnitTools::UNITS_METRIC);
    }

    this->updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::assignConductivityToCellsInsideEllipse()
{
    std::vector<RigFractureCell> fractureCells;

    int numberOfCellsI = 35;
    int numberOfCellsJ = 35;
    
    double height = m_height * m_heightScaleFactor;
    double halfLength = m_halfLength * m_widthScaleFactor;

    double cellSizeX = (halfLength * 2) / numberOfCellsI * m_widthScaleFactor;
    double cellSizeZ = height / numberOfCellsJ * m_heightScaleFactor;

    double cellArea = cellSizeX * cellSizeZ;
    double areaTresholdForIncludingCell = 0.5 * cellArea;


    for (int i = 0; i < numberOfCellsI; i++)
    {
        for (int j = 0; j < numberOfCellsJ; j++)
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
            
            double cond = conductivity();

            std::vector<cvf::Vec3f> ellipseFracPolygon = fractureBorderPolygon();
            std::vector<cvf::Vec3d> ellipseFracPolygonDouble;
            for (const auto& v : ellipseFracPolygon) ellipseFracPolygonDouble.push_back(static_cast<cvf::Vec3d>(v));
            std::vector<std::vector<cvf::Vec3d> >clippedFracturePolygons = RigCellGeometryTools::intersectPolygons(cellPolygon, ellipseFracPolygonDouble);
            if (!clippedFracturePolygons.empty())
            {
                for (const auto& clippedFracturePolygon : clippedFracturePolygons)
                {
                    double areaCutPolygon = cvf::GeometryTools::polygonAreaNormal3D(clippedFracturePolygon).length();
                    if (areaCutPolygon < areaTresholdForIncludingCell) cond = 0.0; //Cell is excluded from calculation, cond is set to zero. Must be included for indexing to be correct
                }
            }
            else cond = 0.0;

            RigFractureCell fractureCell(cellPolygon, i, j);
            fractureCell.setConductivityValue(cond);

            fractureCells.push_back(fractureCell);
        }
    }
    
    m_fractureGrid->setFractureCells(fractureCells);

    // Set well intersection to center of ellipse
    std::pair<size_t, size_t> wellCenterFractureCellIJ = std::make_pair(numberOfCellsI / 2, numberOfCellsJ / 2);
    m_fractureGrid->setWellCenterFractureCellIJ(wellCenterFractureCellIJ);
    
    m_fractureGrid->setICellCount(numberOfCellsI);
    m_fractureGrid->setJCellCount(numberOfCellsJ);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
FractureWidthAndConductivity RimEllipseFractureTemplate::widthAndConductivityAtWellPathIntersection() const
{
    FractureWidthAndConductivity values;
    values.m_width = m_width;
    values.m_permeability = m_userDefinedEffectivePermeability;

    return values;
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
void RimEllipseFractureTemplate::setDefaultValuesFromUnit()
{
    if (fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_FIELD)
    {
        m_width        = 0.5;
        m_userDefinedEffectivePermeability = 80000.0;
        m_halfLength   = 300.0;
        m_height       = 225.0;
    }
    else
    {
        m_width        = 0.01;
        m_userDefinedEffectivePermeability = 100000.0;
        m_halfLength   = 100.0;
        m_height       = 75.0;
    }

    this->setDefaultWellDiameterFromUnit();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimEllipseFractureTemplate::conductivity() const
{
    double cond = cvf::UNDEFINED_DOUBLE;
    if (fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_METRIC)
    {
        //Conductivity should be md-m, width is in m
        cond = m_userDefinedEffectivePermeability * m_width;
    }
    else if (fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_FIELD)
    {
        //Conductivity should be md-ft, but width is in inches 
        cond = m_userDefinedEffectivePermeability * RiaEclipseUnitTools::inchToFeet(m_width);
    }

    return m_conductivityScaleFactor * cond;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::appendDataToResultStatistics(const QString& uiResultName, const QString& unit, MinMaxAccumulator& minMaxAccumulator, PosNegAccumulator& posNegAccumulator) const
{
    if (uiResultName == RiaDefines::conductivityResultName())
    {
        minMaxAccumulator.addValue(conductivity());
        posNegAccumulator.addValue(conductivity());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> RimEllipseFractureTemplate::uiResultNamesWithUnit() const
{
    std::vector<std::pair<QString, QString>> propertyNamesAndUnits;

    QString condUnit = RiaDefines::unitStringConductivity(fractureTemplateUnit());
    propertyNamesAndUnits.push_back(std::make_pair(RiaDefines::conductivityResultName(), condUnit));

    return propertyNamesAndUnits;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::reload()
{
    RimProject* proj;
    this->firstAncestorOrThisOfType(proj);
    if (proj)
    {
        //Regenerate geometry
        proj->createDisplayModelAndRedrawAllViews();
        assignConductivityToCellsInsideEllipse();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::convertToUnitSystem(RiaEclipseUnitTools::UnitSystem neededUnit)
{
    if (m_fractureTemplateUnit() == neededUnit) return;

    setFractureTemplateUnit(neededUnit);
    RimFractureTemplate::convertToUnitSystem(neededUnit);

    if (neededUnit == RiaEclipseUnitTools::UNITS_FIELD)
    {
        m_halfLength         = RiaEclipseUnitTools::meterToFeet(m_halfLength);
        m_height             = RiaEclipseUnitTools::meterToFeet(m_height);
        m_width              = RiaEclipseUnitTools::meterToInch(m_width);
    }
    else if (neededUnit == RiaEclipseUnitTools::UNITS_METRIC)
    {
        m_halfLength         = RiaEclipseUnitTools::feetToMeter(m_halfLength);
        m_height             = RiaEclipseUnitTools::feetToMeter(m_height);
        m_width              = RiaEclipseUnitTools::inchToMeter(m_width);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEllipseFractureTemplate::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    
    if (fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_METRIC)
    {
        m_halfLength.uiCapability()->setUiName("Halflenght X<sub>f</sub> [m]");
        m_height.uiCapability()->setUiName("Height [m]");
        m_width.uiCapability()->setUiName("Width [m]");
    }
    else if (fractureTemplateUnit() == RiaEclipseUnitTools::UNITS_FIELD)
    {
        m_halfLength.uiCapability()->setUiName("Halflenght X<sub>f</sub> [ft]");
        m_height.uiCapability()->setUiName("Height [ft]");
        m_width.uiCapability()->setUiName("Width [inches]");
    }

    if (conductivityType() == FINITE_CONDUCTIVITY)
    {
        m_userDefinedEffectivePermeability.uiCapability()->setUiHidden(false);
        m_width.uiCapability()->setUiHidden(false);
    }
    else if (conductivityType() == INFINITE_CONDUCTIVITY)
    {
        m_userDefinedEffectivePermeability.uiCapability()->setUiHidden(true);
        m_width.uiCapability()->setUiHidden(true);
    }
    
    uiOrdering.add(&m_name);
    uiOrdering.add(&m_id);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Geometry");
    geometryGroup->add(&m_halfLength);
    geometryGroup->add(&m_height);
    geometryGroup->add(&m_orientationType);
    geometryGroup->add(&m_azimuthAngle);

    caf::PdmUiGroup* trGr = uiOrdering.addNewGroup("Fracture Truncation");
    m_fractureContainment()->defineUiOrdering(uiConfigName, *trGr);

    caf::PdmUiGroup* propertyGroup = uiOrdering.addNewGroup("Properties");
    propertyGroup->add(&m_conductivityType);
    propertyGroup->add(&m_userDefinedEffectivePermeability);
    propertyGroup->add(&m_width);
    propertyGroup->add(&m_skinFactor);
    propertyGroup->add(&m_perforationLength);
    propertyGroup->add(&m_perforationEfficiency);
    propertyGroup->add(&m_wellDiameter);

    RimFractureTemplate::defineUiOrdering(uiConfigName, uiOrdering);
}

