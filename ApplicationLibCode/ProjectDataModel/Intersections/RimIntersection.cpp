/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimIntersection.h"

#include "RigEclipseCaseData.h"
#include "RigGeoMechCaseData.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "RimIntersectionResultDefinition.h"
#include "RimIntersectionResultsDefinitionCollection.h"
#include "RimSurfaceIntersectionBand.h"
#include "RimSurfaceIntersectionCollection.h"
#include "RimSurfaceIntersectionCurve.h"

#include "RivEclipseIntersectionGrid.h"
#include "RivFemIntersectionGrid.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_ABSTRACT_SOURCE_INIT( RimIntersection, "RimIntersectionHandle" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersection::RimIntersection()
{
    CAF_PDM_InitField( &m_isActive, "Active", true, "Active" );
    m_isActive.uiCapability()->setUiHidden( true );

    CAF_PDM_InitField( &m_showInactiveCells, "ShowInactiveCells", false, "Show Inactive Cells" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_showInactiveCells );

    CAF_PDM_InitField( &m_useSeparateDataSource, "UseSeparateIntersectionDataSource", true, "Enable" );
    CAF_PDM_InitFieldNoDefault( &m_separateDataSource, "SeparateIntersectionDataSource", "Source" );

    CAF_PDM_InitFieldNoDefault( &m_surfaceIntersections, "SurfaceIntersections", "Surface Intersections" );
    m_surfaceIntersections = new RimSurfaceIntersectionCollection;
    m_surfaceIntersections->objectChanged.connect( this, &RimIntersection::onSurfaceIntersectionsChanged );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersection::~RimIntersection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersection::isActive() const
{
    return m_isActive();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersection::setActive( bool isActive )
{
    m_isActive = isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersection::isInactiveCellsVisible() const
{
    return m_showInactiveCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultDefinition* RimIntersection::activeSeparateResultDefinition()
{
    updateDefaultSeparateDataSource();

    if ( !m_useSeparateDataSource ) return nullptr;

    if ( !m_separateDataSource ) return nullptr;

    if ( !m_separateDataSource->isActive() ) return nullptr;

    if ( !findSeparateResultsCollection() ) return nullptr;

    if ( !findSeparateResultsCollection()->isActive() ) return nullptr;

    return m_separateDataSource;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimIntersection::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_separateDataSource )
    {
        if ( findSeparateResultsCollection() )
        {
            std::vector<RimIntersectionResultDefinition*> iResDefs = findSeparateResultsCollection()->intersectionResultsDefinitions();

            for ( auto iresdef : iResDefs )
            {
                options.push_back( caf::PdmOptionItemInfo( iresdef->autoName(), iresdef ) );
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionResultsDefinitionCollection* RimIntersection::findSeparateResultsCollection()
{
    auto view = firstAncestorOrThisOfTypeAsserted<RimGridView>();
    if ( view ) return view->separateIntersectionResultsCollection();
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIntersection::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersection::defineSeparateDataSourceUi( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    QString inactiveText;
    if ( !activeSeparateResultDefinition() )
    {
        inactiveText = " (Inactive)";
    }

    caf::PdmUiGroup* separateResultsGroup = uiOrdering.addNewGroupWithKeyword( "Result Reference" + inactiveText, "SeparateResultReference" );
    separateResultsGroup->setCollapsedByDefault();
    separateResultsGroup->add( &m_useSeparateDataSource );
    separateResultsGroup->add( &m_separateDataSource );
    m_separateDataSource.uiCapability()->setUiReadOnly( !m_useSeparateDataSource() );

    m_separateDataSource.uiCapability()->setUiName( "Source" + inactiveText );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersection::updateDefaultSeparateDataSource()
{
    if ( m_separateDataSource() == nullptr )
    {
        RimIntersectionResultsDefinitionCollection* defcoll = findSeparateResultsCollection();

        if ( defcoll )
        {
            std::vector<RimIntersectionResultDefinition*> iResDefs = defcoll->intersectionResultsDefinitions();

            if ( !iResDefs.empty() )
            {
                m_separateDataSource = iResDefs[0];
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceIntersectionCollection* RimIntersection::surfaceIntersectionCollection() const
{
    return m_surfaceIntersections();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurfaceIntersectionCurve*> RimIntersection::surfaceIntersectionCurves() const
{
    return m_surfaceIntersections->surfaceIntersectionCurves();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurfaceIntersectionBand*> RimIntersection::surfaceIntersectionBands() const
{
    return m_surfaceIntersections->surfaceIntersectionBands();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceIntersectionCurve* RimIntersection::addIntersectionCurve()
{
    return m_surfaceIntersections->addIntersectionCurve();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurfaceIntersectionBand* RimIntersection::addIntersectionBand()
{
    return m_surfaceIntersections->addIntersectionBand();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIntersection::supportsSurfaceIntersectionCurves() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionCurtain RimIntersection::surfaceCurtain() const
{
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimIntersection::defaultCurtainExtent()
{
    return 10000.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIntersectionCurtain RimIntersection::verticalCurtain( const std::vector<cvf::Vec3d>& trace, double topZ, double bottomZ )
{
    RimIntersectionCurtain curtain;
    curtain.trace = trace;
    curtain.pillars.reserve( trace.size() );

    for ( const auto& point : trace )
    {
        curtain.pillars.emplace_back( cvf::Vec3d( point.x(), point.y(), topZ ), cvf::Vec3d( point.x(), point.y(), bottomZ ) );
    }

    return curtain;
}

//--------------------------------------------------------------------------------------------------
/// Overridden by the intersection types that build their geometry from a part manager
//--------------------------------------------------------------------------------------------------
void RimIntersection::rebuildGeometryAndScheduleCreateDisplayModel()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersection::appendSurfaceIntersectionsToTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering )
{
    for ( auto c : m_surfaceIntersections->surfaceIntersectionCurves() )
    {
        uiTreeOrdering.add( c );
    }
    for ( auto c : m_surfaceIntersections->surfaceIntersectionBands() )
    {
        uiTreeOrdering.add( c );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersection::appendCommonMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder << "RicCreateSurfaceIntersectionBandFeature";
    menuBuilder << "RicCreateSurfaceIntersectionCurveFeature";
    menuBuilder.addSeparator();
    menuBuilder << "RicPasteIntersectionsFeature";
    menuBuilder.addSeparator();
    menuBuilder << "RicAppendIntersectionFeature";
    menuBuilder << "RicAppendIntersectionBoxFeature";
    menuBuilder << "RicAppendIjkIntersectionFeature";
    menuBuilder.addSeparator();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIntersection::onSurfaceIntersectionsChanged( const caf::SignalEmitter* emitter )
{
    updateAllRequiredEditors();
    rebuildGeometryAndScheduleCreateDisplayModel();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RivIntersectionHexGridInterface> RimIntersection::createHexGridInterface()
{
    auto geoView = firstAncestorOrThisOfType<RimGeoMechView>();

    RimIntersectionResultDefinition* resDef = activeSeparateResultDefinition();
    if ( resDef && resDef->activeCase() )
    {
        // Eclipse case

        auto* eclipseCase = dynamic_cast<RimEclipseCase*>( resDef->activeCase() );
        if ( eclipseCase && eclipseCase->eclipseCaseData() )
        {
            return new RivEclipseIntersectionGrid( eclipseCase->eclipseCaseData()->mainGrid(),
                                                   eclipseCase->eclipseCaseData()->activeCellInfo(
                                                       resDef->eclipseResultDefinition()->porosityModel() ),
                                                   isInactiveCellsVisible() );
        }

        // Geomech case

        auto* geomCase = dynamic_cast<RimGeoMechCase*>( resDef->activeCase() );

        if ( geomCase && geomCase->geoMechData() && geomCase->geoMechData()->femParts() && geoView )
        {
            return new RivFemIntersectionGrid( geomCase->geoMechData()->femParts(), geoView->partsCollection() );
        }
    }

    auto eclipseView = firstAncestorOrThisOfType<RimEclipseView>();
    if ( eclipseView && eclipseView->mainGrid() )
    {
        RigMainGrid* grid = eclipseView->mainGrid();

        return new RivEclipseIntersectionGrid( grid, eclipseView->currentActiveCellInfo(), isInactiveCellsVisible() );
    }

    if ( geoView && geoView->femParts() )
    {
        return new RivFemIntersectionGrid( geoView->femParts(), geoView->partsCollection() );
    }

    return nullptr;
}
