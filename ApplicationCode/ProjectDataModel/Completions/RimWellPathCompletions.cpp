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

#include "RimWellPathCompletions.h"

#include "RiaStdStringTools.h"

#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimFractureModel.h"
#include "RimFractureModelCollection.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"
#include "RimWellPathComponentInterface.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"
#include "RimWellPathValve.h"

#include "cvfAssert.h"

#include "cafPdmDoubleStringValidator.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include <QRegExpValidator>
#include <cmath>

//--------------------------------------------------------------------------------------------------
/// Internal constants
//--------------------------------------------------------------------------------------------------
#define DOUBLE_INF std::numeric_limits<double>::infinity()

namespace caf
{
template <>
void RimWellPathCompletions::WellTypeEnum::setUp()
{
    addItem( RimWellPathCompletions::OIL, "OIL", "Oil" );
    addItem( RimWellPathCompletions::GAS, "GAS", "Gas" );
    addItem( RimWellPathCompletions::WATER, "WATER", "Water" );
    addItem( RimWellPathCompletions::LIQUID, "LIQUID", "Liquid" );

    setDefault( RimWellPathCompletions::OIL );
}

template <>
void RimWellPathCompletions::GasInflowEnum::setUp()
{
    addItem( RimWellPathCompletions::STANDARD_EQ, "STD", "Standard" );
    addItem( RimWellPathCompletions::RUSSELL_GOODRICH, "R-G", "Russell-Goodrich" );
    addItem( RimWellPathCompletions::DRY_GAS_PSEUDO_PRESSURE, "P-P", "Dry Gas Pseudo-Pressure" );
    addItem( RimWellPathCompletions::GENERALIZED_PSEUDO_PRESSURE, "GPP", "Generalized Pseudo-Pressure" );

    setDefault( RimWellPathCompletions::STANDARD_EQ );
}

template <>
void RimWellPathCompletions::AutomaticWellShutInEnum::setUp()
{
    addItem( RimWellPathCompletions::ISOLATE_FROM_FORMATION, "SHUT", "Isolate from Formation" );
    addItem( RimWellPathCompletions::STOP_ABOVE_FORMATION, "STOP", "Stop above Formation" );

    setDefault( RimWellPathCompletions::STOP_ABOVE_FORMATION );
}

template <>
void RimWellPathCompletions::HydrostaticDensityEnum::setUp()
{
    addItem( RimWellPathCompletions::SEGMENTED, "SEG", "Segmented" );
    addItem( RimWellPathCompletions::AVERAGED, "AVG", "Averaged" );

    setDefault( RimWellPathCompletions::SEGMENTED );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimWellPathCompletions, "WellPathCompletions" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathCompletions::RimWellPathCompletions()
{
    CAF_PDM_InitObject( "Completions", ":/CompletionsSymbol16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_perforationCollection, "Perforations", "Perforations", "", "", "" );
    m_perforationCollection = new RimPerforationCollection;
    m_perforationCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_fishbonesCollection, "Fishbones", "Fishbones", "", "", "" );
    m_fishbonesCollection = new RimFishbonesCollection;
    m_fishbonesCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_fractureCollection, "Fractures", "Fractures", "", "", "" );
    m_fractureCollection = new RimWellPathFractureCollection;
    m_fractureCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_fractureModelCollection, "FractureModels", "Fracture Models", "", "", "" );
    m_fractureModelCollection = new RimFractureModelCollection;
    m_fractureModelCollection.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_wellNameForExport, "WellNameForExport", QString(), "Well Name", "", "", "" );
    m_wellNameForExport.uiCapability()->setUiEditorTypeName( caf::PdmUiLineEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_wellGroupName, "WellGroupNameForExport", QString(), "Well Group Name", "", "", "" );
    CAF_PDM_InitField( &m_referenceDepth, "ReferenceDepthForExport", QString(), "Reference Depth for BHP", "", "", "" );
    CAF_PDM_InitField( &m_preferredFluidPhase, "WellTypeForExport", WellTypeEnum(), "Preferred Fluid Phase", "", "", "" );
    CAF_PDM_InitField( &m_drainageRadiusForPI, "DrainageRadiusForPI", QString( "0.0" ), "Drainage Radius for PI", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_gasInflowEquation, "GasInflowEq", "Gas Inflow Equation", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_automaticWellShutIn, "AutoWellShutIn", "Automatic well shut-in", "", "", "" );
    CAF_PDM_InitField( &m_allowWellCrossFlow, "AllowWellCrossFlow", true, "Allow Well Cross-Flow", "", "", "" );
    CAF_PDM_InitField( &m_wellBoreFluidPVTTable, "WellBoreFluidPVTTable", 0, "Wellbore Fluid PVT table", "", "", "" );
    CAF_PDM_InitFieldNoDefault( &m_hydrostaticDensity, "HydrostaticDensity", "Hydrostatic Density", "", "", "" );
    CAF_PDM_InitField( &m_fluidInPlaceRegion, "FluidInPlaceRegion", 0, "Fluid In-Place Region", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RimWellPathCompletions::fishbonesCollection() const
{
    CVF_ASSERT( m_fishbonesCollection );

    return m_fishbonesCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPerforationCollection* RimWellPathCompletions::perforationCollection() const
{
    CVF_ASSERT( m_perforationCollection );

    return m_perforationCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::setWellNameForExport( const QString& name )
{
    auto n              = name;
    m_wellNameForExport = n.remove( ' ' );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::updateWellPathNameHasChanged( const QString& newWellPathName,
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
QString RimWellPathCompletions::wellNameForExport() const
{
    return formatStringForExport( m_wellNameForExport() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::wellGroupNameForExport() const
{
    return formatStringForExport( m_wellGroupName, "1*" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::referenceDepthForExport() const
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
QString RimWellPathCompletions::wellTypeNameForExport() const
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
RimWellPathFractureCollection* RimWellPathCompletions::fractureCollection() const
{
    CVF_ASSERT( m_fractureCollection );

    return m_fractureCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelCollection* RimWellPathCompletions::fractureModelCollection() const
{
    CVF_ASSERT( m_fractureModelCollection );

    return m_fractureModelCollection;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPathValve*> RimWellPathCompletions::valves() const
{
    std::vector<RimWellPathValve*> allValves;
    this->descendantsIncludingThisOfType( allValves );
    return allValves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<const RimWellPathComponentInterface*> RimWellPathCompletions::allCompletions() const
{
    std::vector<const RimWellPathComponentInterface*> completions;

    for ( const RimWellPathFracture* fracture : fractureCollection()->allFractures() )
    {
        completions.push_back( fracture );
    }
    for ( const RimFishbonesMultipleSubs* fishbones : fishbonesCollection()->allFishbonesSubs() )
    {
        completions.push_back( fishbones );
    }
    for ( const RimPerforationInterval* perforation : perforationCollection()->perforations() )
    {
        completions.push_back( perforation );
    }

    std::vector<RimWellPathValve*> allValves = valves();
    for ( const RimWellPathValve* valve : allValves )
    {
        completions.push_back( valve );
    }

    return completions;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathCompletions::hasCompletions() const
{
    if ( !fractureCollection()->allFractures().empty() || !fractureModelCollection()->allFractureModels().empty() )
    {
        return true;
    }

    return !fishbonesCollection()->allFishbonesSubs().empty() ||
           !fishbonesCollection()->wellPathCollection()->wellPaths().empty() ||
           !perforationCollection()->perforations().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::drainageRadiusForExport() const
{
    return m_drainageRadiusForPI();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::gasInflowEquationForExport() const
{
    return m_gasInflowEquation().text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::automaticWellShutInForExport() const
{
    return m_automaticWellShutIn().text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::allowWellCrossFlowForExport() const
{
    return m_allowWellCrossFlow() ? "YES" : "NO";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::wellBoreFluidPVTForExport() const
{
    return QString( "%1" ).arg( m_wellBoreFluidPVTTable() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::hydrostaticDensityForExport() const
{
    return m_hydrostaticDensity().text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::fluidInPlaceRegionForExport() const
{
    return QString( "%1" ).arg( m_fluidInPlaceRegion() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::setUnitSystemSpecificDefaults()
{
    m_fishbonesCollection->setUnitSystemSpecificDefaults();
    m_fractureCollection->setUnitSystemSpecificDefaults();
    m_perforationCollection->setUnitSystemSpecificDefaults();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QRegExp RimWellPathCompletions::wellNameForExportRegExp()
{
    QRegExp rx( "[\\w\\-\\_]{1,8}" );
    return rx;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.skipRemainingChildren( true );

    if ( !perforationCollection()->perforations().empty() )
    {
        uiTreeOrdering.add( &m_perforationCollection );
    }

    if ( !fishbonesCollection()->allFishbonesSubs().empty() ||
         !fishbonesCollection()->wellPathCollection()->wellPaths().empty() )
    {
        uiTreeOrdering.add( &m_fishbonesCollection );
    }

    if ( !fractureCollection()->allFractures().empty() )
    {
        uiTreeOrdering.add( &m_fractureCollection );
    }

    if ( !fractureModelCollection()->allFractureModels().empty() )
    {
        uiTreeOrdering.add( &m_fractureModelCollection );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
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
void RimWellPathCompletions::defineEditorAttribute( const caf::PdmFieldHandle* field,
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
QString RimWellPathCompletions::formatStringForExport( const QString& text, const QString& defaultValue ) const
{
    if ( text.isEmpty() ) return defaultValue;
    if ( text.contains( ' ' ) ) return QString( "'%1'" ).arg( text );
    return text;
}
