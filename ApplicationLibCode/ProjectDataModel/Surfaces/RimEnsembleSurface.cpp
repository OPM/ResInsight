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

#include "RigSurfaceResampler.h"
#include "RigSurfaceStatisticsCalculator.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleStatisticsSurface.h"
#include "RimFileSurface.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
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
    CAF_PDM_InitScriptableObject( "Ensemble Surface", ":/ReservoirSurfaces16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_ensembleCurveSet, "FilterEnsembleCurveSet", "Filter by Ensemble Curve Set", "", "", "" );

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
        addSurface( statSurface );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEnsembleSurface::addFileSurface( RimFileSurface* fileSurface )
{
    if ( !sourceFileSurfaceCollection() )
    {
        auto coll = new RimSurfaceCollection;
        coll->setCollectionName( ensembleSourceFileCollectionName() );
        addSubCollection( coll );
    }

    sourceFileSurfaceCollection()->addSurface( fileSurface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimFileSurface*> RimEnsembleSurface::sourceFileSurfaces() const
{
    std::vector<RimFileSurface*> fileSurfs;
    for ( auto& w : sourceFileSurfaceCollection()->surfaces() )
    {
        if ( auto fsurf = dynamic_cast<RimFileSurface*>( w ) )
        {
            fileSurfs.push_back( fsurf );
        }
        else
        {
            RiaLogging::warning( QString( "Detected unknown surface type in File Surface collection" ) );
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

    std::vector<RimFileSurface*> sourceSurfaceForStatistics = sourceFileSurfaces();
    if ( m_ensembleCurveSet != nullptr )
    {
        sourceSurfaceForStatistics = filterByEnsembleCurveSet( sourceSurfaceForStatistics );
    }

    if ( !sourceSurfaceForStatistics.empty() )
    {
        cvf::ref<RigSurface> firstSurface = sourceSurfaceForStatistics[0]->surfaceData();

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
        if ( auto ensambleSurface = dynamic_cast<RimEnsembleStatisticsSurface*>( s ) )
        {
            if ( ensambleSurface->statisticsType() == statisticsType ) return s;
        }
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
