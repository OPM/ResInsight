/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RimEclipseInputCaseOpm.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RifReaderOpmParserInput.h"
#include "RifReaderSettings.h"

#include "RigCaseCellResultsData.h"
#include "RigCaseData.h"

#include "RimEclipseCase.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTools.h"

#include "cafProgressInfo.h"

#include <QFileInfo>


CAF_PDM_SOURCE_INIT(RimEclipseInputCaseOpm, "EclipseInputCaseOpm");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseInputCaseOpm::RimEclipseInputCaseOpm()
    : RimEclipseCase()
{
    CAF_PDM_InitObject("RimInputCase", ":/EclipseInput48x48.png", "", "");

    CAF_PDM_InitField(&m_gridFileName, "GridFileName",  QString(), "Case grid filename", "", "" ,"");
    m_gridFileName.uiCapability()->setUiReadOnly(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseInputCaseOpm::~RimEclipseInputCaseOpm()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCaseOpm::importNewEclipseGridAndProperties(const QString& fileName)
{
    m_gridFileName = fileName;

    QFileInfo gridFileName(m_gridFileName);
    QString caseName = gridFileName.completeBaseName();
    this->caseUserDescription = caseName + " (opm-parser)";

    importEclipseGridAndProperties(m_gridFileName);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseInputCaseOpm::openEclipseGridFile()
{
    importEclipseGridAndProperties(m_gridFileName);
    
    return true;
 }

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEclipseInputCaseOpm::locationOnDisc() const
{
    if (m_gridFileName().isEmpty()) return QString();

    QFileInfo fi(m_gridFileName);
    return fi.absolutePath();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCaseOpm::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    bool foundFile = false;
    std::vector<QString> searchedPaths;

    m_gridFileName = RimTools::relocateFile(m_gridFileName(), newProjectPath, oldProjectPath, &foundFile, &searchedPaths);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCaseOpm::importEclipseGridAndProperties(const QString& fileName)
{
    if (this->reservoirData() == NULL)
    {
        this->setReservoirData(new RigCaseData);

        RifReaderOpmParserInput::importGridPropertiesFaults(fileName, reservoirData());

        if (this->reservoirData()->mainGrid() == NULL)
        {
            return;
        }

        this->reservoirData()->mainGrid()->setFlipAxis(flipXAxis, flipYAxis);

        computeCachedData();

        RiaApplication* app = RiaApplication::instance();
        if (app->preferences()->autocomputeDepthRelatedProperties)
        {
            RimReservoirCellResultsStorage* matrixResults = results(RifReaderInterface::MATRIX_RESULTS);
            RimReservoirCellResultsStorage* fractureResults = results(RifReaderInterface::FRACTURE_RESULTS);

            matrixResults->computeDepthRelatedResults();
            fractureResults->computeDepthRelatedResults();
        }
    }
}

