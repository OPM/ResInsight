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

#include "RimPltPlotCollection.h"

#include "RigGeoMechCaseData.h"
#include "Well/RigEclipseWellLogExtractor.h"
#include "Well/RigGeoMechWellLogExtractor.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"
#include "RimWellLogPlot.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPltPlot.h"

#include "cvfAssert.h"

CAF_PDM_SOURCE_INIT( RimPltPlotCollection, "WellPltPlotCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPltPlotCollection::RimPltPlotCollection()
{
    CAF_PDM_InitObject( "PLT Plots", ":/WellAllocPlots16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_pltPlots, "PltPlots", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RimPltPlotCollection::findOrCreateSimWellExtractor( const QString&                    simWellName,
                                                                                const QString&                    caseUserDescription,
                                                                                gsl::not_null<const RigWellPath*> wellPathGeometry,
                                                                                gsl::not_null<const RigEclipseCaseData*> eclCaseData )
{
    if ( !( wellPathGeometry && eclCaseData ) )
    {
        return nullptr;
    }

    for ( size_t exIdx = 0; exIdx < m_extractors.size(); ++exIdx )
    {
        if ( m_extractors[exIdx]->caseData() == eclCaseData && m_extractors[exIdx]->wellPathGeometry() == wellPathGeometry )
        {
            return m_extractors[exIdx].p();
        }
    }

    std::string                          errorIdName = ( simWellName + " " + caseUserDescription ).toStdString();
    cvf::ref<RigEclipseWellLogExtractor> extractor   = new RigEclipseWellLogExtractor( eclCaseData, wellPathGeometry, errorIdName );
    m_extractors.push_back( extractor.p() );

    return extractor.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEclipseWellLogExtractor* RimPltPlotCollection::findOrCreateExtractor( gsl::not_null<RimWellPath*>    wellPath,
                                                                         gsl::not_null<RimEclipseCase*> eclCase )
{
    if ( !( wellPath && eclCase && wellPath->wellPathGeometry() && eclCase->eclipseCaseData() ) )
    {
        return nullptr;
    }

    RigEclipseCaseData* eclCaseData      = eclCase->eclipseCaseData();
    RigWellPath*        wellPathGeometry = wellPath->wellPathGeometry();
    for ( size_t exIdx = 0; exIdx < m_extractors.size(); ++exIdx )
    {
        if ( m_extractors[exIdx]->caseData() == eclCaseData && m_extractors[exIdx]->wellPathGeometry() == wellPathGeometry )
        {
            return m_extractors[exIdx].p();
        }
    }

    std::string                          errorIdName = ( wellPath->name() + " " + eclCase->caseUserDescription() ).toStdString();
    cvf::ref<RigEclipseWellLogExtractor> extractor   = new RigEclipseWellLogExtractor( eclCaseData, wellPathGeometry, errorIdName );

    m_extractors.push_back( extractor.p() );

    return extractor.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigGeoMechWellLogExtractor*
    RimPltPlotCollection::findOrCreateExtractor( gsl::not_null<RimWellPath*> wellPath, gsl::not_null<RimGeoMechCase*> geomCase, int partId )
{
    if ( !( wellPath && geomCase && wellPath->wellPathGeometry() && geomCase->geoMechData() ) )
    {
        return nullptr;
    }

    RigGeoMechCaseData* geomCaseData     = geomCase->geoMechData();
    RigWellPath*        wellPathGeometry = wellPath->wellPathGeometry();
    for ( size_t exIdx = 0; exIdx < m_geomExtractors.size(); ++exIdx )
    {
        if ( ( m_geomExtractors[exIdx]->caseData() == geomCaseData ) &&
             ( m_geomExtractors[exIdx]->wellPathGeometry() == wellPathGeometry ) && ( m_geomExtractors[exIdx]->partId() == partId ) )
        {
            return m_geomExtractors[exIdx].p();
        }
    }

    std::string                          errorIdName = ( wellPath->name() + " " + geomCase->caseUserDescription() ).toStdString();
    cvf::ref<RigGeoMechWellLogExtractor> extractor = new RigGeoMechWellLogExtractor( geomCaseData, partId, wellPathGeometry, errorIdName );

    if ( extractor->valid() )
    {
        m_geomExtractors.push_back( extractor.p() );
        return extractor.p();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPltPlotCollection::removeExtractors( const RigWellPath* wellPathGeometry )
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
void RimPltPlotCollection::removeExtractors( const RigEclipseCaseData* caseData )
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
void RimPltPlotCollection::removeExtractors( const RigGeoMechCaseData* caseData )
{
    for ( int eIdx = (int)m_geomExtractors.size() - 1; eIdx >= 0; eIdx-- )
    {
        if ( m_geomExtractors[eIdx]->caseData() == caseData )
        {
            m_geomExtractors.eraseAt( eIdx );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPltPlotCollection::deleteAllExtractors()
{
    m_extractors.clear();
    m_geomExtractors.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RimWellPltPlot*> RimPltPlotCollection::pltPlots() const
{
    std::vector<RimWellPltPlot*> plots;
    for ( const auto& plot : m_pltPlots )
    {
        plots.push_back( plot );
    }
    return plots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPltPlotCollection::loadDataAndUpdateAllPlots()
{
    for ( auto& plot : m_pltPlots )
    {
        plot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RimPltPlotCollection::plotCount() const
{
    return m_pltPlots.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPltPlotCollection::addPlot( gsl::not_null<RimWellPltPlot*> newPlot )
{
    m_pltPlots.push_back( newPlot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPltPlotCollection::removePlot( gsl::not_null<RimWellPltPlot*> plot )
{
    m_pltPlots.removeChild( plot );
    updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPltPlotCollection::deleteAllPlots()
{
    m_pltPlots.deleteChildren();
}
