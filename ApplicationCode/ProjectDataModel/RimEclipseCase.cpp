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
#include "RiaPreferences.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimCaseCollection.h"
#include "RimCellEdgeColors.h"
#include "RimCommandObject.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilter.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimFormationNames.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimProject.h"
#include "RimMainPlotCollection.h"
#include "RimWellLogPlotCollection.h"

#include "cafPdmDocument.h"
#include "cafProgressInfo.h"

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

    CAF_PDM_InitFieldNoDefault(&filesContainingFaults,    "FilesContainingFaults", "", "", "", "");
    filesContainingFaults.uiCapability()->setUiHidden(true);

    // Obsolete field
    CAF_PDM_InitField(&caseName, "CaseName",  QString(), "Obsolete", "", "" ,"");
    caseName.xmlCapability()->setIOWritable(false);
    caseName.uiCapability()->setUiHidden(true);

    m_matrixModelResults = new RimReservoirCellResultsStorage;
    m_matrixModelResults.uiCapability()->setUiHidden(true);
    m_matrixModelResults.uiCapability()->setUiTreeChildrenHidden(true);

    m_fractureModelResults = new RimReservoirCellResultsStorage;
    m_fractureModelResults.uiCapability()->setUiHidden(true);
    m_fractureModelResults.uiCapability()->setUiTreeChildrenHidden(true);

    this->setReservoirData( NULL );
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
void RimEclipseCase::initAfterRead()
{
    size_t j;
    for (j = 0; j < reservoirViews().size(); j++)
    {
        RimEclipseView* riv = reservoirViews()[j];
        CVF_ASSERT(riv);

        riv->setEclipseCase(this);
    }

    if (caseUserDescription().isEmpty() && !caseName().isEmpty())
    {
        caseUserDescription = caseName;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RimEclipseCase::createAndAddReservoirView()
{
    RimEclipseView* rimEclipseView = new RimEclipseView();
    rimEclipseView->setEclipseCase(this);
    rimEclipseView->cellEdgeResult()->setResultVariable("MULT");
    rimEclipseView->cellEdgeResult()->enableCellEdgeColors = false;

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
void RimEclipseCase::removeResult(const QString& resultName)
{
    size_t i;
    for (i = 0; i < reservoirViews().size(); i++)
    {
        RimEclipseView* reservoirView = reservoirViews()[i];
        CVF_ASSERT(reservoirView);

        RimEclipseCellColors* result = reservoirView->cellResult;
        CVF_ASSERT(result);

        bool rebuildDisplayModel = false;

        // Set cell result variable to none if displaying 
        if (result->resultVariable() == resultName)
        {
            result->setResultVariable(RimDefines::undefinedResultName());
            result->loadResult();

            rebuildDisplayModel = true;
        }

        RimEclipsePropertyFilterCollection* propFilterCollection = reservoirView->eclipsePropertyFilterCollection();
        for (size_t filter = 0; filter < propFilterCollection->propertyFilters().size(); filter++)
        {
            RimEclipsePropertyFilter* propertyFilter = propFilterCollection->propertyFilters()[filter];
            if (propertyFilter->resultDefinition->resultVariable() == resultName)
            {
                propertyFilter->resultDefinition->setResultVariable(RimDefines::undefinedResultName());
                propertyFilter->resultDefinition->loadResult();
                propertyFilter->setToDefaultValues();

                rebuildDisplayModel = true;
            }
        }

        if (rebuildDisplayModel)
        {
            reservoirViews()[i]->createDisplayModelAndRedraw();
        }


        // TODO
        // CellEdgeResults are not considered, as they do not support display of input properties yet
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseCase::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &releaseResultMemory)
    {
        if (this->eclipseCaseData())
        {
            for (size_t i = 0; i < reservoirViews().size(); i++)
            {
                RimEclipseView* reservoirView = reservoirViews()[i];
                CVF_ASSERT(reservoirView);

                RimEclipseCellColors* result = reservoirView->cellResult;
                CVF_ASSERT(result);

                result->setResultVariable(RimDefines::undefinedResultName());
                result->loadResult();

                RimCellEdgeColors* cellEdgeResult = reservoirView->cellEdgeResult;
                CVF_ASSERT(cellEdgeResult);

                cellEdgeResult->setResultVariable(RimDefines::undefinedResultName());
                cellEdgeResult->loadResult();

                reservoirView->createDisplayModelAndRedraw();
            }

            RigCaseCellResultsData* matrixModelResults = eclipseCaseData()->results(RifReaderInterface::MATRIX_RESULTS);
            if (matrixModelResults)
            {
                matrixModelResults->clearAllResults();
            }

            RigCaseCellResultsData* fractureModelResults = eclipseCaseData()->results(RifReaderInterface::FRACTURE_RESULTS);
            if (fractureModelResults)
            {
                fractureModelResults->clearAllResults();
            }
        }

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
        std::vector<RimView*> views = this->views();
        for(RimView* view : views)
        {
            RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(view);

            if(eclView && eclView->isUsingFormationNames())
            {
                if (!activeFormationNames())
                {
                    if (eclView->cellResult()->resultType() == RimDefines::FORMATION_NAMES) 
                    {
                        eclView->cellResult()->setResultVariable(RimDefines::undefinedResultName());
                        eclView->cellResult()->updateConnectedEditors();
                    }

                    RimEclipsePropertyFilterCollection* eclFilColl = eclView->eclipsePropertyFilterCollection();
                    for ( RimEclipsePropertyFilter* propFilter : eclFilColl->propertyFilters )
                    {
                        if ( propFilter->resultDefinition->resultType() == RimDefines::FORMATION_NAMES ) 
                        {
                            propFilter->resultDefinition()->setResultVariable(RimDefines::undefinedResultName());
                        }
                    }
                }

                RimEclipsePropertyFilterCollection* eclFilColl = eclView->eclipsePropertyFilterCollection();
                for ( RimEclipsePropertyFilter* propFilter : eclFilColl->propertyFilters )
                {
                    if ( propFilter->resultDefinition->resultType() == RimDefines::FORMATION_NAMES )
                    {
                        propFilter->setToDefaultValues();
                        propFilter->updateConnectedEditors();
                    }
                }

                view->scheduleGeometryRegen(PROPERTY_FILTERED);
                view->scheduleCreateDisplayModelAndRedraw();
            }
        }
    }
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
        rigEclipseCase->mainGrid()->calculateFaults(rigEclipseCase->activeCellInfo(RifReaderInterface::MATRIX_RESULTS));
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
        m_fractureModelResults()->setCellResults(eclipseCaseData()->results(RifReaderInterface::FRACTURE_RESULTS));
        m_matrixModelResults()->setCellResults(eclipseCaseData()->results(RifReaderInterface::MATRIX_RESULTS));
        m_fractureModelResults()->setMainGrid(this->eclipseCaseData()->mainGrid());
        m_matrixModelResults()->setMainGrid(this->eclipseCaseData()->mainGrid());
    }
    else
    {
        m_fractureModelResults()->setCellResults(NULL);
        m_matrixModelResults()->setCellResults(NULL);
        m_fractureModelResults()->setMainGrid(NULL);
        m_matrixModelResults()->setMainGrid(NULL);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RimEclipseCase::activeCellsBoundingBox() const
{
    if (m_rigEclipseCase.notNull() && m_rigEclipseCase->activeCellInfo(RifReaderInterface::MATRIX_RESULTS))
    {
        return m_rigEclipseCase->activeCellInfo(RifReaderInterface::MATRIX_RESULTS)->geometryBoundingBox();
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
RimReservoirCellResultsStorage* RimEclipseCase::results(RifReaderInterface::PorosityModelResultType porosityModel)
{
    if (porosityModel == RifReaderInterface::MATRIX_RESULTS)
    {
        return m_matrixModelResults();
    }

    return m_fractureModelResults();
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseCase::openReserviorCase()
{
    // If read already, return

    if (this->eclipseCaseData() != NULL) return true;
    
    if (!openEclipseGridFile())
    {
        return false;
    }

    {
        RimReservoirCellResultsStorage* results = this->results(RifReaderInterface::MATRIX_RESULTS);
        if (results->cellResults())
        {
            results->cellResults()->createPlaceholderResultEntries();
            // After the placeholder result for combined transmissibility is created, 
            // make sure the nnc transmissibilities can be addressed by this scalarResultIndex as well
            size_t combinedTransResIdx = results->cellResults()->findScalarResultIndex(RimDefines::STATIC_NATIVE, RimDefines::combinedTransmissibilityResultName());
            if (combinedTransResIdx != cvf::UNDEFINED_SIZE_T)
            {
                eclipseCaseData()->mainGrid()->nncData()->setCombTransmisibilityScalarResultIndex(combinedTransResIdx);
            }
        }

    }
    {
        RimReservoirCellResultsStorage* results = this->results(RifReaderInterface::FRACTURE_RESULTS);
        if (results->cellResults()) results->cellResults()->createPlaceholderResultEntries();
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimView*> RimEclipseCase::views()
{
    std::vector<RimView*> views;
    for (size_t vIdx = 0; vIdx < reservoirViews.size(); ++vIdx)
    {
        views.push_back(reservoirViews[vIdx]);
    }
    return views;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList RimEclipseCase::timeStepStrings()
{
    QStringList stringList;

    int timeStepCount = static_cast<int>(results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->maxTimeStepCount());
    for (int i = 0; i < timeStepCount; i++)
    {
        stringList += this->timeStepName(i);
    }

    return stringList;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEclipseCase::timeStepName(int frameIdx)
{
    std::vector<QDateTime> timeStepDates = this->timeStepDates();
    CVF_ASSERT(frameIdx < static_cast<int>(timeStepDates.size()));

    if (m_timeStepFormatString.isEmpty())
    {
        bool hasHoursAndMinutesInTimesteps = false;
        for (size_t i = 0; i < timeStepDates.size(); i++)
        {
            if (timeStepDates[i].time().hour() != 0.0 || timeStepDates[i].time().minute() != 0.0)
            {
                hasHoursAndMinutesInTimesteps = true;
                break;
            }
        }

        m_timeStepFormatString = "dd.MMM yyyy";
        if (hasHoursAndMinutesInTimesteps)
        {
            m_timeStepFormatString += " - hh:mm";
        }
    }

    QDateTime date = timeStepDates.at(frameIdx);

    return date.toString(m_timeStepFormatString);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RimEclipseCase::timeStepDates()
{
    return results(RifReaderInterface::MATRIX_RESULTS)->cellResults()->timeStepDates();
}
