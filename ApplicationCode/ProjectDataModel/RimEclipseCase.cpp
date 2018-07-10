/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipseCase.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaPreferences.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"
#include "CompletionExportCommands/RicExportCompletionDataSettingsUi.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigSimWellData.h"
#include "RigVirtualPerforationTransmissibilities.h"

#include "Rim2dIntersectionViewCollection.h"
#include "RimCaseCollection.h"
#include "RimCellEdgeColors.h"
#include "RimCommandObject.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseView.h"
#include "RimFlowCharacteristicsPlot.h"
#include "RimFlowPlotCollection.h"
#include "RimFormationNames.h"
#include "RimIntersectionCollection.h"
#include "RimRegularLegendConfig.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimPerforationCollection.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimStimPlanColors.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimTools.h"
#include "RimVirtualPerforationResults.h"
#include "RimWellAllocationPlot.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafPdmDocument.h"
#include "cafProgressInfo.h"
#include "cafPdmUiTreeOrdering.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>


CAF_PDM_XML_ABSTRACT_SOURCE_INIT(RimEclipseCase, "RimReservoir");

//------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase::RimEclipseCase() 
{
    CAF_PDM_InitFieldNoDefault(&reservoirViews, "ReservoirViews", "",  "", "", "");
    reservoirViews.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_matrixModelResults, "MatrixModelResults", "",  "", "", "");
    m_matrixModelResults.uiCapability()->setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_fractureModelResults, "FractureModelResults", "",  "", "", "");
    m_fractureModelResults.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&flipXAxis, "FlipXAxis", false, "Flip X Axis", "", "", "");
    CAF_PDM_InitField(&flipYAxis, "FlipYAxis", false, "Flip Y Axis", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_filesContainingFaultsSemColSeparated,    "CachedFileNamesContainingFaults", "", "", "", "");
    m_filesContainingFaultsSemColSeparated.uiCapability()->setUiHidden(true);

    // Obsolete fields
    CAF_PDM_InitFieldNoDefault(&m_filesContainingFaults_OBSOLETE,    "FilesContainingFaults", "", "", "", "");
    m_filesContainingFaults_OBSOLETE.xmlCapability()->setIOWritable(false);
    m_filesContainingFaults_OBSOLETE.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_caseName_OBSOLETE, "CaseName",  QString(), "Obsolete", "", "" ,"");
    m_caseName_OBSOLETE.xmlCapability()->setIOWritable(false);
    m_caseName_OBSOLETE.uiCapability()->setUiHidden(true);

    // Init

    m_matrixModelResults = new RimReservoirCellResultsStorage;
    m_matrixModelResults.uiCapability()->setUiHidden(true);
    m_matrixModelResults.uiCapability()->setUiTreeChildrenHidden(true);

    m_fractureModelResults = new RimReservoirCellResultsStorage;
    m_fractureModelResults.uiCapability()->setUiHidden(true);
    m_fractureModelResults.uiCapability()->setUiTreeChildrenHidden(true);

    this->setReservoirData( nullptr );
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase::~RimEclipseCase()
{
    reservoirViews.deleteAllChildObjects();

    delete m_matrixModelResults();
    delete m_fractureModelResults();

    RimProject* project = RiaApplication::instance()->project();
    if (project)
    {
        if (project->mainPlotCollection())
        {
            RimWellLogPlotCollection* plotCollection = project->mainPlotCollection()->wellLogPlotCollection();
            if (plotCollection)
            {
                plotCollection->removeExtractors(this->eclipseCaseData());
            }
        }
    }

    if (this->eclipseCaseData())
    {
        // At this point, we assume that memory should be released
        CVF_ASSERT(this->eclipseCaseData()->refCount() == 1);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigEclipseCaseData* RimEclipseCase::eclipseCaseData()
{
    return m_rigEclipseCase.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigEclipseCaseData* RimEclipseCase::eclipseCaseData() const
{
    return m_rigEclipseCase.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Color3f RimEclipseCase::defaultWellColor(const QString& wellName)
{
    if ( m_wellToColorMap.empty() )
    {
        const caf::ColorTable& colorTable = RiaColorTables::wellsPaletteColors();
        cvf::Color3ubArray wellColors = colorTable.color3ubArray();
        cvf::Color3ubArray interpolatedWellColors = wellColors;

        const cvf::Collection<RigSimWellData>& simWellData = this->eclipseCaseData()->wellResults();
        if ( simWellData.size() > 1 )
        {
            interpolatedWellColors = caf::ColorTable::interpolateColorArray(wellColors, simWellData.size());
        }

        for ( size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx )
        {
            m_wellToColorMap[simWellData[wIdx]->m_wellName] = cvf::Color3f::BLACK;
        }

        size_t wIdx = 0;
        for ( auto & wNameColorPair: m_wellToColorMap )
        {
            wNameColorPair.second = cvf::Color3f(interpolatedWellColors[wIdx]);

            ++wIdx;
        }
    }

    auto nmColor = m_wellToColorMap.find(wellName);
    if (nmColor != m_wellToColorMap.end()) 
    {
        return nmColor->second;
    }
    else
    {
        return cvf::Color3f::LIGHT_GRAY;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigMainGrid* RimEclipseCase::mainGrid() const
{
    if (eclipseCaseData())
    {
        return eclipseCaseData()->mainGrid();
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::initAfterRead()
{
    size_t j;
    for (j = 0; j < reservoirViews().size(); j++)
    {
        RimEclipseView* riv = reservoirViews()[j];
        CVF_ASSERT(riv);

        riv->setEclipseCase(this);
    }

    if (caseUserDescription().isEmpty() && !m_caseName_OBSOLETE().isEmpty())
    {
        caseUserDescription = m_caseName_OBSOLETE;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseCase::createAndAddReservoirView()
{
    RimEclipseView* rimEclipseView = new RimEclipseView();
    rimEclipseView->setEclipseCase(this);

    // Set default values
    {
        rimEclipseView->cellResult()->setResultType(RiaDefines::DYNAMIC_NATIVE);

        if (RiaApplication::instance()->preferences()->loadAndShowSoil)
        {
            rimEclipseView->cellResult()->setResultVariable("SOIL");
        }

        rimEclipseView->hasUserRequestedAnimation = true;

        rimEclipseView->cellEdgeResult()->setResultVariable("MULT");
        rimEclipseView->cellEdgeResult()->enableCellEdgeColors = false;
        rimEclipseView->fractureColors()->setDefaultResultName();
    }

    caf::PdmDocument::updateUiIconStateRecursively(rimEclipseView);

    size_t i = reservoirViews().size();
    rimEclipseView->name = QString("View %1").arg(i + 1);

    reservoirViews().push_back(rimEclipseView);

    return rimEclipseView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseCase::createCopyAndAddView(const RimEclipseView* sourceView)
{
    CVF_ASSERT(sourceView);

    RimEclipseView* rimEclipseView = dynamic_cast<RimEclipseView*>(sourceView->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));
    CVF_ASSERT(rimEclipseView);

    rimEclipseView->setEclipseCase(this);
   
    caf::PdmDocument::updateUiIconStateRecursively(rimEclipseView);

    reservoirViews().push_back(rimEclipseView);

    // Resolve references after reservoir view has been inserted into Rim structures
    rimEclipseView->resolveReferencesRecursively();
    rimEclipseView->initAfterReadRecursively();

    return rimEclipseView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigVirtualPerforationTransmissibilities* RimEclipseCase::computeAndGetVirtualPerforationTransmissibilities()
{
    RigEclipseCaseData* rigEclipseCase = eclipseCaseData();

    if (rigEclipseCase->virtualPerforationTransmissibilities() == nullptr)
    {
        cvf::ref<RigVirtualPerforationTransmissibilities> perfTrans = new RigVirtualPerforationTransmissibilities;

        std::vector<RimWellPath*> visibleWellPaths;
        bool anyPerforationsPresent = false;
        {
            RimProject* proj = RiaApplication::instance()->project();
            std::vector<RimWellPath*> wellPaths = proj->allWellPaths();
            for (auto w : wellPaths)
            {
                if (w->showWellPath())
                {
                    visibleWellPaths.push_back(w);

                    if (!w->perforationIntervalCollection()->perforations().empty())
                    {
                        anyPerforationsPresent = true;
                    }
                }
            }
        }

        for (auto w : visibleWellPaths)
        {
            std::vector<RigCompletionData> staticCompletionData = RicWellPathExportCompletionDataFeatureImpl::computeStaticCompletionsForWellPath(w, this);
            
            if (anyPerforationsPresent)
            {
                std::vector<std::vector<RigCompletionData>> allCompletionData;
                
                for (size_t i = 0; i < timeStepDates().size(); i++)
                {
                    std::vector<RigCompletionData> dynamicCompletionDataOneTimeStep = RicWellPathExportCompletionDataFeatureImpl::computeDynamicCompletionsForWellPath(w, this, i);

                    std::copy(staticCompletionData.begin(), staticCompletionData.end(), std::back_inserter(dynamicCompletionDataOneTimeStep));

                    allCompletionData.push_back(dynamicCompletionDataOneTimeStep);
                }
            
                perfTrans->setCompletionDataForWellPath(w, allCompletionData);
            }
            else
            {
                std::vector<std::vector<RigCompletionData>> allCompletionData;
                allCompletionData.push_back(staticCompletionData);

                perfTrans->setCompletionDataForWellPath(w, allCompletionData);
            }
        }

        for (const auto& wellRes : rigEclipseCase->wellResults())
        {
            std::vector<std::vector<RigCompletionData>> completionsPerTimeStep;
            for (size_t i = 0; i < timeStepDates().size(); i++)
            {
                std::vector<RigCompletionData> completionData;

                if (wellRes->hasWellResult(i))
                {
                    for (const auto& wellResultBranch : wellRes->wellResultFrame(i).m_wellResultBranches)
                    {
                        for (const auto& r : wellResultBranch.m_branchResultPoints)
                        {
                            if (r.isValid())
                            {
                                RigCompletionData compData(wellRes->m_wellName, RigCompletionDataGridCell(r.m_gridCellIndex, rigEclipseCase->mainGrid()), 0);
                                compData.setTransmissibility(r.connectionFactor());

                                completionData.push_back(compData);
                            }
                        }
                    }

                }

                completionsPerTimeStep.push_back(completionData);

                perfTrans->setCompletionDataForSimWell(wellRes.p(), completionsPerTimeStep);
            }
        }

        rigEclipseCase->setVirtualPerforationTransmissibilities(perfTrans.p());
    }

    return rigEclipseCase->virtualPerforationTransmissibilities();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &releaseResultMemory)
    {
        reloadDataAndUpdate();

        releaseResultMemory = oldValue.toBool();
    }
    else if (changedField == &flipXAxis || changedField == &flipYAxis)
    {
        RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
        if (rigEclipseCase)
        {
            rigEclipseCase->mainGrid()->setFlipAxis(flipXAxis, flipYAxis);

            computeCachedData();

            for (size_t i = 0; i < reservoirViews().size(); i++)
            {
                RimEclipseView* reservoirView = reservoirViews()[i];

                reservoirView->scheduleReservoirGridGeometryRegen();
                reservoirView->scheduleSimWellGeometryRegen();
                reservoirView->createDisplayModelAndRedraw();
            }
        }
    }
    else if(changedField == &activeFormationNames)
    {
        updateFormationNamesData();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::updateFormationNamesData()
{
    RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if(rigEclipseCase)
    {
        if(activeFormationNames())
        {
            rigEclipseCase->setActiveFormationNames(activeFormationNames()->formationNamesData());
        }
        else
        {
            rigEclipseCase->setActiveFormationNames(nullptr);
        }
        std::vector<Rim3dView*> views = this->views();
        for(Rim3dView* view : views)
        {
            RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(view);

            if(eclView && eclView->isUsingFormationNames())
            {
                if (!activeFormationNames())
                {
                    if (eclView->cellResult()->resultType() == RiaDefines::FORMATION_NAMES) 
                    {
                        eclView->cellResult()->setResultVariable(RiaDefines::undefinedResultName());
                        eclView->cellResult()->updateConnectedEditors();
                    }

                    RimEclipsePropertyFilterCollection* eclFilColl = eclView->eclipsePropertyFilterCollection();
                    for ( RimEclipsePropertyFilter* propFilter : eclFilColl->propertyFilters )
                    {
                        if ( propFilter->resultDefinition->resultType() == RiaDefines::FORMATION_NAMES ) 
                        {
                            propFilter->resultDefinition()->setResultVariable(RiaDefines::undefinedResultName());
                        }
                    }
                }

                RimEclipsePropertyFilterCollection* eclFilColl = eclView->eclipsePropertyFilterCollection();
                for ( RimEclipsePropertyFilter* propFilter : eclFilColl->propertyFilters )
                {
                    if ( propFilter->resultDefinition->resultType() == RiaDefines::FORMATION_NAMES )
                    {
                        propFilter->setToDefaultValues();
                        propFilter->updateConnectedEditors();
                    }
                }

                view->scheduleGeometryRegen(PROPERTY_FILTERED);
                view->scheduleCreateDisplayModelAndRedraw();
                eclView->crossSectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/)
{
    std::vector<PdmObjectHandle*> children;
    reservoirViews.childObjects(&children);

    for (auto child : children) uiTreeOrdering.add(child); 
    
    if (!m_2dIntersectionViewCollection->views().empty())
    {
        uiTreeOrdering.add(&m_2dIntersectionViewCollection);
    }

    uiTreeOrdering.skipRemainingChildren(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::computeCachedData()
{
    RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if (rigEclipseCase)
    {
        caf::ProgressInfo pInf(30, "");
        pInf.setNextProgressIncrement(1);
        rigEclipseCase->computeActiveCellBoundingBoxes();
        pInf.incrementProgress();

        pInf.setNextProgressIncrement(10);
        pInf.setProgressDescription("Calculating Cell Search Tree");
        rigEclipseCase->mainGrid()->computeCachedData();
        pInf.incrementProgress();

        pInf.setNextProgressIncrement(17);
        pInf.setProgressDescription("Calculating faults");
        rigEclipseCase->mainGrid()->calculateFaults(rigEclipseCase->activeCellInfo(RiaDefines::MATRIX_MODEL));
        pInf.incrementProgress();
        
        pInf.setProgressDescription("Calculating Formation Names Result");
        if ( activeFormationNames() )
        {
            rigEclipseCase->setActiveFormationNames(activeFormationNames()->formationNamesData());
        }
        else
        {
            rigEclipseCase->setActiveFormationNames(nullptr);
        }

        pInf.incrementProgress();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCaseCollection* RimEclipseCase::parentCaseCollection()
{
    return dynamic_cast<RimCaseCollection*>(this->parentField()->ownerObject());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::setReservoirData(RigEclipseCaseData* eclipseCase)
{
    m_rigEclipseCase  = eclipseCase;
    if (this->eclipseCaseData())
    {
        m_fractureModelResults()->setCellResults(eclipseCaseData()->results(RiaDefines::FRACTURE_MODEL));
        m_matrixModelResults()->setCellResults(eclipseCaseData()->results(RiaDefines::MATRIX_MODEL));
    }
    else
    {
        m_fractureModelResults()->setCellResults(nullptr);
        m_matrixModelResults()->setCellResults(nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::createTimeStepFormatString()
{
    std::vector<QDateTime> timeStepDates = this->timeStepDates();

    m_timeStepFormatString = RimTools::createTimeFormatStringFromDates(timeStepDates);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimEclipseCase::activeCellsBoundingBox() const
{
    if (m_rigEclipseCase.notNull() && m_rigEclipseCase->activeCellInfo(RiaDefines::MATRIX_MODEL))
    {
        return m_rigEclipseCase->activeCellInfo(RiaDefines::MATRIX_MODEL)->geometryBoundingBox();
    }
    else
    {
        return cvf::BoundingBox();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimEclipseCase::allCellsBoundingBox() const
{
    if (m_rigEclipseCase.notNull() && m_rigEclipseCase->mainGrid())
    {
        return m_rigEclipseCase->mainGrid()->boundingBox();
    }
    else
    {
        return cvf::BoundingBox();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Vec3d RimEclipseCase::displayModelOffset() const
{
    if (m_rigEclipseCase.notNull() && m_rigEclipseCase->mainGrid())
    {
        return m_rigEclipseCase->mainGrid()->displayModelOffset();
    }
    else
    {
        return cvf::Vec3d::ZERO;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseCellResultsData* RimEclipseCase::results(RiaDefines::PorosityModelType porosityModel)
{
    if (m_rigEclipseCase.notNull()) 
    {
        return m_rigEclipseCase->results(porosityModel);
    }
    
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigCaseCellResultsData* RimEclipseCase::results(RiaDefines::PorosityModelType porosityModel) const
{
    if (m_rigEclipseCase.notNull()) 
    {
        return m_rigEclipseCase->results(porosityModel);
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirCellResultsStorage* RimEclipseCase::resultsStorage(RiaDefines::PorosityModelType porosityModel)
{
    if (porosityModel == RiaDefines::MATRIX_MODEL)
    {
        return m_matrixModelResults();
    }

    return m_fractureModelResults();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RimReservoirCellResultsStorage* RimEclipseCase::resultsStorage(RiaDefines::PorosityModelType porosityModel) const
{
    if (porosityModel == RiaDefines::MATRIX_MODEL)
    {
        return m_matrixModelResults();
    }

    return m_fractureModelResults();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimEclipseCase::filesContainingFaults() const
{
    QString separatedPaths = m_filesContainingFaultsSemColSeparated;
    QStringList pathList =  separatedPaths.split(";", QString::SkipEmptyParts);
    std::vector<QString> stdPathList;

    for (auto& path: pathList) stdPathList.push_back(path);

    return stdPathList;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::setFilesContainingFaults(const std::vector<QString>& val)
{
    QString separatedPaths;
    
    for (size_t i = 0; i < val.size(); ++i)
    {
        const auto& path = val[i];
        separatedPaths += path;
        if (!(i+1 >=  val.size()) )
        {
            separatedPaths += ";";
        }
    }
    m_filesContainingFaultsSemColSeparated = separatedPaths;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseCase::openReserviorCase()
{
    if (!openEclipseGridFile())
    {
        return false;
    }

    bool createPlaceholderEntries = true;
    if (dynamic_cast<RimEclipseStatisticsCase*>(this))
    {
        // Never create placeholder entries for statistical cases. This does not make sense, and breaks the 
        // logic for testing if data is present in RimEclipseStatisticsCase::hasComputedStatistics()
        createPlaceholderEntries = false;
    }

    if (createPlaceholderEntries)
    {
        {
            RigCaseCellResultsData* results = this->results(RiaDefines::MATRIX_MODEL);
            if (results )
            {
                results->createPlaceholderResultEntries();
                // After the placeholder result for combined transmissibility is created, 
                // make sure the nnc transmissibilities can be addressed by this scalarResultIndex as well
                size_t combinedTransResIdx = results->findScalarResultIndex(RiaDefines::STATIC_NATIVE, RiaDefines::combinedTransmissibilityResultName());
                if (combinedTransResIdx != cvf::UNDEFINED_SIZE_T)
                {
                    eclipseCaseData()->mainGrid()->nncData()->setScalarResultIndex(RigNNCData::propertyNameCombTrans(), combinedTransResIdx);
                }
                size_t combinedWatFluxResIdx = results->findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::combinedWaterFluxResultName());
                if (combinedWatFluxResIdx != cvf::UNDEFINED_SIZE_T)
                {
                    eclipseCaseData()->mainGrid()->nncData()->setScalarResultIndex(RigNNCData::propertyNameFluxWat(), combinedWatFluxResIdx);
                }
                size_t combinedOilFluxResIdx = results->findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::combinedOilFluxResultName());
                if (combinedOilFluxResIdx != cvf::UNDEFINED_SIZE_T)
                {
                    eclipseCaseData()->mainGrid()->nncData()->setScalarResultIndex(RigNNCData::propertyNameFluxOil(), combinedOilFluxResIdx);
                }
                size_t combinedGasFluxResIdx = results->findScalarResultIndex(RiaDefines::DYNAMIC_NATIVE, RiaDefines::combinedGasFluxResultName());
                if (combinedGasFluxResIdx != cvf::UNDEFINED_SIZE_T)
                {
                    eclipseCaseData()->mainGrid()->nncData()->setScalarResultIndex(RigNNCData::propertyNameFluxGas(), combinedGasFluxResIdx);
                }
            }

        }

        {
            RigCaseCellResultsData* results = this->results(RiaDefines::FRACTURE_MODEL);
            if (results)
            {
                results->createPlaceholderResultEntries();
            }
        }
    }

    createTimeStepFormatString();

    // Associate existing well paths with simulation wells
    RimProject* proj = RiaApplication::instance()->project();
    for (const auto& oilField : proj->oilFields())
    {
        for (const auto& wellPath : oilField->wellPathCollection()->wellPaths())
        {
            if (!wellPath->isAssociatedWithSimulationWell())
            {
                wellPath->tryAssociateWithSimulationWell();
            }
        }
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<Rim3dView*> RimEclipseCase::allSpecialViews() const
{
    std::vector<Rim3dView*> views;
    for (size_t vIdx = 0; vIdx < reservoirViews.size(); ++vIdx)
    {
        views.push_back(reservoirViews[vIdx]);
    }
 
    return views;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RimEclipseCase::timeStepStrings() const
{
    QStringList stringList;

    int timeStepCount = static_cast<int>(results(RiaDefines::MATRIX_MODEL)->maxTimeStepCount());
    for (int i = 0; i < timeStepCount; i++)
    {
        stringList += this->timeStepName(i);
    }

    return stringList;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEclipseCase::timeStepName(int frameIdx) const
{
    std::vector<QDateTime> timeStepDates = this->timeStepDates();
    if (frameIdx < static_cast<int>(timeStepDates.size()))
    {
        QDateTime date = timeStepDates.at(frameIdx);
        return date.toString(m_timeStepFormatString);
    }
    return QString("");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::reloadDataAndUpdate()
{
    if (this->eclipseCaseData())
    {
        RigCaseCellResultsData* matrixModelResults = eclipseCaseData()->results(RiaDefines::MATRIX_MODEL);
        if (matrixModelResults)
        {
            matrixModelResults->clearAllResults();
        }

        RigCaseCellResultsData* fractureModelResults = eclipseCaseData()->results(RiaDefines::FRACTURE_MODEL);
        if (fractureModelResults)
        {
            fractureModelResults->clearAllResults();
        }

        reloadEclipseGridFile();

        for (size_t i = 0; i < reservoirViews().size(); i++)
        {
            RimEclipseView* reservoirView = reservoirViews()[i];
            CVF_ASSERT(reservoirView);
            reservoirView->loadDataAndUpdate();
            reservoirView->updateGridBoxData();
            reservoirView->updateAnnotationItems();
        }

        RimProject* project = RiaApplication::instance()->project();
        if (project)
        {
            RimSummaryCaseMainCollection* sumCaseColl = project->activeOilField() ? project->activeOilField()->summaryCaseMainCollection() : nullptr;
            if (sumCaseColl)
            {
                sumCaseColl->loadAllSummaryCaseData();
            }

            if (project->mainPlotCollection())
            {
                RimWellLogPlotCollection* wellPlotCollection = project->mainPlotCollection()->wellLogPlotCollection();
                RimSummaryPlotCollection* summaryPlotCollection = project->mainPlotCollection()->summaryPlotCollection();
                RimFlowPlotCollection* flowPlotCollection = project->mainPlotCollection()->flowPlotCollection();

                if (wellPlotCollection)
                {
                    for (size_t i = 0; i < wellPlotCollection->wellLogPlots().size(); ++i)
                    {
                        wellPlotCollection->wellLogPlots()[i]->loadDataAndUpdate();
                    }
                }
                if (summaryPlotCollection)
                {
                    for (size_t i = 0; i < summaryPlotCollection->summaryPlots().size(); ++i)
                    {
                        summaryPlotCollection->summaryPlots()[i]->loadDataAndUpdate();
                    }
                }
                if (flowPlotCollection)
                {
                    flowPlotCollection->loadDataAndUpdate();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimEclipseCase::characteristicCellSize() const
{
    const RigEclipseCaseData* rigEclipseCase = eclipseCaseData();
    if (rigEclipseCase)
    {
        return rigEclipseCase->mainGrid()->characteristicIJCellSize();
    }

    return 10.0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::setFormationNames(RimFormationNames* formationNames)
{
    activeFormationNames = formationNames;
    if (m_rigEclipseCase.notNull() && formationNames != nullptr)
    {
        m_rigEclipseCase->setActiveFormationNames(formationNames->formationNamesData());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimEclipseCase::sortedSimWellNames() const
{
    std::set<QString> sortedWellNames;
    if (eclipseCaseData())
    {
        const cvf::Collection<RigSimWellData>& simWellData = eclipseCaseData()->wellResults();

        for (size_t wIdx = 0; wIdx < simWellData.size(); ++wIdx)
        {
            sortedWellNames.insert(simWellData[wIdx]->m_wellName);
        }
    }
    return sortedWellNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimEclipseCase::timeStepDates() const
{
    if (results(RiaDefines::MATRIX_MODEL))
    {
        return results(RiaDefines::MATRIX_MODEL)->timeStepDates();
    }
    return std::vector<QDateTime>();
}
