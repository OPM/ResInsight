/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimEnsembleFileSetTools.h"

#include "Summary/RiaSummaryPlotTools.h"
#include "Summary/RiaSummaryTools.h"

#include "Ensemble/RimSummaryFileSetEnsemble.h"
#include "EnsembleFileSet/RimEnsembleFileSet.h"
#include "EnsembleFileSet/RimEnsembleFileSetCollection.h"
#include "RiaEnsembleNameTools.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"

namespace RimEnsembleFileSetTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryEnsemble*> createSummaryEnsemblesFromFileSets( const std::vector<RimEnsembleFileSet*> fileSets )
{
    std::vector<RimSummaryEnsemble*> ensembles;
    for ( auto fileSet : fileSets )
    {
        auto ensemble = new RimSummaryFileSetEnsemble();
        ensemble->setEnsembleFileSet( fileSet );
        RiaSummaryTools::summaryCaseMainCollection()->addEnsemble( ensemble );
        RiaSummaryTools::summaryCaseMainCollection()->updateEnsembleNames();
        ensemble->loadDataAndUpdate();
        ensembles.push_back( ensemble );
    }

    RiaSummaryTools::summaryCaseMainCollection()->updateAllRequiredEditors();

    return ensembles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleFileSet*> createEnsembleFileSets( const QStringList& fileNames, RiaDefines::EnsembleGroupingMode groupingMode )
{
    std::vector<RimEnsembleFileSet*> fileSets;

    auto collection = RimProject::current()->ensembleFileSetCollection();
    auto grouping   = RiaEnsembleNameTools::groupFilesByEnsembleName( fileNames, groupingMode );
    for ( const auto& [groupName, fileNames] : grouping )
    {
        auto ensembleFileSet = new RimEnsembleFileSet();
        ensembleFileSet->setName( groupName );
        ensembleFileSet->setGroupingMode( groupingMode );
        ensembleFileSet->findAndSetPathPatternAndRangeString( fileNames );

        collection->addFileSet( ensembleFileSet );

        fileSets.push_back( ensembleFileSet );
    }

    collection->updateFileSetNames();
    collection->updateAllRequiredEditors();

    return fileSets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleFileSet* createEnsembleFileSetFromOpm( const QString& pathPattern, const QString& name )
{
    auto collection      = RimProject::current()->ensembleFileSetCollection();
    auto ensembleFileSet = new RimEnsembleFileSet();
    ensembleFileSet->setAutoName( false );
    ensembleFileSet->setName( name );
    ensembleFileSet->setGroupingMode( RiaDefines::EnsembleGroupingMode::RESINSIGHT_OPMFLOW_STRUCTURE );
    ensembleFileSet->setPathPattern( pathPattern );

    collection->addFileSet( ensembleFileSet );
    collection->updateFileSetNames();
    collection->updateAllRequiredEditors();

    return ensembleFileSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> ensembleFileSetOptions()
{
    return RimProject::current()->ensembleFileSetCollection()->ensembleFileSetOptions();
}

} // namespace RimEnsembleFileSetTools
