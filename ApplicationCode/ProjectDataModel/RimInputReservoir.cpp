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

#include "RIStdInclude.h"

#include "RimInputReservoir.h"
#include "RimInputProperty.h"
#include "RimReservoirView.h"

#include "RifReaderEclipseInput.h"

#include "RigEclipseCase.h"
#include "RigReservoirCellResults.h"

#include "cvfAssert.h"

#include <QString>
#include "RifReaderMockModel.h"
#include "RifEclipseInputFileTools.h"
#include "cafProgressInfo.h"

#include "RIApplication.h"
#include "RIPreferences.h"


CAF_PDM_SOURCE_INIT(RimInputReservoir, "RimInputReservoir");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimInputReservoir::RimInputReservoir()
    : RimReservoir()
{
    CAF_PDM_InitObject("RimInputReservoir", ":/EclipseInput48x48.png", "", "");
    CAF_PDM_InitField(&m_gridFileName, "GridFileName",  QString(), "Case grid filename", "", "" ,"");
    CAF_PDM_InitFieldNoDefault(&m_additionalFileNames, "AdditionalFileNames", "Additional files", "", "" ,"");


    CAF_PDM_InitFieldNoDefault(&m_inputPropertyCollection, "InputPropertyCollection", "",  "", "", "");
    m_inputPropertyCollection = new RimInputPropertyCollection;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimInputReservoir::~RimInputReservoir()
{
    delete m_inputPropertyCollection;
}

//--------------------------------------------------------------------------------------------------
/// Open the supplied file set. If no grid data has been read, it will first find the possible 
/// grid data among the files then read all supported properties from the files matching the grid
//--------------------------------------------------------------------------------------------------
void RimInputReservoir::openDataFileSet(const QStringList& filenames)
{
    if (caseName().contains("Input Mock Debug Model"))
    {
        cvf::ref<RifReaderInterface> readerInterface = this->createMockModel(this->caseName());
        m_rigEclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->setReaderInterface(readerInterface.p());
        m_rigEclipseCase->results(RifReaderInterface::FRACTURE_RESULTS)->setReaderInterface(readerInterface.p());

        m_rigEclipseCase->activeCellInfo(RifReaderInterface::MATRIX_RESULTS)->computeDerivedData();
        m_rigEclipseCase->activeCellInfo(RifReaderInterface::FRACTURE_RESULTS)->computeDerivedData();

        return;
    }

    if (m_rigEclipseCase.isNull()) 
    {
        m_rigEclipseCase = new RigEclipseCase;
    }

    // First find and read the grid data 
    if (this->reservoirData()->mainGrid()->gridPointDimensions() == cvf::Vec3st(0,0,0))
    {
         for (int i = 0; i < filenames.size(); i++)
         {
             if (RifEclipseInputFileTools::openGridFile(filenames[i], this->reservoirData()))
             {
                 m_gridFileName = filenames[i];

                 computeCachedData();

                 break;
             }
         }
    }

    if (this->reservoirData()->mainGrid()->gridPointDimensions() == cvf::Vec3st(0,0,0))
    {
        return ; // No grid present
    }

    // Then read the properties possibly in the grid file
    QStringList filesToRead;
    for (int i = 0; i < filenames.size(); i++)
    {
        size_t j;
        bool exist = false;
        for (j = 0; j < m_additionalFileNames().size(); j++)
        {
            if (m_additionalFileNames()[j] == filenames[i])
            {
                exist = true;
            }
        }

        if (!exist)
        {
            filesToRead.push_back(filenames[i]);
        }
    }

    for (int i = 0; i < filesToRead.size(); i++)
    {
        QString propertyFileName = filesToRead[i];
        std::map<QString, QString> readProperties = RifEclipseInputFileTools::readProperties(propertyFileName, this->reservoirData());

        std::map<QString, QString>::iterator it;
        for (it = readProperties.begin(); it != readProperties.end(); ++it)
        {
            RimInputProperty* inputProperty = new RimInputProperty;
            inputProperty->resultName = it->first;
            inputProperty->eclipseKeyword = it->second;
            inputProperty->fileName = propertyFileName;
            inputProperty->resolvedState = RimInputProperty::RESOLVED;
            m_inputPropertyCollection->inputProperties.push_back(inputProperty);
        }

        if (propertyFileName != m_gridFileName)
        {
            m_additionalFileNames.v().push_back(propertyFileName);
        }
    }
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimInputReservoir::openEclipseGridFile()
{
    // Early exit if reservoir data is created
    if (m_rigEclipseCase.isNull())
    {
        cvf::ref<RifReaderInterface> readerInterface;

        if (caseName().contains("Input Mock Debug Model"))
        {
            readerInterface = this->createMockModel(this->caseName());
        }
        else
        {
            cvf::ref<RigEclipseCase> eclipseCase = new RigEclipseCase;
            readerInterface = new RifReaderEclipseInput;
            if (!readerInterface->open(m_gridFileName, eclipseCase.p()))
            {
                return false;
            }

            m_rigEclipseCase = eclipseCase;
        }

        CVF_ASSERT(m_rigEclipseCase.notNull());
        CVF_ASSERT(readerInterface.notNull());

        m_rigEclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->setReaderInterface(readerInterface.p());
        m_rigEclipseCase->results(RifReaderInterface::FRACTURE_RESULTS)->setReaderInterface(readerInterface.p());
        
        computeCachedData();
        loadAndSyncronizeInputProperties();
    }

    
    RIApplication* app = RIApplication::instance();
    if (app->preferences()->autocomputeDepthRelatedProperties)
    {
        RigReservoirCellResults* matrixResults = m_rigEclipseCase->results(RifReaderInterface::MATRIX_RESULTS);
        RigReservoirCellResults* fractureResults = m_rigEclipseCase->results(RifReaderInterface::FRACTURE_RESULTS);

        matrixResults->computeDepthRelatedResults();
        fractureResults->computeDepthRelatedResults();
    }

    return true;
 }

#define for_all(stdVector, indexName) for (size_t indexName = 0; indexName < stdVector.size(); ++indexName)
//--------------------------------------------------------------------------------------------------
/// Loads input property data from the gridFile and additional files
/// Creates new InputProperties if necessary, and flags the unused ones as obsolete
//--------------------------------------------------------------------------------------------------
void RimInputReservoir::loadAndSyncronizeInputProperties()
{
    // Make sure we actually have reservoir data

    CVF_ASSERT(m_rigEclipseCase.notNull());
    CVF_ASSERT(this->reservoirData()->mainGrid()->gridPointDimensions() != cvf::Vec3st(0,0,0));

    // Then read the properties from all the files referenced by the InputReservoir

    std::vector<QString> filenames = m_additionalFileNames;
    filenames.push_back(m_gridFileName);

    const std::vector<QString>& knownKeywords = RifEclipseInputFileTools::knownPropertyKeywords();

    size_t inputPropCount = this->m_inputPropertyCollection()->inputProperties.size();

    caf::ProgressInfo progInfo(static_cast<int>(filenames.size() *( inputPropCount + knownKeywords.size())), "Reading Input properties" );
    int progress = 0;

    for_all(filenames, i)
    {
        progress = static_cast<int>(i*( inputPropCount + knownKeywords.size()));
        // Find all the keywords present on the file

        progInfo.setProgressDescription(filenames[i]);

        QFileInfo fileNameInfo(filenames[i]);
        bool isExistingFile = fileNameInfo.exists();

        std::set<QString> fileKeywordSet;

        if (isExistingFile)
        {
            std::vector< RifKeywordAndFilePos > fileKeywords = RifEclipseInputFileTools::findKeywordsOnFile(filenames[i]);
            for_all(fileKeywords, fkIt) fileKeywordSet.insert(fileKeywords[fkIt].keyword);
        }

        // Find the input property objects referring to the file

        std::vector<RimInputProperty*> ipsUsingThisFile = this->m_inputPropertyCollection()->findInputProperties(filenames[i]);

        // Read property data for each inputProperty

        for_all(ipsUsingThisFile, ipIdx)
        {
            if (!isExistingFile) 
            {
                ipsUsingThisFile[ipIdx]->resolvedState = RimInputProperty::FILE_MISSING;
            }
            else
            {
                QString kw = ipsUsingThisFile[ipIdx]->eclipseKeyword();
                ipsUsingThisFile[ipIdx]->resolvedState = RimInputProperty::KEYWORD_NOT_IN_FILE;
                if (fileKeywordSet.count(kw))
                {
                    if (RifEclipseInputFileTools::readProperty(filenames[i], this->reservoirData(), kw,  ipsUsingThisFile[ipIdx]->resultName ))
                    {
                        ipsUsingThisFile[ipIdx]->resolvedState = RimInputProperty::RESOLVED;
                    }
                }
                fileKeywordSet.erase(kw);
            }

            progInfo.setProgress(static_cast<int>(progress + ipIdx) );
        }

        progInfo.setProgress(static_cast<int>(progress +  inputPropCount));
        // Check if there are more known property keywords left on file. If it is, read them and create inputProperty objects

        if (!fileKeywordSet.empty())
        {
           
           
            std::vector<QString> knownKwsLeft;
            for_all(knownKeywords, fkIt) 
            {
                if (fileKeywordSet.count(knownKeywords[fkIt]))
                {
                    QString resultName = this->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->makeResultNameUnique(knownKeywords[fkIt]);
                    if (RifEclipseInputFileTools::readProperty(filenames[i], this->reservoirData(), knownKeywords[fkIt], resultName))
                    {
                        RimInputProperty* inputProperty = new RimInputProperty;
                        inputProperty->resultName = resultName;
                        inputProperty->eclipseKeyword = knownKeywords[fkIt];
                        inputProperty->fileName = filenames[i];
                        inputProperty->resolvedState = RimInputProperty::RESOLVED;
                        m_inputPropertyCollection->inputProperties.push_back(inputProperty);
                    }
                }
                 progInfo.setProgress(static_cast<int>(progress +  inputPropCount + fkIt));
            }
        }
    }

    for_all(m_inputPropertyCollection->inputProperties, i)
    {
        if (m_inputPropertyCollection->inputProperties[i]->resolvedState() == RimInputProperty::UNKNOWN)
        {
            m_inputPropertyCollection->inputProperties[i]->resolvedState = RimInputProperty::FILE_MISSING;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimInputReservoir::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimInputReservoir::addFiles(const QStringList& newFileNames)
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimInputReservoir::removeFiles(const QStringList& obsoleteFileNames)
{

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimInputReservoir::removeProperty(RimInputProperty* inputProperty)
{
    bool isPropertyFileReferencedByOthers = false;

    m_inputPropertyCollection->removeInputProperty(inputProperty, isPropertyFileReferencedByOthers);
    if (!isPropertyFileReferencedByOthers)
    {
        std::vector<QString> newList;
        size_t i;
        for (i = 0; i < m_additionalFileNames().size(); i++)
        {
            if (m_additionalFileNames()[i] != inputProperty->fileName)
            {
               newList.push_back(m_additionalFileNames()[i]);
            }
        }

        m_additionalFileNames.v() = newList;
    }

    // Remove the results pointed to by this input property
    RigReservoirCellResults* results = m_rigEclipseCase->results(RifReaderInterface::MATRIX_RESULTS);
    results->removeResult(inputProperty->resultName);

    this->removeResult(inputProperty->resultName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RifReaderInterface> RimInputReservoir::createMockModel(QString modelName)
{
    cvf::ref<RigEclipseCase> reservoir = new RigEclipseCase;
    cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;

    if (modelName == "Input Mock Debug Model Simple")
    {
        // Create the mock file interface and and RigSerervoir and set them up.
        mockFileInterface->setWorldCoordinates(cvf::Vec3d(10, 10, 10), cvf::Vec3d(20, 20, 20));
        mockFileInterface->setGridPointDimensions(cvf::Vec3st(4, 5, 6));
        mockFileInterface->addLocalGridRefinement(cvf::Vec3st(0, 2, 2), cvf::Vec3st(0, 2, 2), cvf::Vec3st(3, 3, 3));
        mockFileInterface->setResultInfo(3, 10);

        mockFileInterface->open("", reservoir.p());
        {
            size_t idx = reservoir->mainGrid()->cellIndexFromIJK(1, 3, 4);
            
            //TODO: Rewrite active cell info in mock models
            //reservoir->mainGrid()->cell(idx).setActiveIndexInMatrixModel(cvf::UNDEFINED_SIZE_T);
        }

        {
            size_t idx = reservoir->mainGrid()->cellIndexFromIJK(2, 2, 3);

            //TODO: Rewrite active cell info in mock models
            //reservoir->mainGrid()->cell(idx).setActiveIndexInMatrixModel(cvf::UNDEFINED_SIZE_T);
        }

        // Add a property
        RimInputProperty* inputProperty = new RimInputProperty;
        inputProperty->resultName = "PORO";
        inputProperty->eclipseKeyword = "PORO";
        inputProperty->fileName = "PORO.prop";
        m_inputPropertyCollection->inputProperties.push_back(inputProperty);
    }

    m_rigEclipseCase = reservoir;

    return mockFileInterface.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimInputReservoir::locationOnDisc() const
{
    if (m_gridFileName().isEmpty()) return QString();

    QFileInfo fi(m_gridFileName);
    return fi.absolutePath();
}
