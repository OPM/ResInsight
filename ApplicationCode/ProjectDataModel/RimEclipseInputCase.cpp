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

#include "RimEclipseInputCase.h"

#include "RiaPreferences.h"

#include "RifEclipseInputFileTools.h"
#include "RifReaderEclipseInput.h"
#include "RifReaderInterface.h"
#include "RifReaderMockModel.h"
#include "RifReaderSettings.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RiaDefines.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTools.h"

#include "cafProgressInfo.h"

#include <QFileInfo>
#include <QDir>

CAF_PDM_SOURCE_INIT(RimEclipseInputCase, "RimInputReservoir");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseInputCase::RimEclipseInputCase()
    : RimEclipseCase()
{
    CAF_PDM_InitObject("RimInputCase", ":/EclipseInput48x48.png", "", "");
    CAF_PDM_InitField(&m_gridFileName, "GridFileName",  QString(), "Case File Name", "", "" ,"");
    m_gridFileName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_inputPropertyCollection, "InputPropertyCollection", "",  "", "", "");
    m_inputPropertyCollection = new RimEclipseInputPropertyCollection;
    m_inputPropertyCollection->parentField()->uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_additionalFiles, "AdditionalFileNamesProxy", "Additional files", "", "", "");
    m_additionalFiles.registerGetMethod(this, &RimEclipseInputCase::additionalFiles);
    m_additionalFiles.uiCapability()->setUiReadOnly(true);
    m_additionalFiles.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitFieldNoDefault(&m_additionalFilenames_OBSOLETE, "AdditionalFileNames", "Additional files", "", "" ,"");
    m_additionalFilenames_OBSOLETE.uiCapability()->setUiReadOnly(true);
    m_additionalFilenames_OBSOLETE.uiCapability()->setUiHidden(true);
    m_additionalFilenames_OBSOLETE.xmlCapability()->setIOWritable(false);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseInputCase::~RimEclipseInputCase()
{
    delete m_inputPropertyCollection;
}

