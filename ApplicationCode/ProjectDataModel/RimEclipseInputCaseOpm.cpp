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
#include "RigMainGrid.h"

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
    CAF_PDM_InitFieldNoDefault(&m_additionalFileNames, "AdditionalFileNames", "Additional files", "", "", "");
    m_additionalFileNames.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_inputPropertyCollection, "InputPropertyCollection", "", "", "", "");
    m_inputPropertyCollection = new RimEclipseInputPropertyCollection;
    m_inputPropertyCollection->parentField()->uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseInputCaseOpm::~RimEclipseInputCaseOpm()
{
    delete m_inputPropertyCollection;
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
void RimEclipseInputCaseOpm::appendPropertiesFromStandaloneFiles(const QStringList& fileNames)
{
    for (auto filename : fileNames)
    {
        QFileInfo fi(filename);
        if (!fi.exists()) continue;

        RifReaderOpmParserPropertyReader propertyReader(filename);
        std::set<std::string> fileKeywordSet = propertyReader.keywords();

        std::vector<std::string> knownKeywords = RifReaderOpmParserInput::knownPropertyKeywords();
        for (auto knownKeyword : knownKeywords)
        {
            if (fileKeywordSet.count(knownKeyword) > 0)
            {
                QString qtKnownKeyword = QString::fromStdString(knownKeyword);
                QString resultName = this->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->makeResultNameUnique(qtKnownKeyword);
                if (propertyReader.copyPropertyToCaseData(knownKeyword, this->reservoirData(), resultName))
                {
                    RimEclipseInputProperty* inputProperty = new RimEclipseInputProperty;
                    inputProperty->resultName = resultName;
                    inputProperty->eclipseKeyword = qtKnownKeyword;
                    inputProperty->fileName = filename;
                    inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED;
                    m_inputPropertyCollection->inputProperties.push_back(inputProperty);
                }
            }
        }

        m_additionalFileNames.v().push_back(filename);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseInputCaseOpm::openEclipseGridFile()
{
    importEclipseGridAndProperties(m_gridFileName);
    
    loadAndSyncronizeInputProperties();

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

    for (size_t i = 0; i < m_additionalFileNames().size(); i++)
    {
        m_additionalFileNames.v()[i] = RimTools::relocateFile(m_additionalFileNames()[i], newProjectPath, oldProjectPath, &foundFile, &searchedPaths);
    }
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCaseOpm::loadAndSyncronizeInputProperties()
{
    // Make sure we actually have reservoir data

    CVF_ASSERT(this->reservoirData());
    CVF_ASSERT(this->reservoirData()->mainGrid()->gridPointDimensions() != cvf::Vec3st(0, 0, 0));

    // Then read the properties from all the files referenced by the InputReservoir

    for (QString filename : m_additionalFileNames())
    {
        QFileInfo fileNameInfo(filename);
        bool isExistingFile = fileNameInfo.exists();

        // Find the input property objects referring to the file

        std::vector<RimEclipseInputProperty*> ipsUsingThisFile = this->m_inputPropertyCollection()->findInputProperties(filename);

        if (!isExistingFile)
        {
            for (auto inputProperty : ipsUsingThisFile)
            {
                inputProperty->resolvedState = RimEclipseInputProperty::FILE_MISSING;
            }
        }
        else
        {
            RifReaderOpmParserPropertyReader propertyReader(filename);
            std::set<std::string> fileKeywordSet = propertyReader.keywords();

            for (auto inputProperty : ipsUsingThisFile)
            {
                QString kw = inputProperty->eclipseKeyword();
                inputProperty->resolvedState = RimEclipseInputProperty::KEYWORD_NOT_IN_FILE;
                if (fileKeywordSet.count(kw.toStdString()))
                {
                    if (propertyReader.copyPropertyToCaseData(kw.toStdString(), this->reservoirData(), inputProperty->resultName))
                    {
                        inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED;
                    }
                }
                fileKeywordSet.erase(kw.toStdString());
            }

            if (!fileKeywordSet.empty())
            {
                std::vector<std::string> knownKeywords = RifReaderOpmParserInput::knownPropertyKeywords();
                for (auto knownKeyword : knownKeywords)
                {
                    if (fileKeywordSet.count(knownKeyword) > 0)
                    {
                        QString qtKnownKeyword = QString::fromStdString(knownKeyword);
                        QString resultName = this->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->makeResultNameUnique(qtKnownKeyword);
                        if (propertyReader.copyPropertyToCaseData(knownKeyword, this->reservoirData(), resultName))
                        {
                            RimEclipseInputProperty* inputProperty = new RimEclipseInputProperty;
                            inputProperty->resultName = resultName;
                            inputProperty->eclipseKeyword = qtKnownKeyword;
                            inputProperty->fileName = filename;
                            inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED;
                            m_inputPropertyCollection->inputProperties.push_back(inputProperty);
                        }
                    }
                }
            }
        }
    }

    for(auto inputProperty : m_inputPropertyCollection->inputProperties())
    {
        if (inputProperty->resolvedState() == RimEclipseInputProperty::UNKNOWN)
        {
            inputProperty->resolvedState = RimEclipseInputProperty::FILE_MISSING;
        }
    }
}

