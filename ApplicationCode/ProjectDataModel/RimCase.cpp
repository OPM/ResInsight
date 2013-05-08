/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

//#include "RiaStdInclude.h"
#include "RimCase.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "RifReaderEclipseOutput.h"
#include "RifReaderMockModel.h"

#include "RimReservoirView.h"
#include "cafPdmFieldCvfMat4d.h"
#include "cafPdmFieldCvfColor.h"
#include "RimCellRangeFilter.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilter.h"
#include "RimCellPropertyFilterCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimWellCollection.h"

#include "RimScriptCollection.h"

#include "RigCaseData.h"
#include "RigMainGrid.h"
#include "RigCaseCellResultsData.h"

#include "cvfAssert.h"

#include "cafPdmFieldCvfColor.h"

#include "cafPdmUiPushButtonEditor.h"

#include <QString>
#include "RimProject.h"
#include "RimReservoirCellResultsCacher.h"
#include "RimResultSlot.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCaseCollection.h"

CAF_PDM_SOURCE_INIT(RimCase, "RimReservoir");

//------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase::RimCase()
{
    CAF_PDM_InitField(&caseUserDescription, "CaseUserDescription",  QString(), "Case name", "", "" ,"");
    
    CAF_PDM_InitField(&caseId, "CaseId", -1, "Case ID", "", "" ,"");
    caseId.setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&reservoirViews, "ReservoirViews", "",  "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_matrixModelResults, "MatrixModelResults", "",  "", "", "");
    m_matrixModelResults.setUiHidden(true);
    CAF_PDM_InitFieldNoDefault(&m_fractureModelResults, "FractureModelResults", "",  "", "", "");
    m_fractureModelResults.setUiHidden(true);

    CAF_PDM_InitField(&flipXAxis, "FlipXAxis", false, "Flip X Axis", "", "", "");
    CAF_PDM_InitField(&flipYAxis, "FlipYAxis", false, "Flip Y Axis", "", "", "");


    // Obsolete field
    CAF_PDM_InitField(&caseName, "CaseName",  QString(), "Obsolete", "", "" ,"");
    caseName.setIOWritable(false);
    caseName.setUiHidden(true);

    m_matrixModelResults = new RimReservoirCellResultsStorage;
    m_fractureModelResults = new RimReservoirCellResultsStorage;

    this->setReservoirData( NULL );
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase::~RimCase()
{
    reservoirViews.deleteAllChildObjects();

    delete m_matrixModelResults();
    delete m_fractureModelResults();

    if (this->reservoirData())
    {
        // At this point, we assume that memory should be released
        CVF_ASSERT(this->reservoirData()->refCount() == 1);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigCaseData* RimCase::reservoirData()
{
    return m_rigEclipseCase.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCaseData* RimCase::reservoirData() const
{
    return m_rigEclipseCase.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCase::initAfterRead()
{
    size_t j;
    for (j = 0; j < reservoirViews().size(); j++)
    {
        RimReservoirView* riv = reservoirViews()[j];
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
RimReservoirView* RimCase::createAndAddReservoirView()
{
    RimReservoirView* riv = new RimReservoirView();
    riv->setEclipseCase(this);

    size_t i = reservoirViews().size();
    riv->name = QString("View %1").arg(i + 1);

    reservoirViews().push_back(riv);

    return riv;
}

//--------------------------------------------------------------------------------------------------
/// TODO: Move this functionality to PdmPointersField
//--------------------------------------------------------------------------------------------------
void RimCase::removeReservoirView(RimReservoirView* reservoirView)
{
    std::vector<size_t> indices;

    size_t i;
    for (i = 0; i < reservoirViews().size(); i++)
    {
        if (reservoirViews()[i] == reservoirView)
        {
            indices.push_back(i);
        }
    }

    // NB! Make sure the ordering goes from large to low index
    while (!indices.empty())
    {
        reservoirViews().erase(indices.back());
        indices.pop_back();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCase::removeResult(const QString& resultName)
{
    size_t i;
    for (i = 0; i < reservoirViews().size(); i++)
    {
        RimReservoirView* reservoirView = reservoirViews()[i];
        CVF_ASSERT(reservoirView);

        RimResultSlot* result = reservoirView->cellResult;
        CVF_ASSERT(result);

        bool rebuildDisplayModel = false;

        // Set cell result variable to none if displaying 
        if (result->resultVariable() == resultName)
        {
            result->setResultVariable(RimDefines::undefinedResultName());
            result->loadResult();

            rebuildDisplayModel = true;
        }

        std::list< caf::PdmPointer< RimCellPropertyFilter > >::iterator it;
        RimCellPropertyFilterCollection* propFilterCollection = reservoirView->propertyFilterCollection();
        for (it = propFilterCollection->propertyFilters.v().begin(); it != propFilterCollection->propertyFilters.v().end(); ++it)
        {
            RimCellPropertyFilter* propertyFilter = *it;
            if (propertyFilter->resultDefinition->resultVariable() == resultName)
            {
                propertyFilter->resultDefinition->setResultVariable(RimDefines::undefinedResultName());
                propertyFilter->resultDefinition->loadResult();
                propertyFilter->setDefaultValues();

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
void RimCase::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &releaseResultMemory)
    {
        if (this->reservoirData())
        {
            for (size_t i = 0; i < reservoirViews().size(); i++)
            {
                RimReservoirView* reservoirView = reservoirViews()[i];
                CVF_ASSERT(reservoirView);

                RimResultSlot* result = reservoirView->cellResult;
                CVF_ASSERT(result);

                result->setResultVariable(RimDefines::undefinedResultName());
                result->loadResult();

                RimCellEdgeResultSlot* cellEdgeResult = reservoirView->cellEdgeResult;
                CVF_ASSERT(cellEdgeResult);

                cellEdgeResult->resultVariable.v() = RimDefines::undefinedResultName();
                cellEdgeResult->loadResult();

                reservoirView->createDisplayModelAndRedraw();
            }

            RigCaseCellResultsData* matrixModelResults = reservoirData()->results(RifReaderInterface::MATRIX_RESULTS);
            if (matrixModelResults)
            {
                matrixModelResults->clearAllResults();
            }

            RigCaseCellResultsData* fractureModelResults = reservoirData()->results(RifReaderInterface::FRACTURE_RESULTS);
            if (fractureModelResults)
            {
                fractureModelResults->clearAllResults();
            }
        }

        releaseResultMemory = oldValue.toBool();
    }
    else if (changedField == &flipXAxis || changedField == &flipYAxis)
    {
        RigCaseData* rigEclipseCase = reservoirData();
        if (rigEclipseCase)
        {
            rigEclipseCase->mainGrid()->setFlipAxis(flipXAxis, flipYAxis);

            computeCachedData();

            for (size_t i = 0; i < reservoirViews().size(); i++)
            {
                RimReservoirView* reservoirView = reservoirViews()[i];

                reservoirView->scheduleReservoirGridGeometryRegen();
                reservoirView->schedulePipeGeometryRegen();
                reservoirView->createDisplayModelAndRedraw();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCase::computeCachedData()
{
    RigCaseData* rigEclipseCase = reservoirData();
    if (rigEclipseCase)
    {
        rigEclipseCase->computeActiveCellBoundingBoxes();

        rigEclipseCase->mainGrid()->computeCachedData();

        std::vector<RigGridBase*> grids;
        rigEclipseCase->allGrids(&grids);

        size_t i;
        for (i = 0; i < grids.size(); i++)
        {
            grids[i]->computeFaults();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCaseCollection* RimCase::parentCaseCollection()
{
    std::vector<RimCaseCollection*> parentObjects;
    this->parentObjectsOfType(parentObjects);

    if (parentObjects.size() > 0)
    {
        return parentObjects[0];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup* RimCase::parentGridCaseGroup()
{
    RimCaseCollection* caseColl = parentCaseCollection();
    if (caseColl) 
    {
        return caseColl->parentCaseGroup();
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCase::setReservoirData(RigCaseData* eclipseCase)
{
    m_rigEclipseCase  = eclipseCase;
    if (this->reservoirData())
    {
        m_fractureModelResults()->setCellResults(reservoirData()->results(RifReaderInterface::FRACTURE_RESULTS));
        m_matrixModelResults()->setCellResults(reservoirData()->results(RifReaderInterface::MATRIX_RESULTS));
        m_fractureModelResults()->setMainGrid(this->reservoirData()->mainGrid());
        m_matrixModelResults()->setMainGrid(this->reservoirData()->mainGrid());
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
RimReservoirCellResultsStorage* RimCase::results(RifReaderInterface::PorosityModelResultType porosityModel)
{
    if (porosityModel == RifReaderInterface::MATRIX_RESULTS)
    {
        return m_matrixModelResults();
    }

    return m_fractureModelResults();
}


//--------------------------------------------------------------------------------------------------
///  Relocate the supplied file name, based on the search path as follows:
///  fileName, newProjectPath/fileNameWoPath, relocatedPath/fileNameWoPath
///  If the file is not found in any of the positions, the fileName is returned unchanged
///
///  The relocatedPath is found in this way:
///  use the start of newProjectPath
///  plus the end of the path to m_gridFileName
///  such that the common start of oldProjectPath and m_gridFileName is removed from m_gridFileName
///  and replaced with the start of newProjectPath up to where newProjectPath starts to be equal to oldProjectPath
//--------------------------------------------------------------------------------------------------
QString RimCase::relocateFile(const QString& fileName,  const QString& newProjectPath, const QString& oldProjectPath, 
                              bool* foundFile, std::vector<QString>* searchedPaths)
{
    if (foundFile) *foundFile = true;

    if (searchedPaths) searchedPaths->push_back(fileName);
    if (QFile::exists(fileName))
    {
        return fileName;
    }

    // First check in the new project file directory
    {
        QString fileNameWithoutPath = QFileInfo(fileName).fileName();
        QString candidate = QDir::fromNativeSeparators(newProjectPath + QDir::separator() + fileNameWithoutPath);
        if (searchedPaths) searchedPaths->push_back(candidate);

        if (QFile::exists(candidate))
        {
            return candidate;
        }
    }    

    // Then find the possible move of a directory structure where projects and files referenced are moved in "paralell"

    QFileInfo gridFileInfo(QDir::fromNativeSeparators(fileName));
    QString gridFilePath = gridFileInfo.path();
    QString gridFileNameWoPath = gridFileInfo.fileName();
    QStringList gridPathElements = gridFilePath.split("/", QString::KeepEmptyParts);

    QString oldProjPath  = QDir::fromNativeSeparators(oldProjectPath);
    QStringList oldProjPathElements = oldProjPath.split("/", QString::KeepEmptyParts);

    QString newProjPath  = QDir::fromNativeSeparators(newProjectPath);
    QStringList newProjPathElements = newProjPath.split("/", QString::KeepEmptyParts);

    // Find the possible equal start of the old project path, and the referenced file

    bool pathStartsAreEqual = false;
    bool pathEndsDiffer = false;
    int firstDiffIdx = 0;
    for ( firstDiffIdx = 0; firstDiffIdx < gridPathElements.size() && firstDiffIdx < oldProjPathElements.size(); ++firstDiffIdx)
    {
        if (gridPathElements[firstDiffIdx] == oldProjPathElements[firstDiffIdx])
        {
            pathStartsAreEqual = pathStartsAreEqual || !gridPathElements[firstDiffIdx].isEmpty();
        }
        else
        {
            pathEndsDiffer = true;
            break;
        }
    }

    if (!pathEndsDiffer && firstDiffIdx < gridPathElements.size() || firstDiffIdx < oldProjPathElements.size())
    {
        pathEndsDiffer = true;
    }

    // If the path starts are equal, try to substitute it in the referenced file, with the corresponding new project path start

    if (pathStartsAreEqual)
    {
        if (pathEndsDiffer)
        {
            QString oldGridFilePathEnd;
            for (int i = firstDiffIdx; i < gridPathElements.size(); ++i)
            {
                oldGridFilePathEnd += gridPathElements[i];
                oldGridFilePathEnd += "/";
            }

            // Find the new Project File Start Path

            QStringList oldProjectFilePathEndElements;
            for (int i = firstDiffIdx; i < oldProjPathElements.size(); ++i)
            {
                oldProjectFilePathEndElements.push_back(oldProjPathElements[i]);
            }

            int ppIdx = oldProjectFilePathEndElements.size() -1;
            int lastDiffIdx = newProjPathElements.size() -1;

            for (; lastDiffIdx >= 0 && ppIdx >= 0; --lastDiffIdx, --ppIdx)
            {
                if (oldProjectFilePathEndElements[ppIdx] != newProjPathElements[lastDiffIdx])
                {
                    break;   
                }
            }

            QString newProjecetFileStartPath;
            for (int i = 0; i <= lastDiffIdx; ++i)
            {
                newProjecetFileStartPath += newProjPathElements[i];
                newProjecetFileStartPath += "/";
            }

            QString relocationPath = newProjecetFileStartPath + oldGridFilePathEnd;

            QString relocatedFileName = relocationPath + gridFileNameWoPath;

            if (searchedPaths) searchedPaths->push_back(relocatedFileName);

            if (QFile::exists(relocatedFileName))
            {
                return relocatedFileName;
            }
        }
        else
        {
            // The Grid file was located in the same dir as the Project file. This is supposed to be handled above.
            // So we did not find it
        }
    }

    // return the unchanged filename, if we could not find a valid relocation file
    if (foundFile) *foundFile = false;

    return fileName;
}