//--------------------------------------------------------------------------------------------------
/// Open the supplied file set. If no grid data has been read, it will first find the possible 
/// grid data among the files then read all supported properties from the files matching the grid
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCase::openDataFileSet(const QStringList& fileNames)
{
    if (fileNames.contains(RiaDefines::mockModelBasicInputCase()))
    {
        cvf::ref<RifReaderInterface> readerInterface = this->createMockModel(fileNames[0]);
        results(RiaDefines::MATRIX_MODEL)->setReaderInterface(readerInterface.p());
        results(RiaDefines::FRACTURE_MODEL)->setReaderInterface(readerInterface.p());

        eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL)->computeDerivedData();
        eclipseCaseData()->activeCellInfo(RiaDefines::FRACTURE_MODEL)->computeDerivedData();
        
        QFileInfo gridFileName(fileNames[0]);
        QString caseName = gridFileName.completeBaseName();
        this->caseUserDescription = caseName;
        
        computeCachedData();

        return;
    }

    if (this->eclipseCaseData() == NULL) 
    {
        this->setReservoirData(new RigEclipseCaseData);
    }

    // First find and read the grid data 
    if (this->eclipseCaseData()->mainGrid()->gridPointDimensions() == cvf::Vec3st(0,0,0))
    {
        RiaPreferences* prefs = RiaApplication::instance()->preferences();

        for (int i = 0; i < fileNames.size(); i++)
        {
            if (RifEclipseInputFileTools::openGridFile(fileNames[i], this->eclipseCaseData(), prefs->readerSettings()->importFaults()))
            {
                m_gridFileName = fileNames[i];

                QFileInfo gridFileName(fileNames[i]);
                QString caseName = gridFileName.completeBaseName();

                this->caseUserDescription = caseName;

                this->eclipseCaseData()->mainGrid()->setFlipAxis(flipXAxis, flipYAxis);

                computeCachedData();

                break;
            }
        }
    }

    if (this->eclipseCaseData()->mainGrid()->gridPointDimensions() == cvf::Vec3st(0,0,0))
    {
        return ; // No grid present
    }

    std::vector<QString> filesToRead;
    for (const QString& filename : fileNames)
    {
        bool exists = false;
        for (const QString& currentFileName : additionalFiles())
        {
            if (filename == currentFileName)
            {
                exists = true;
                break;
            }
        }
        if (!exists)
        {
            filesToRead.push_back(filename);
        }
    }

    for (const QString& propertyFileName : filesToRead)
    {
        std::map<QString, QString> readProperties = RifEclipseInputFileTools::readProperties(propertyFileName, this->eclipseCaseData());

        std::map<QString, QString>::iterator it;
        for (it = readProperties.begin(); it != readProperties.end(); ++it)
        {
            RimEclipseInputProperty* inputProperty = new RimEclipseInputProperty;
            inputProperty->resultName = it->first;
            inputProperty->eclipseKeyword = it->second;
            inputProperty->fileName = propertyFileName;
            inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED;
            m_inputPropertyCollection->inputProperties.push_back(inputProperty);
        }
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseInputCase::openEclipseGridFile()
{
    // Early exit if reservoir data is created
    if (this->eclipseCaseData() == NULL)
    {
        cvf::ref<RifReaderInterface> readerInterface;

        if (m_gridFileName().contains(RiaDefines::mockModelBasicInputCase()))
        {
            readerInterface = this->createMockModel(this->m_gridFileName());
        }
        else
        {
            readerInterface = new RifReaderEclipseInput;

            cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData;
            if (!readerInterface->open(m_gridFileName, eclipseCase.p()))
            {
                return false;
            }

            this->setReservoirData( eclipseCase.p() );
        }

        CVF_ASSERT(this->eclipseCaseData());
        CVF_ASSERT(readerInterface.notNull());

        results(RiaDefines::MATRIX_MODEL)->setReaderInterface(readerInterface.p());
        results(RiaDefines::FRACTURE_MODEL)->setReaderInterface(readerInterface.p());

        this->eclipseCaseData()->mainGrid()->setFlipAxis(flipXAxis, flipYAxis);
        
        computeCachedData();
        loadAndSyncronizeInputProperties();
    }

    
    RiaApplication* app = RiaApplication::instance();
    if (app->preferences()->autocomputeDepthRelatedProperties)
    {
        RimReservoirCellResultsStorage* matrixResults = results(RiaDefines::MATRIX_MODEL);
        RimReservoirCellResultsStorage* fractureResults = results(RiaDefines::FRACTURE_MODEL);

        matrixResults->computeDepthRelatedResults();
        fractureResults->computeDepthRelatedResults();
    }

    return true;
 }

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCase::reloadEclipseGridFile()
{
    setReservoirData(nullptr);
    openReserviorCase();
}

#define for_all(stdVector, indexName) for (size_t indexName = 0; indexName < stdVector.size(); ++indexName)
//--------------------------------------------------------------------------------------------------
/// Loads input property data from the gridFile and additional files
/// Creates new InputProperties if necessary, and flags the unused ones as obsolete
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCase::loadAndSyncronizeInputProperties()
{
    // Make sure we actually have reservoir data

    CVF_ASSERT(this->eclipseCaseData());
    CVF_ASSERT(this->eclipseCaseData()->mainGrid()->gridPointDimensions() != cvf::Vec3st(0,0,0));

    // Then read the properties from all the files referenced by the InputReservoir

    std::vector<QString> filenames;
    for (const RimEclipseInputProperty* inputProperty : m_inputPropertyCollection()->inputProperties())
    {
        filenames.push_back(inputProperty->fileName);
    }
    filenames.push_back(m_gridFileName);

    size_t inputPropCount = this->m_inputPropertyCollection()->inputProperties.size();

    caf::ProgressInfo progInfo(static_cast<int>(filenames.size() * inputPropCount), "Reading Input properties" );

    for_all(filenames, i)
    {
        int progress = static_cast<int>(i*inputPropCount);
        // Find all the keywords present on the file

        progInfo.setProgressDescription(filenames[i]);

        QFileInfo fileNameInfo(filenames[i]);
        bool isExistingFile = fileNameInfo.exists();

        std::set<QString> fileKeywordSet;

        if (isExistingFile)
        {
            std::vector< RifKeywordAndFilePos > fileKeywords;
            RifEclipseInputFileTools::findKeywordsOnFile(filenames[i], &fileKeywords);

            for_all(fileKeywords, fkIt) fileKeywordSet.insert(fileKeywords[fkIt].keyword);
        }

        // Find the input property objects referring to the file

        std::vector<RimEclipseInputProperty*> ipsUsingThisFile = this->m_inputPropertyCollection()->findInputProperties(filenames[i]);

        // Read property data for each inputProperty

        for_all(ipsUsingThisFile, ipIdx)
        {
            if (!isExistingFile) 
            {
                ipsUsingThisFile[ipIdx]->resolvedState = RimEclipseInputProperty::FILE_MISSING;
            }
            else
            {
                QString kw = ipsUsingThisFile[ipIdx]->eclipseKeyword();
                ipsUsingThisFile[ipIdx]->resolvedState = RimEclipseInputProperty::KEYWORD_NOT_IN_FILE;
                if (fileKeywordSet.count(kw))
                {
                    if (RifEclipseInputFileTools::readProperty(filenames[i], this->eclipseCaseData(), kw, ipsUsingThisFile[ipIdx]->resultName ))
                    {
                        ipsUsingThisFile[ipIdx]->resolvedState = RimEclipseInputProperty::RESOLVED;
                    }
                }
                fileKeywordSet.erase(kw);
            }

            progInfo.setProgress(static_cast<int>(progress + ipIdx) );
        }

        progInfo.setProgress(static_cast<int>(progress +  inputPropCount));
        // Check if there are more known property keywords left on file. If it is, read them and create inputProperty objects

        for (const QString fileKeyword : fileKeywordSet)
        {
            {
                QString resultName = this->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->makeResultNameUnique(fileKeyword);
                if (RifEclipseInputFileTools::readProperty(filenames[i], this->eclipseCaseData(), fileKeyword, resultName))
                {
                    RimEclipseInputProperty* inputProperty = new RimEclipseInputProperty;
                    inputProperty->resultName = resultName;
                    inputProperty->eclipseKeyword = fileKeyword;
                    inputProperty->fileName = filenames[i];
                    inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED;
                    m_inputPropertyCollection->inputProperties.push_back(inputProperty);
                }
            }

            progInfo.setProgress(static_cast<int>(progress + inputPropCount));
        }
    }

    for_all(m_inputPropertyCollection->inputProperties, i)
    {
        if (m_inputPropertyCollection->inputProperties[i]->resolvedState() == RimEclipseInputProperty::UNKNOWN)
        {
            m_inputPropertyCollection->inputProperties[i]->resolvedState = RimEclipseInputProperty::FILE_MISSING;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RifReaderInterface> RimEclipseInputCase::createMockModel(QString modelName)
{
    cvf::ref<RigEclipseCaseData> reservoir = new RigEclipseCaseData;
    cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;

    if (modelName == RiaDefines::mockModelBasicInputCase())
    {
        m_gridFileName = modelName;

        // Create the mock file interface and and RigSerervoir and set them up.
        mockFileInterface->setWorldCoordinates(cvf::Vec3d(10, 10, 10), cvf::Vec3d(20, 20, 20));
        mockFileInterface->setGridPointDimensions(cvf::Vec3st(4, 5, 6));
        mockFileInterface->addLocalGridRefinement(cvf::Vec3st(0, 2, 2), cvf::Vec3st(0, 2, 2), cvf::Vec3st(3, 3, 3));
        mockFileInterface->setResultInfo(3, 10);

        mockFileInterface->open("", reservoir.p());
        {
            //size_t idx = reservoir->mainGrid()->cellIndexFromIJK(1, 3, 4);
            
            //TODO: Rewrite active cell info in mock models
            //reservoir->mainGrid()->cell(idx).setActiveIndexInMatrixModel(cvf::UNDEFINED_SIZE_T);
        }

        {
            //size_t idx = reservoir->mainGrid()->cellIndexFromIJK(2, 2, 3);

            //TODO: Rewrite active cell info in mock models
            //reservoir->mainGrid()->cell(idx).setActiveIndexInMatrixModel(cvf::UNDEFINED_SIZE_T);
        }

        // Add a property
        RimEclipseInputProperty* inputProperty = new RimEclipseInputProperty;
        inputProperty->resultName = "PORO";
        inputProperty->eclipseKeyword = "PORO";
        inputProperty->fileName = "PORO.prop";
        m_inputPropertyCollection->inputProperties.push_back(inputProperty);
    }

    this->setReservoirData( reservoir.p() );

    return mockFileInterface.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCase::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&caseUserDescription);
    uiOrdering.add(&caseId);
    uiOrdering.add(&m_gridFileName);
    uiOrdering.add(&m_additionalFiles);

    auto group = uiOrdering.addNewGroup("Case Options");
    group->add(&activeFormationNames);
    group->add(&flipXAxis);
    group->add(&flipYAxis);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEclipseInputCase::locationOnDisc() const
{
    if (m_gridFileName().isEmpty()) return QString();

    QFileInfo fi(m_gridFileName);
    return fi.absolutePath();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCase::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    bool foundFile = false;
    std::vector<QString> searchedPaths;

    m_gridFileName = RimTools::relocateFile(m_gridFileName(), newProjectPath, oldProjectPath, &foundFile, &searchedPaths);

    for (RimEclipseInputProperty* inputProperty : m_inputPropertyCollection()->inputProperties())
    {
        inputProperty->fileName = RimTools::relocateFile(inputProperty->fileName, newProjectPath, oldProjectPath, &foundFile, &searchedPaths);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseInputCase::updateAdditionalFileFolder(const QString& newFolder)
{
    QDir newDir(newFolder);
    for (RimEclipseInputProperty* inputProperty : m_inputPropertyCollection()->inputProperties())
    {
        if (inputProperty->fileName == m_gridFileName) continue;
        QFileInfo oldFilePath(inputProperty->fileName);
        QFileInfo newFilePath(newDir, oldFilePath.fileName());
        inputProperty->fileName = newFilePath.absoluteFilePath();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimEclipseInputCase::additionalFiles() const
{
    std::vector<QString> additionalFiles;
    for (const RimEclipseInputProperty* inputProperty : m_inputPropertyCollection()->inputProperties())
    {
        if (inputProperty->fileName == m_gridFileName) continue;
        additionalFiles.push_back(inputProperty->fileName);
    }
    return additionalFiles;
}
