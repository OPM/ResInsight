/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogPlotCollection.h"

#include "RiaGuiApplication.h"

#include "RiuPlotMainWindow.h"

#include "RigEclipseWellLogExtractor.h"
#include "RigGeoMechCaseData.h"
#include "RigGeoMechWellLogExtractor.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimProject.h"
#include "RimWellLogPlot.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cvfAssert.h"

CAF_PDM_SOURCE_INIT( RimWellLogPlotCollection, "WellLogPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection::RimWellLogPlotCollection()
{
    CAF_PDM_InitObject( "Well Log Plots", ":/WellLogPlots16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_wellLogPlots, "WellLogPlots", "", "", "", "" );
    m_wellLogPlots.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection::~RimWellLogPlotCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RimWellLogPlotCollection::findOrCreateSimWellExtractor( const QString& simWellName,
                                                                                    const QString& caseUserDescription,
                                                                                    const RigWellPath* wellPathGeometry,
                                                                                    const RigEclipseCaseData* eclCaseData )
{
    if ( !( wellPathGeometry && eclCaseData ) ) return nullptr;

    for ( size_t exIdx = 0; exIdx < m_extractors.size(); ++exIdx )
    {
        if ( m_extractors[exIdx]->caseData() == eclCaseData && m_extractors[exIdx]->wellPathGeometry() == wellPathGeometry )
        {
            return m_extractors[exIdx].p();
        }
    }

    std::string                          errorIdName = ( simWellName + " " + caseUserDescription ).toStdString();
    cvf::ref<RigEclipseWellLogExtractor> extractor =
        new RigEclipseWellLogExtractor( eclCaseData, wellPathGeometry, errorIdName );
    m_extractors.push_back( extractor.p() );

    return extractor.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RimWellLogPlotCollection::findOrCreateExtractor( RimWellPath* wellPath, RimEclipseCase* eclCase )
{
    if ( !( wellPath && eclCase ) ) return nullptr;

    RigEclipseCaseData* eclCaseData      = eclCase->eclipseCaseData();
    auto                wellPathGeometry = wellPath->wellPathGeometry();

    if ( !( eclCaseData && wellPathGeometry ) ) return nullptr;

    for ( size_t exIdx = 0; exIdx < m_extractors.size(); ++exIdx )
    {
        if ( m_extractors[exIdx]->caseData() == eclCaseData && m_extractors[exIdx]->wellPathGeometry() == wellPathGeometry )
        {
            return m_extractors[exIdx].p();
        }
    }

    std::string errorIdName = ( wellPath->name() + " " + eclCase->caseUserDescription() ).toStdString();
    cvf::ref<RigEclipseWellLogExtractor> extractor =
        new RigEclipseWellLogExtractor( eclCaseData, wellPathGeometry, errorIdName );
    m_extractors.push_back( extractor.p() );

    return extractor.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor* RimWellLogPlotCollection::findOrCreateExtractor( RimWellPath*    wellPath,
                                                                             RimGeoMechCase* geoMechCase )
{
    if ( !( wellPath && geoMechCase ) ) return nullptr;

    RigGeoMechCaseData* caseData         = geoMechCase->geoMechData();
    auto                wellPathGeometry = wellPath->wellPathGeometry();
    if ( !( caseData && wellPathGeometry ) ) return nullptr;

    for ( size_t exIdx = 0; exIdx < m_geomExtractors.size(); ++exIdx )
    {
        if ( m_geomExtractors[exIdx]->caseData() == caseData &&
             m_geomExtractors[exIdx]->wellPathGeometry() == wellPathGeometry )
        {
            return m_geomExtractors[exIdx].p();
        }
    }

    std::string errorIdName = ( wellPath->name() + " " + geoMechCase->caseUserDescription() ).toStdString();
    cvf::ref<RigGeoMechWellLogExtractor> extractor =
        new RigGeoMechWellLogExtractor( caseData, wellPathGeometry, errorIdName );
    m_geomExtractors.push_back( extractor.p() );

    return extractor.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogPlot*> RimWellLogPlotCollection::wellLogPlots() const
{
    return m_wellLogPlots.childObjectsByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCollection::addWellLogPlot( gsl::not_null<RimWellLogPlot*> wellLogPlot )
{
    m_wellLogPlots.push_back( wellLogPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCollection::deleteAllPlots()
{
    m_wellLogPlots.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCollection::reloadAllPlots()
{
    for ( const auto& w : m_wellLogPlots() )
    {
        w->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCollection::deleteAllExtractors()
{
    m_extractors.clear();
    m_geomExtractors.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCollection::removeExtractors( const RigWellPath* wellPathGeometry )
{
    for ( int eIdx = (int)m_extractors.size() - 1; eIdx >= 0; eIdx-- )
    {
        if ( m_extractors[eIdx]->wellPathGeometry() == wellPathGeometry )
        {
            m_extractors.eraseAt( eIdx );
        }
    }

    for ( int eIdx = (int)m_geomExtractors.size() - 1; eIdx >= 0; eIdx-- )
    {
        if ( m_geomExtractors[eIdx]->wellPathGeometry() == wellPathGeometry )
        {
            m_geomExtractors.eraseAt( eIdx );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCollection::removeExtractors( const RigEclipseCaseData* caseData )
{
    for ( int eIdx = (int)m_extractors.size() - 1; eIdx >= 0; eIdx-- )
    {
        if ( m_extractors[eIdx]->caseData() == caseData )
        {
            m_extractors.eraseAt( eIdx );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotCollection::removeExtractors( const RigGeoMechCaseData* caseData )
{
    for ( int eIdx = (int)m_geomExtractors.size() - 1; eIdx >= 0; eIdx-- )
    {
        if ( m_geomExtractors[eIdx]->caseData() == caseData )
        {
            m_geomExtractors.eraseAt( eIdx );
        }
    }
}

void RimWellLogPlotCollection::onChildDeleted( caf::PdmChildArrayFieldHandle*      childArray,
                                               std::vector<caf::PdmObjectHandle*>& referringObjects )
{
    // Make sure the plot collection disappears with the last plot
    if ( m_wellLogPlots().empty() )
    {
        RimProject* project = RimProject::current();
        if ( project )
        {
            project->updateConnectedEditors();
        }
    }

    RiuPlotMainWindow* mainPlotWindow = RiaGuiApplication::instance()->mainPlotWindow();
    mainPlotWindow->updateWellLogPlotToolBar();
}
