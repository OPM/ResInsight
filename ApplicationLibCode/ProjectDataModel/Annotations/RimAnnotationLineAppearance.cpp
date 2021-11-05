/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimAnnotationLineAppearance.h"
#include "RimAnnotationCollection.h"

#include "RiaStdStringTools.h"

#include <QValidator>

#include "cafPdmUiLineEditor.h"

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

namespace caf
{
template <>
void RimAnnotationLineAppearance::LineStyle::setUp()
{
    addItem( RimAnnotationLineAppearance::STYLE_SOLID, "STYLE_SOLID", "Solid" );
    addItem( RimAnnotationLineAppearance::STYLE_DASH, "STYLE_DASH", "Dashes" );

    setDefault( RimAnnotationLineAppearance::STYLE_SOLID );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimAnnotationLineAppearance, "RimAnnotationLineAppearance" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimAnnotationLineAppearance::RimAnnotationLineAppearance()
    : objectChanged( this )
{
    CAF_PDM_InitObject( "AnnotationLineAppearance", ":/WellCollection.png", "", "" );

    CAF_PDM_InitField( &m_lineFieldsHidden, "LineFieldsHidden", false, "Line Fields Hidden" );
    CAF_PDM_InitField( &m_color, "Color", cvf::Color3f( cvf::Color3f::BLACK ), "Line Color" );
    CAF_PDM_InitField( &m_thickness, "Thickness", 2, "Line Thickness" );

    // Stippling not yet supported. Needs new stuff in VizFwk
    CAF_PDM_InitField( &m_style, "Style", LineStyle(), "Style" );
    m_style.uiCapability()->setUiHidden( true );
    m_style.xmlCapability()->disableIO();

    m_lineFieldsHidden.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationLineAppearance::setLineFieldsHidden( bool hidden )
{
    m_lineFieldsHidden = hidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationLineAppearance::setColor( const cvf::Color3f& newColor )
{
    m_color = newColor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimAnnotationLineAppearance::color() const
{
    return m_color();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimAnnotationLineAppearance::isDashed() const
{
    return m_style() == STYLE_DASH;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimAnnotationLineAppearance::thickness() const
{
    return m_thickness();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationLineAppearance::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    if ( !m_lineFieldsHidden() )
    {
        uiOrdering.add( &m_color );
        uiOrdering.add( &m_style );
        uiOrdering.add( &m_thickness );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationLineAppearance::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue )
{
    RimAnnotationCollection* annColl = nullptr;
    this->firstAncestorOrThisOfType( annColl );
    if ( annColl ) annColl->scheduleRedrawOfRelevantViews();

    objectChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimAnnotationLineAppearance::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_thickness )
    {
        auto myAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->validator = new ThicknessValidator();
        }
    }
}

CAF_PDM_SOURCE_INIT( RimPolylineAppearance, "RimPolylineAppearance" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolylineAppearance::RimPolylineAppearance()
{
    CAF_PDM_InitObject( "PolylineAppearance", ":/WellCollection.png", "", "" );

    CAF_PDM_InitField( &m_sphereFieldsHidden, "SphereFieldsHidden", false, "Sphere Fields Hidden" );
    CAF_PDM_InitField( &m_sphereColor, "SphereColor", cvf::Color3f( cvf::Color3f::BLACK ), "Sphere Color" );
    CAF_PDM_InitField( &m_sphereRadiusFactor, "SphereRadiusFactor", 0.1, "Sphere Radius Factor" );

    m_sphereFieldsHidden.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineAppearance::setSphereFieldsHidden( bool hidden )
{
    m_sphereFieldsHidden = hidden;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineAppearance::setSphereColor( const cvf::Color3f& color )
{
    m_sphereColor = color;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimPolylineAppearance::sphereColor() const
{
    return m_sphereColor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimPolylineAppearance::sphereRadiusFactor() const
{
    return m_sphereRadiusFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineAppearance::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimAnnotationLineAppearance::defineUiOrdering( uiConfigName, uiOrdering );

    if ( !m_sphereFieldsHidden )
    {
        uiOrdering.add( &m_sphereColor );
        uiOrdering.add( &m_sphereRadiusFactor );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolylineAppearance::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                              const QVariant&            oldValue,
                                              const QVariant&            newValue )
{
    RimAnnotationLineAppearance::fieldChangedByUi( changedField, oldValue, newValue );
}
