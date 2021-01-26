/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-         Equinor ASA
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
#include "RimWellPathCompletionSettings.h"

#include "RiaStdStringTools.h"
#include "RimMswCompletionParameters.h"

#include "cafPdmDoubleStringValidator.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeOrdering.h"

namespace caf
{
template <>
void RimWellPathCompletionSettings::WellTypeEnum::setUp()
{
    addItem( RimWellPathCompletionSettings::OIL, "OIL", "Oil" );
    addItem( RimWellPathCompletionSettings::GAS, "GAS", "Gas" );
    addItem( RimWellPathCompletionSettings::WATER, "WATER", "Water" );
    addItem( RimWellPathCompletionSettings::LIQUID, "LIQUID", "Liquid" );

    setDefault( RimWellPathCompletionSettings::OIL );
}

template <>
void RimWellPathCompletionSettings::GasInflowEnum::setUp()
{
    addItem( RimWellPathCompletionSettings::STANDARD_EQ, "STD", "Standard" );
    addItem( RimWellPathCompletionSettings::RUSSELL_GOODRICH, "R-G", "Russell-Goodrich" );
    addItem( RimWellPathCompletionSettings::DRY_GAS_PSEUDO_PRESSURE, "P-P", "Dry Gas Pseudo-Pressure" );
    addItem( RimWellPathCompletionSettings::GENERALIZED_PSEUDO_PRESSURE, "GPP", "Generalized Pseudo-Pressure" );

    setDefault( RimWellPathCompletionSettings::STANDARD_EQ );
}

template <>
void RimWellPathCompletionSettings::AutomaticWellShutInEnum::setUp()
{
    addItem( RimWellPathCompletionSettings::ISOLATE_FROM_FORMATION, "SHUT", "Isolate from Formation" );
    addItem( RimWellPathCompletionSettings::STOP_ABOVE_FORMATION, "STOP", "Stop above Formation" );

    setDefault( RimWellPathCompletionSettings::STOP_ABOVE_FORMATION );
}

template <>
void RimWellPathCompletionSettings::HydrostaticDensityEnum::setUp()
{
    addItem( RimWellPathCompletionSettings::SEGMENTED, "SEG", "Segmented" );
    addItem( RimWellPathCompletionSettings::AVERAGED, "AVG", "Averaged" );

    setDefault( RimWellPathCompletionSettings::SEGMENTED );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimWellPathCompletionSettings, "WellPathCompletionSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCompletionSettings::RimWellPathCompletionSettings()
{
    CAF_PDM_InitObject( "Completion Settings", ":/CompletionsSymbol16x16.png", "", "" );
    CAF_PDM_InitField( &m_wellNameForExport, "WellNameForExport", QString(), "Well Name", "", "", "" );
    m_wellNameForExport.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_wellGroupName, "WellGroupNameForExport", QString(), "Well Group Name", "", "", "" );
    CAF_PDM_InitField( &m_referenceDepth, "ReferenceDepthForExport", QString(), "Reference Depth for BHP", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_preferredFluidPhase, "WellTypeForExport", "Preferred Fluid Phase", "", "", "" );
    CAF_PDM_InitField( &m_drainageRadiusForPI, "DrainageRadiusForPI", QString( "0.0" ), "Drainage Radius for PI", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_gasInflowEquation, "GasInflowEq", "Gas Inflow Equation", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_automaticWellShutIn, "AutoWellShutIn", "Automatic well shut-in", "", "", "" );
    CAF_PDM_InitField( &m_allowWellCrossFlow, "AllowWellCrossFlow", true, "Allow Well Cross-Flow", "", "", "" );
    CAF_PDM_InitField( &m_wellBoreFluidPVTTable, "WellBoreFluidPVTTable", 0, "Wellbore Fluid PVT table", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_hydrostaticDensity, "HydrostaticDensity", "Hydrostatic Density", "", "", "" );
    CAF_PDM_InitField( &m_fluidInPlaceRegion, "FluidInPlaceRegion", 0, "Fluid In-Place Region", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_mswParameters, "MswParameters", "Multi Segment Well Parameters", "", "", "" );
    m_mswParameters = new RimMswCompletionParameters( false );
    m_mswParameters.uiCapability()->setUiTreeHidden( true );
    m_mswParameters.uiCapability()->setUiTreeChildrenHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCompletionSettings::RimWellPathCompletionSettings( const RimWellPathCompletionSettings& rhs )
    : RimWellPathCompletionSettings()
{
    *this = rhs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCompletionSettings& RimWellPathCompletionSettings::operator=( const RimWellPathCompletionSettings& rhs )
{
    m_wellNameForExport     = rhs.m_wellNameForExport;
    m_wellGroupName         = rhs.m_wellGroupName;
    m_referenceDepth        = rhs.m_referenceDepth;
    m_preferredFluidPhase   = rhs.m_preferredFluidPhase;
    m_drainageRadiusForPI   = rhs.m_drainageRadiusForPI;
    m_gasInflowEquation     = rhs.m_gasInflowEquation;
    m_automaticWellShutIn   = rhs.m_automaticWellShutIn;
    m_allowWellCrossFlow    = rhs.m_allowWellCrossFlow;
    m_wellBoreFluidPVTTable = rhs.m_wellBoreFluidPVTTable;
    m_hydrostaticDensity    = rhs.m_hydrostaticDensity;
    m_fluidInPlaceRegion    = rhs.m_fluidInPlaceRegion;
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletionSettings::setWellNameForExport( const QString& name )
{
    auto n              = name;
    m_wellNameForExport = n.remove( ' ' );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletionSettings::updateWellPathNameHasChanged( const QString& newWellPathName,
                                                                  const QString& previousWellPathName )
{
    if ( m_wellNameForExport == previousWellPathName )
    {
        m_wellNameForExport = newWellPathName;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::wellNameForExport() const
{
    return formatStringForExport( m_wellNameForExport() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::wellGroupNameForExport() const
{
    return formatStringForExport( m_wellGroupName, "1*" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::referenceDepthForExport() const
{
    std::string refDepth = m_referenceDepth.v().toStdString();
    if ( RiaStdStringTools::isNumber( refDepth, '.' ) )
    {
        return m_referenceDepth.v();
    }
    return "1*";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::wellTypeNameForExport() const
{
    switch ( m_preferredFluidPhase.v() )
    {
        case OIL:
            return "OIL";
        case GAS:
            return "GAS";
        case WATER:
            return "WATER";
        case LIQUID:
            return "LIQ";
    }
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::drainageRadiusForExport() const
{
    return m_drainageRadiusForPI();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::gasInflowEquationForExport() const
{
    return m_gasInflowEquation().text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::automaticWellShutInForExport() const
{
    return m_automaticWellShutIn().text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::allowWellCrossFlowForExport() const
{
    return m_allowWellCrossFlow() ? "YES" : "NO";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::wellBoreFluidPVTForExport() const
{
    return QString( "%1" ).arg( m_wellBoreFluidPVTTable() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::hydrostaticDensityForExport() const
{
    return m_hydrostaticDensity().text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::fluidInPlaceRegionForExport() const
{
    return QString( "%1" ).arg( m_fluidInPlaceRegion() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletionSettings::setUnitSystemSpecificDefaults()
{
    m_mswParameters->setUnitSystemSpecificDefaults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimMswCompletionParameters* RimWellPathCompletionSettings::mswParameters() const
{
    return m_mswParameters();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters* RimWellPathCompletionSettings::mswParameters()
{
    return m_mswParameters();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRegExp RimWellPathCompletionSettings::wellNameForExportRegExp()
{
    QRegExp rx( "[\\w\\-\\_]{1,8}" );
    return rx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletionSettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    caf::PdmUiGroup* compExportGroup = uiOrdering.addNewGroup( "Completion Export Parameters" );
    compExportGroup->add( &m_wellNameForExport );
    compExportGroup->add( &m_wellGroupName );
    compExportGroup->add( &m_referenceDepth );
    compExportGroup->add( &m_preferredFluidPhase );
    compExportGroup->add( &m_drainageRadiusForPI );
    compExportGroup->add( &m_gasInflowEquation );
    compExportGroup->add( &m_automaticWellShutIn );
    compExportGroup->add( &m_allowWellCrossFlow );
    compExportGroup->add( &m_wellBoreFluidPVTTable );
    compExportGroup->add( &m_hydrostaticDensity );
    compExportGroup->add( &m_fluidInPlaceRegion );

    caf::PdmUiGroup* mswGroup = uiOrdering.addNewGroup( "Multi Segment Well Options" );
    m_mswParameters->uiOrdering( uiConfigName, *mswGroup );
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletionSettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                      const QVariant&            oldValue,
                                                      const QVariant&            newValue )
{
    if ( changedField == &m_referenceDepth )
    {
        if ( !RiaStdStringTools::isNumber( m_referenceDepth.v().toStdString(), '.' ) )
        {
            if ( !RiaStdStringTools::isNumber( m_referenceDepth.v().toStdString(), ',' ) )
            {
                // Remove invalid input text
                m_referenceDepth = "";
            }
            else
            {
                // Wrong decimal sign entered, replace , by .
                auto text        = m_referenceDepth.v();
                m_referenceDepth = text.replace( ',', '.' );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletionSettings::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                           QString                    uiConfigName,
                                                           caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiLineEditorAttribute* lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
    if ( field == &m_wellNameForExport && lineEditorAttr )
    {
        QRegExpValidator* validator = new QRegExpValidator( nullptr );
        validator->setRegExp( wellNameForExportRegExp() );
        lineEditorAttr->validator = validator;
    }
    else if ( field == &m_drainageRadiusForPI && lineEditorAttr )
    {
        caf::PdmDoubleStringValidator* validator = new caf::PdmDoubleStringValidator( "1*" );
        lineEditorAttr->validator                = validator;
    }
    else if ( field == &m_wellBoreFluidPVTTable && lineEditorAttr )
    {
        // Positive integer
        QIntValidator* validator  = new QIntValidator( 0, std::numeric_limits<int>::max(), nullptr );
        lineEditorAttr->validator = validator;
    }
    else if ( field == &m_fluidInPlaceRegion && lineEditorAttr )
    {
        // Any integer
        QIntValidator* validator =
            new QIntValidator( -std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), nullptr );
        lineEditorAttr->validator = validator;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::formatStringForExport( const QString& text, const QString& defaultValue ) const
{
    if ( text.isEmpty() ) return defaultValue;
    if ( text.contains( ' ' ) ) return QString( "'%1'" ).arg( text );
    return text;
}
