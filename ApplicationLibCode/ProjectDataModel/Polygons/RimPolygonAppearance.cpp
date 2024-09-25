/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimPolygonAppearance.h"

#include "RimCase.h"
#include "RimProject.h"

#include "RigPolyLinesData.h"

#include "RiaNumericalTools.h"
#include "RiaStdStringTools.h"

#include "cafPdmUiDoubleSliderEditor.h"
#include "cafPdmUiLineEditor.h"

#include "cvfBoundingBox.h"

CAF_PDM_SOURCE_INIT( RimPolygonAppearance, "RimPolygonAppearance" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class ThicknessValidator : public QValidator
{
public:
    State validate( QString& input, int& pos ) const override
    {
        if ( input.isEmpty() ) return State::Intermediate;

        int val = RiaStdStringTools::toInt( input.toStdString() );
        if ( val > 0 && val < 8 )
            return State::Acceptable;
        else
            return State::Invalid;
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RadiusValidator : public QValidator
{
public:
    State validate( QString& input, int& pos ) const override
    {
        if ( input.isEmpty() ) return State::Intermediate;

        double val = 0.0;
        RiaStdStringTools::toDouble( input.toStdString(), val );
        if ( val > 0.001 && val <= 2.0 )
            return State::Acceptable;
        else
            return State::Invalid;
    }
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonAppearance::RimPolygonAppearance()
    : objectChanged( this )

{
    CAF_PDM_InitObject( "Polygon", ":/PolylinesFromFile16x16.png" );

    CAF_PDM_InitField( &m_isClosed, "IsClosed", true, "Closed Polygon" );
    CAF_PDM_InitField( &m_showLines, "ShowLines", true, "Show Lines" );
    CAF_PDM_InitField( &m_showSpheres, "ShowSpheres", false, "Show Spheres" );

    CAF_PDM_InitField( &m_lineThickness, "LineThickness", 3, "Line Thickness" );
    CAF_PDM_InitField( &m_sphereRadiusFactor, "SphereRadiusFactor", 0.15, "Sphere Radius Factor" );

    CAF_PDM_InitField( &m_lineColor, "LineColor", cvf::Color3f( cvf::Color3f::ORANGE ), "Line Color" );
    CAF_PDM_InitField( &m_sphereColor, "SphereColor", cvf::Color3f( cvf::Color3f::ORANGE ), "Sphere Color" );

    CAF_PDM_InitField( &m_polygonPlaneDepth, "PolygonPlaneDepth", 0.0, "Polygon Plane Depth" );
    CAF_PDM_InitField( &m_lockPolygonToPlane, "LockPolygon", false, "Lock Polygon to Plane" );

    m_polygonPlaneDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleSliderEditor::uiEditorTypeName() );
    m_polygonPlaneDepth.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosType::TOP );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonAppearance::applyAppearanceSettings( RigPolyLinesData* polyLinesData )
{
    polyLinesData->setLineAppearance( m_lineThickness, m_lineColor, m_isClosed );
    polyLinesData->setSphereAppearance( m_sphereRadiusFactor, m_sphereColor );
    polyLinesData->setZPlaneLock( m_lockPolygonToPlane, -m_polygonPlaneDepth );
    polyLinesData->setVisibility( m_showLines, m_showSpheres );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonAppearance::setIsClosed( bool isClosed )
{
    m_isClosed = isClosed;
    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimPolygonAppearance::isClosed() const
{
    return m_isClosed();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimPolygonAppearance::lineColor() const
{
    return m_lineColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonAppearance::setLineColor( const cvf::Color3f& color )
{
    m_lineColor = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonAppearance::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_showLines );
    if ( m_showLines )
    {
        uiOrdering.add( &m_lineThickness );
        uiOrdering.add( &m_lineColor );
    }

    uiOrdering.add( &m_showSpheres );
    if ( m_showSpheres )
    {
        uiOrdering.add( &m_sphereRadiusFactor );
        uiOrdering.add( &m_sphereColor );
    }

    uiOrdering.add( &m_lockPolygonToPlane );
    if ( m_lockPolygonToPlane )
    {
        uiOrdering.add( &m_polygonPlaneDepth );
    }

    uiOrdering.add( &m_isClosed );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonAppearance::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonAppearance::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_lineThickness )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            myAttr->validator = new ThicknessValidator();
        }
    }
    else if ( field == &m_lineThickness )
    {
        if ( auto myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            myAttr->validator = new RadiusValidator();
        }
    }
    else if ( field == &m_polygonPlaneDepth )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>( attribute ) )
        {
            auto allCases = RimProject::current()->allGridCases();
            if ( allCases.empty() )
            {
                attr->m_minimum = 0;
                attr->m_maximum = 10000.0;
            }
            else
            {
                double min = std::numeric_limits<double>::max();
                double max = -std::numeric_limits<double>::max();

                for ( auto gridCase : allCases )
                {
                    auto bb = gridCase->allCellsBoundingBox();

                    min = std::min( min, bb.min().z() );
                    max = std::max( max, bb.max().z() );
                }

                auto adjustedMin = RiaNumericalTools::roundToNumSignificantDigitsFloor( -min, 2 );
                auto adjustedMax = RiaNumericalTools::roundToNumSignificantDigitsCeil( -max, 2 );

                attr->m_minimum = adjustedMax;
                attr->m_maximum = adjustedMin;
            }
        }
    }
}
