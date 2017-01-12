/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimFracture.h"

#include "RigFracture.h"
#include "RigTesselatorTools.h"

#include "RimEllipseFractureTemplate.h"
#include "RimView.h"

#include "cafPdmUiDoubleSliderEditor.h"

#include "cvfMath.h"
#include "cvfMatrix4.h"
#include "RiaApplication.h"
#include "RigMainGrid.h"
#include "RimEclipseView.h"
#include "cvfBoundingBox.h"


CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimFracture, "Fracture");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFracture::RimFracture(void)
{
    CAF_PDM_InitObject("Fracture", "", "", "");

    CAF_PDM_InitField(&azimuth, "Azimuth", 0.0, "Azimuth", "", "", "");
    azimuth.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    m_rigFracture = new RigFracture;
    m_recomputeGeometry = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFracture::~RimFracture()
{
}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::uint>& RimFracture::triangleIndices() const
{
    return m_rigFracture->triangleIndices();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<cvf::Vec3f>& RimFracture::nodeCoords() const
{
    return m_rigFracture->nodeCoords();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimFracture::getPotentiallyFracturedCells()
{
    std::vector<size_t> cellindecies;

    RiaApplication* app = RiaApplication::instance();
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return cellindecies;
 
    RimEclipseView* activeRiv = dynamic_cast<RimEclipseView*>(activeView);
    if (!activeRiv) return cellindecies;

    const RigMainGrid* mainGrid = activeRiv->mainGrid();
    if (!mainGrid) return cellindecies;

    const std::vector<cvf::Vec3f>& nodeCoordVec = nodeCoords();

    if (!hasValidGeometry()) computeGeometry();

    cvf::BoundingBox polygonBBox;
    for (cvf::Vec3f nodeCoord : nodeCoordVec) polygonBBox.add(nodeCoord);

    mainGrid->findIntersectingCells(polygonBBox, &cellindecies);

    return cellindecies;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &azimuth)
    {
        computeGeometry();

        RimView* rimView = nullptr;
        this->firstAncestorOrThisOfType(rimView);
        if (rimView)
        {
            rimView->createDisplayModelAndRedraw();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::computeGeometry()
{
    std::vector<cvf::Vec3f> nodeCoords;
    std::vector<cvf::uint>  polygonIndices;

    RimEllipseFractureTemplate* fractureDef = attachedFractureDefinition();
    if (fractureDef )
    {

        //TODO: Move to fracture template
        RigEllipsisTesselator tesselator(20);

        float a = fractureDef->height / 2.0f;
        float b = fractureDef->halfLength;

        tesselator.tesselateEllipsis(a, b, &polygonIndices, &nodeCoords);

    }

    cvf::Mat4f m = transformMatrix();

    for (cvf::Vec3f& v : nodeCoords)
    {
        v.transformPoint(m);
    }

    m_rigFracture->setGeometry(polygonIndices, nodeCoords); 

    m_recomputeGeometry = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Mat4f RimFracture::transformMatrix()
{
    cvf::Vec3d center = centerPointForFracture();

    // Ellipsis geometry is produced in XY-plane, rotate 90 deg around X to get zero azimuth along Y
    cvf::Mat4f rotationFromTesselator = cvf::Mat4f::fromRotation(cvf::Vec3f::X_AXIS, cvf::Math::toRadians(90.0f));

    // Azimuth rotation
    cvf::Mat4f azimuthRotation = cvf::Mat4f::fromRotation(cvf::Vec3f::Z_AXIS, cvf::Math::toRadians(-azimuth()));

    cvf::Mat4f m = azimuthRotation * rotationFromTesselator;
    m.setTranslation(cvf::Vec3f(center));

    return m;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::computeTransmissibility()
{ 
    std::vector<RigFractureData> fracDataVec;

    std::vector<size_t> fracCells = getPotentiallyFracturedCells();

    for (auto fracCell : fracCells)
    {
        RigFractureData fracData; 
        fracData.reservoirCellIndex = fracCell;

        //TODO: get correct input values...
        double area = 2.468;
        double fractureLength = 1.2345;
        double flowLength = 2.718281828;

        double c = 0.008527; // TODO: Get value with units, is defined in RimReservoirCellResultsStorage
        
        double transmissibility;
        if (attachedFractureDefinition())
        {
            transmissibility = 8 * c * attachedFractureDefinition()->permeability * area /
                            ( flowLength + (attachedFractureDefinition()->skinFactor * fractureLength) / cvf::PI_D);
        }
        else transmissibility = cvf::UNDEFINED_DOUBLE;

        fracData.transmissibility = transmissibility;
        fracDataVec.push_back(fracData);
    }

    m_rigFracture->setFractureData(fracDataVec);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::setRecomputeGeometryFlag()
{
    m_recomputeGeometry = true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimFracture::isRecomputeGeometryFlagSet()
{
    return m_recomputeGeometry;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Properties");
    geometryGroup->add(&azimuth);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFracture::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    if (field == &azimuth)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_minimum = 0;
            myAttr->m_maximum = 360;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RigFracture> RimFracture::attachedRigFracture()
{
    return m_rigFracture;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimFracture::hasValidGeometry() const
{
    if (m_recomputeGeometry) return false;

    return (nodeCoords().size() > 0 && triangleIndices().size() > 0);
}

