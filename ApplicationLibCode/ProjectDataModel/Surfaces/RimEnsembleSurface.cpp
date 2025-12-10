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
#include "RiaOptionItemFactory.h"

#include "Surface/RigSurfaceResampler.h"
#include "Surface/RigSurfaceStatisticsCalculator.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleStatisticsSurface.h"
#include "RimFileSurface.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSurface.h"
#include "RimSurfaceCollection.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimEnsembleSurface, "EnsembleSurface" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleSurface::RimEnsembleSurface()
{
    CAF_PDM_InitScriptableObject( "Ensemble Surface", ":/ReservoirSurfaces16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSet, "FilterEnsembleCurveSet", "Filter by Ensemble Curve Set" );

    std::vector<RigSurfaceStatisticsCalculator::StatisticsType> statisticsTypes = { RigSurfaceStatisticsCalculator::StatisticsType::MIN,
                                                                                    RigSurfaceStatisticsCalculator::StatisticsType::MAX,
                                                                                    RigSurfaceStatisticsCalculator::StatisticsType::MEAN,
                                                                                    RigSurfaceStatisticsCalculator::StatisticsType::P10,
                                                                                    RigSurfaceStatisticsCalculator::StatisticsType::P50,
                                                                                    RigSurfaceStatisticsCalculator::StatisticsType::P90 };

    for ( auto s : statisticsTypes )
    {
        auto statSurface = new RimEnsembleStatisticsSurface;
        statSurface->setStatisticsType( s );
        addSurface( statSurface );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::addFileSurface( RimFileSurface* fileSurface )
{
    addSurface( fileSurface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::addSurface( RimSurface* surface )
{
    if ( !sourceFileSurfaceCollection() )
    {
        auto coll = new RimSurfaceCollection;
        coll->setCollectionName( ensembleSourceFileCollectionName() );
        addSubCollection( coll );
    }

    sourceFileSurfaceCollection()->addSurface( surface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurface*> RimEnsembleSurface::sourceFileSurfaces() const
{
    std::vector<RimSurface*> fileSurfs;
    for ( auto& w : sourceFileSurfaceCollection()->surfaces() )
    {
        if ( w->isFileBased() )
        {
            fileSurfs.push_back( w );
        }
    }

    return fileSurfs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::loadDataAndUpdate()
{
    {
        auto fileSurfaces = sourceFileSurfaces();
        auto surfaceCount = static_cast<int>( fileSurfaces.size() );
#pragma omp parallel for
        for ( int i = 0; i < surfaceCount; i++ )
        {
            auto surf = fileSurfaces[i];
            surf->onLoadData();
        }
    }

    std::vector<RimSurface*> sourceSurfaceForStatistics = sourceFileSurfaces();
    if ( m_ensembleCurveSet != nullptr )
    {
        sourceSurfaceForStatistics = filterByEnsembleCurveSet( sourceSurfaceForStatistics );
    }

    if ( !sourceSurfaceForStatistics.empty() )
    {
        for ( auto& surf : sourceSurfaceForStatistics )
        {
            // The search tree must be created before the multi threading loop is initiated to avoid crash in
            // RigSurfaceResampler::resampleSurface
            // NB! Do not use OpenMP on this loop, as the construction of the AABB tree is using OpenMP internally, and
            // mixing these causes crash.

            surf->surfaceData()->ensureIntersectionSearchTreeIsBuilt();
        }

        cvf::ref<RigSurface>              firstSurface = sourceSurfaceForStatistics[0]->surfaceData();
        auto                              surfaceCount = static_cast<int>( sourceSurfaceForStatistics.size() );
        std::vector<cvf::ref<RigSurface>> sourceSurfaces( surfaceCount );
#pragma omp parallel for
        for ( int i = 0; i < surfaceCount; i++ )
        {
            auto surf             = sourceSurfaceForStatistics[i];
            auto resampledSurface = RigSurfaceResampler::resampleSurface( firstSurface, surf->surfaceData() );
            sourceSurfaces[i]     = resampledSurface;
        }

        m_statisticsSurface = RigSurfaceStatisticsCalculator::computeStatistics( sourceSurfaces );
        if ( !m_statisticsSurface.isNull() )
        {
            for ( auto statSurface : surfaces() )
            {
                statSurface->onLoadData();
            }
        }
    }

    auto surfColl = firstAncestorOrThisOfTypeAsserted<RimSurfaceCollection>();

    std::vector<RimSurface*> surfacesToUpdate;
    surfColl->updateViews( surfaces(), false );
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSurface*> RimEnsembleSurface::filterByEnsembleCurveSet( const std::vector<RimSurface*>& fileSurfaces ) const
{
    std::vector<RimSurface*> filteredCases;

    if ( m_ensembleCurveSet != nullptr )
    {
        // Get the summary cases from the related ensemble summary curve set.
        RimSummaryEnsemble* summaryEnsemble = m_ensembleCurveSet->summaryEnsemble();

        //
        std::vector<RimSummaryCase*> sumCases = m_ensembleCurveSet->filterEnsembleCases( summaryEnsemble->allSummaryCases() );
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
bool RimEnsembleSurface::isSameRealization( RimSummaryCase* summaryCase, RimSurface* fileSurface ) const
{
    // TODO: duplication with RimEnsembleWellLogCurveSet::isSameRealization
    QString fileSurfaceName = fileSurface->filePath();
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
QList<caf::PdmOptionItemInfo> RimEnsembleSurface::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_ensembleCurveSet )
    {
        RiaOptionItemFactory::appendOptionItemsForEnsembleCurveSets( &options );
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
RimSurfaceCollection* RimEnsembleSurface::sourceFileSurfaceCollection() const

{
    auto name = ensembleSourceFileCollectionName();

    return getSubCollection( name );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimEnsembleSurface::ensembleSourceFileCollectionName()
{
    auto name = caf::AppEnum<RimSurface::SurfaceType>::uiText( RimSurface::SurfaceType::ENSEMBLE_SOURCE );

    return name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::loadData()
{
    loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSurface* RimEnsembleSurface::findStatisticsSurface( RigSurfaceStatisticsCalculator::StatisticsType statisticsType )
{
    for ( auto s : surfaces() )
    {
        auto ensembleSurface = dynamic_cast<RimEnsembleStatisticsSurface*>( s );

        if ( ensembleSurface && ensembleSurface->statisticsType() == statisticsType ) return s;
    }

    return nullptr;
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
void RimEnsembleSurface::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_ensembleCurveSet )
    {
        connectEnsembleCurveSetFilterSignals();
        loadDataAndUpdate();
    }
}
