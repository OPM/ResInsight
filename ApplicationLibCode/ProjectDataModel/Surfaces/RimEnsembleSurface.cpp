/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RimEnsembleSurface.h"

#include "RiaLogging.h"

#include "RigSurfaceResampler.h"
#include "RigSurfaceStatisticsCalculator.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleStatisticsSurface.h"
#include "RimFileSurface.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSurfaceCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimEnsembleSurface, "EnsembleSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleSurface::RimEnsembleSurface()
{
    CAF_PDM_InitScriptableObject( "Ensemble Surface", ":/ReservoirSurfaces16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_fileSurfaces, "FileSurfaces", "", "", "", "" );
    m_fileSurfaces.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_statisticsSurfaces, "StatisticsSurfaces", "", "", "", "" );
    m_statisticsSurfaces.uiCapability()->setUiHidden( true );

    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSet, "FilterEnsembleCurveSet", "Filter by Ensemble Curve Set", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::removeFileSurface( RimFileSurface* fileSurface )
{
    m_fileSurfaces.removeChildObject( fileSurface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::addFileSurface( RimFileSurface* fileSurface )
{
    m_fileSurfaces.push_back( fileSurface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFileSurface*> RimEnsembleSurface::fileSurfaces() const
{
    return m_fileSurfaces().childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurface*> RimEnsembleSurface::surfaces() const
{
    std::vector<RimSurface*> surfaces;
    for ( auto fs : m_fileSurfaces.childObjects() )
        surfaces.push_back( fs );

    for ( auto s : m_statisticsSurfaces.childObjects() )
        surfaces.push_back( s );

    return surfaces;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::loadDataAndUpdate()
{
    for ( auto& w : m_fileSurfaces )
    {
        if ( !w->onLoadData() )
        {
            RiaLogging::warning( QString( "Failed to load surface: %1" ).arg( w->surfaceFilePath() ) );
        }
    }

    std::vector<RimFileSurface*> fileSurfaces = m_fileSurfaces.childObjects();
    if ( m_ensembleCurveSet != nullptr )
    {
        fileSurfaces = filterByEnsembleCurveSet( fileSurfaces );
    }

    m_statisticsSurfaces.deleteAllChildObjects();
    m_statisticsSurfaces.clear();

    if ( !fileSurfaces.empty() )
    {
        cvf::ref<RigSurface> firstSurface = fileSurfaces[0]->surfaceData();

        std::vector<cvf::ref<RigSurface>> surfaces;
        for ( auto& w : fileSurfaces )
            surfaces.push_back( RigSurfaceResampler::resampleSurface( firstSurface, w->surfaceData() ) );

        m_statisticsSurface = RigSurfaceStatisticsCalculator::computeStatistics( surfaces );
        if ( !m_statisticsSurface.isNull() )
        {
            std::vector<RigSurfaceStatisticsCalculator::StatisticsType> statisticsTypes =
                { RigSurfaceStatisticsCalculator::StatisticsType::MIN,
                  RigSurfaceStatisticsCalculator::StatisticsType::MAX,
                  RigSurfaceStatisticsCalculator::StatisticsType::MEAN,
                  RigSurfaceStatisticsCalculator::StatisticsType::P10,
                  RigSurfaceStatisticsCalculator::StatisticsType::P50,
                  RigSurfaceStatisticsCalculator::StatisticsType::P90 };
            for ( auto s : statisticsTypes )
            {
                auto statSurface = new RimEnsembleStatisticsSurface;
                statSurface->setStatisticsType( s );
                m_statisticsSurfaces.push_back( statSurface );
                statSurface->onLoadData();
            }
        }
    }

    RimSurfaceCollection* surfColl;
    this->firstAncestorOrThisOfTypeAsserted( surfColl );

    std::vector<RimSurface*> surfacesToUpdate;
    surfColl->updateViews( surfaces(), false );
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFileSurface*>
    RimEnsembleSurface::filterByEnsembleCurveSet( const std::vector<RimFileSurface*>& fileSurfaces ) const
{
    std::vector<RimFileSurface*> filteredCases;

    if ( m_ensembleCurveSet != nullptr )
    {
        // Get the summary cases from the related ensemble summary curve set.
        RimSummaryCaseCollection* summaryCaseCollection = m_ensembleCurveSet->summaryCaseCollection();

        //
        std::vector<RimSummaryCase*> sumCases =
            m_ensembleCurveSet->filterEnsembleCases( summaryCaseCollection->allSummaryCases() );
        for ( auto sumCase : sumCases )
        {
            for ( auto fileSurface : fileSurfaces )
            {
                if ( isSameRealization( sumCase, fileSurface ) )
                {
                    filteredCases.push_back( fileSurface );
                }
            }
        }
    }
    else
    {
        filteredCases = fileSurfaces;
    }

    return filteredCases;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEnsembleSurface::isSameRealization( RimSummaryCase* summaryCase, RimFileSurface* fileSurface ) const
{
    // TODO: duplication with RimEnsembleWellLogCurveSet::isSameRealization
    QString fileSurfaceName = fileSurface->surfaceFilePath();
    if ( summaryCase->hasCaseRealizationParameters() )
    {
        // TODO: make less naive..
        int     realizationNumber   = summaryCase->caseRealizationParameters()->realizationNumber();
        QString summaryCaseFileName = summaryCase->summaryHeaderFilename();

        if ( fileSurfaceName.contains( QString( "realization-%1" ).arg( realizationNumber ) ) )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigSurface* RimEnsembleSurface::statisticsSurface() const
{
    return m_statisticsSurface.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimEnsembleSurface::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                         bool*                      useOptionsOnly )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_ensembleCurveSet )
    {
        options.push_back( caf::PdmOptionItemInfo( "None", nullptr ) );

        RimMainPlotCollection*            mainPlotColl = RimProject::current()->mainPlotCollection();
        std::vector<RimEnsembleCurveSet*> ensembleCurveSets;
        mainPlotColl->descendantsOfType( ensembleCurveSets );
        for ( auto ensembleCurveSet : ensembleCurveSets )
        {
            options.push_back( caf::PdmOptionItemInfo( ensembleCurveSet->name(), ensembleCurveSet ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::connectEnsembleCurveSetFilterSignals()
{
    if ( m_ensembleCurveSet() )
    {
        m_ensembleCurveSet()->filterChanged.connect( this, &RimEnsembleSurface::onFilterSourceChanged );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::onFilterSourceChanged( const caf::SignalEmitter* emitter )
{
    if ( m_ensembleCurveSet() ) loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::initAfterRead()
{
    connectEnsembleCurveSetFilterSignals();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                           const QVariant&            oldValue,
                                           const QVariant&            newValue )
{
    if ( changedField == &m_ensembleCurveSet )
    {
        connectEnsembleCurveSetFilterSignals();
        loadDataAndUpdate();
    }
}
