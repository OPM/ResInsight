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
#include "RiaWellNameComparer.h"

#include "RimMswCompletionParameters.h"
#include "RimProject.h"
#include "RimWellPath.h"

#include "cafPdmDoubleStringValidator.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiLineEditor.h"
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
    CAF_PDM_InitScriptableObject( "Completion Settings", ":/CompletionsSymbol16x16.png" );

    CAF_PDM_InitScriptableField( &m_wellNameForExport, "WellNameForExport", QString(), "Well Name" );
    m_wellNameForExport.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableFieldNoDefault( &m_referenceDepth, "ReferenceDepth", "BHP Reference Depth" );
    m_referenceDepth.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );
    CAF_PDM_InitScriptableFieldWithScriptKeywordNoDefault( &m_drainageRadius, "DrainageRadius", "DrainageRadiusForPi", "Drainage Radius for PI" );
    m_drainageRadius.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitScriptableFieldWithScriptKeyword( &m_groupName, "WellGroupNameForExport", "GroupNameForExport", QString(), "Group Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_preferredFluidPhase, "WellTypeForExport", "Preferred Fluid Phase" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_gasInflowEquation, "GasInflowEq", "Gas Inflow Equation" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_automaticWellShutIn, "AutoWellShutIn", "Automatic well shut-in" );
    CAF_PDM_InitScriptableField( &m_allowWellCrossFlow, "AllowWellCrossFlow", true, "Allow Well Cross-Flow" );
    CAF_PDM_InitScriptableFieldWithScriptKeyword( &m_wellBoreFluidPVTTable,
                                                  "WellBoreFluidPVTTable",
                                                  "WellBoreFluidPvtTable",
                                                  0,
                                                  "Wellbore Fluid PVT table" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_hydrostaticDensity, "HydrostaticDensity", "Hydrostatic Density" );
    CAF_PDM_InitScriptableField( &m_fluidInPlaceRegion, "FluidInPlaceRegion", 0, "Fluid In-Place Region" );

    CAF_PDM_InitFieldNoDefault( &m_mswParameters, "MswParameters", "Multi Segment Well Parameters" );
    m_mswParameters = new RimMswCompletionParameters;
    m_mswParameters.uiCapability()->setUiTreeChildrenHidden( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_mswLinerDiameter, "MswLinerDiameter", "MSW Liner Diameter" );
    m_mswLinerDiameter.registerGetMethod( this, &RimWellPathCompletionSettings::mswLinerDiameter );
    m_mswLinerDiameter.registerSetMethod( this, &RimWellPathCompletionSettings::setMswLinerDiameter );

    CAF_PDM_InitScriptableFieldNoDefault( &m_mswRoughness, "MswRoughness", "MSW Roughness" );
    m_mswRoughness.registerGetMethod( this, &RimWellPathCompletionSettings::mswRoughness );
    m_mswRoughness.registerSetMethod( this, &RimWellPathCompletionSettings::setMswRoughness );

    CAF_PDM_InitField( &m_referenceDepth_OBSOLETE, "ReferenceDepthForExport", QString(), "Reference Depth for BHP" );
    CAF_PDM_InitField( &m_drainageRadiusForPI_OBSOLETE, "DrainageRadiusForPI", QString( "0.0" ), "Drainage Radius for PI" );
    m_referenceDepth_OBSOLETE.xmlCapability()->disableIO();
    m_drainageRadiusForPI_OBSOLETE.xmlCapability()->disableIO();
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
void RimWellPathCompletionSettings::initAfterRead()
{
    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2025.10" ) )
    {
        double      tmpValue = 0.0;
        std::string refDepth = m_referenceDepth_OBSOLETE.v().toStdString();
        if ( RiaStdStringTools::isNumber( refDepth, '.' ) )
        {
            if ( RiaStdStringTools::toDouble( refDepth, tmpValue ) )
            {
                m_referenceDepth = std::optional<double>( tmpValue );
            }
        }

        std::string radius = m_drainageRadiusForPI_OBSOLETE.v().toStdString();
        if ( RiaStdStringTools::isNumber( radius, '.' ) )
        {
            if ( RiaStdStringTools::toDouble( radius, tmpValue ) )
            {
                m_drainageRadius = std::optional<double>( tmpValue );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCompletionSettings& RimWellPathCompletionSettings::operator=( const RimWellPathCompletionSettings& rhs )
{
    m_wellNameForExport     = rhs.m_wellNameForExport;
    m_groupName             = rhs.m_groupName;
    m_referenceDepth        = rhs.m_referenceDepth;
    m_preferredFluidPhase   = rhs.m_preferredFluidPhase;
    m_drainageRadius        = rhs.m_drainageRadius;
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
    m_wellNameForExport = RiaWellNameComparer::removeSpacesFromName( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletionSettings::updateWellPathNameHasChanged( const QString& newWellPathName, const QString& previousWellPathName )
{
    auto previousWellNameWithoutSpaces = RiaWellNameComparer::removeSpacesFromName( previousWellPathName );

    if ( m_wellNameForExport().isEmpty() || m_wellNameForExport == previousWellNameWithoutSpaces )
    {
        setWellNameForExport( newWellPathName );
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
QString RimWellPathCompletionSettings::groupNameForExport() const
{
    return formatStringForExport( m_groupName, "1*" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::wellName() const
{
    return m_wellNameForExport();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::groupName() const
{
    return m_groupName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletionSettings::referenceDepthForExport() const
{
    if ( m_referenceDepth().has_value() ) return QString::number( m_referenceDepth().value() );
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
    if ( m_drainageRadius().has_value() ) return QString::number( m_drainageRadius().value() );
    return "1*";
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
int RimWellPathCompletionSettings::wellBoreFluidPVT() const
{
    return m_wellBoreFluidPVTTable();
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
int RimWellPathCompletionSettings::fluidInPlaceRegion() const
{
    return m_fluidInPlaceRegion();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRegularExpression RimWellPathCompletionSettings::wellNameForExportRegExp()
{
    return QRegularExpression( "[\\w\\-\\_]{1,8}" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMswCompletionParameters* RimWellPathCompletionSettings::mswCompletionParameters() const
{
    return m_mswParameters();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletionSettings::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    auto wellPath = firstAncestorOrThisOfTypeAsserted<RimWellPath>();

    if ( wellPath->isTopLevelWellPath() )
    {
        caf::PdmUiGroup* compExportGroup = uiOrdering.addNewGroup( "Completion Export Parameters" );
        compExportGroup->add( &m_wellNameForExport );
        compExportGroup->add( &m_groupName );
        compExportGroup->add( &m_referenceDepth );
        compExportGroup->add( &m_preferredFluidPhase );
        compExportGroup->add( &m_drainageRadius );
        compExportGroup->add( &m_gasInflowEquation );
        compExportGroup->add( &m_automaticWellShutIn );
        compExportGroup->add( &m_allowWellCrossFlow );
        compExportGroup->add( &m_wellBoreFluidPVTTable );
        compExportGroup->add( &m_hydrostaticDensity );
        compExportGroup->add( &m_fluidInPlaceRegion );
    }

    caf::PdmUiGroup* mswGroup = uiOrdering.addNewGroup( "Multi Segment Well Options" );
    m_mswParameters->uiOrdering( uiConfigName, *mswGroup );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletionSettings::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    m_mswParameters->fieldChangedByUi( changedField, oldValue, newValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletionSettings::defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                           QString                    uiConfigName,
                                                           caf::PdmUiEditorAttribute* attribute )
{
    caf::PdmUiLineEditorAttribute* lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
    if ( field == &m_wellBoreFluidPVTTable && lineEditorAttr )
    {
        // Positive integer
        QIntValidator* validator  = new QIntValidator( 0, std::numeric_limits<int>::max(), nullptr );
        lineEditorAttr->validator = validator;
    }
    else if ( field == &m_fluidInPlaceRegion && lineEditorAttr )
    {
        // Any integer
        QIntValidator* validator  = new QIntValidator( -std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), nullptr );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletionSettings::setMswRoughness( const double& roughness )
{
    m_mswParameters->setRoughnessFactor( roughness );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathCompletionSettings::mswRoughness() const
{
    return m_mswParameters->roughnessFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletionSettings::setMswLinerDiameter( const double& diameter )
{
    m_mswParameters->setLinerDiameter( diameter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellPathCompletionSettings::mswLinerDiameter() const
{
    return m_mswParameters->linerDiameter();
}
