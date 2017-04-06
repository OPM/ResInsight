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

#include "RimEclipseResultCase.h"

#include "RiaPreferences.h"

#include "RifEclipseOutputFileTools.h"
#include "RifReaderEclipseOutput.h"
#include "RifReaderMockModel.h"
#include "RifReaderSettings.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimMockModelSettings.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTools.h"
#include "RimFlowDiagSolution.h"
#include "RigFlowDiagSolverInterface.h"

#include "cafPdmSettings.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

CAF_PDM_SOURCE_INIT(RimEclipseResultCase, "EclipseCase");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase::RimEclipseResultCase()
    : RimEclipseCase()
{
    CAF_PDM_InitObject("Eclipse Case", ":/Case48x48.png", "", "");

    CAF_PDM_InitField(&caseFileName, "CaseFileName",  QString(), "Case file name", "", "" ,"");
    caseFileName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault (&m_flowDiagSolutions, "FlowDiagSolutions", "Flow Diagnostics Solutions", "", "", "");
    m_flowDiagSolutions.uiCapability()->setUiHidden(true);
    m_flowDiagSolutions.uiCapability()->setUiTreeHidden(true);
    m_flowDiagSolutions.uiCapability()->setUiTreeChildrenHidden(true);

    // Obsolete, unused field
    CAF_PDM_InitField(&caseDirectory, "CaseFolder", QString(), "Directory", "", "" ,"");
    caseDirectory.xmlCapability()->setIOWritable(false); 
    caseDirectory.uiCapability()->setUiHidden(true);

    flipXAxis.xmlCapability()->setIOWritable(true);
    //flipXAxis.uiCapability()->setUiHidden(true);
    flipYAxis.xmlCapability()->setIOWritable(true);
    //flipYAxis.uiCapability()->setUiHidden(true);



    m_activeCellInfoIsReadFromFile = false;
    m_gridAndWellDataIsReadFromFile = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultCase::openEclipseGridFile()
{
    caf::ProgressInfo progInfo(50, "Reading Eclipse Grid File");

    progInfo.setProgressDescription("Open Grid File");
    progInfo.setNextProgressIncrement(48);
    
    // Early exit if data is already read
    if (m_gridAndWellDataIsReadFromFile) return true;

    cvf::ref<RifReaderInterface> readerInterface;

    if (caseFileName().contains("Result Mock Debug Model"))
    {
        readerInterface = this->createMockModel(this->caseFileName());
    }
    else
    {
        if (!QFile::exists(caseFileName()))
        {
            return false;
        }

        RiaPreferences* prefs = RiaApplication::instance()->preferences();
        readerInterface = new RifReaderEclipseOutput;
        readerInterface->setReaderSetting(prefs->readerSettings());
        readerInterface->setFilenamesWithFaults(this->filesContainingFaults());

        cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData;
        if (!readerInterface->open(caseFileName(), eclipseCase.p()))
        {
            return false;
        }

        this->filesContainingFaults = readerInterface->filenamesWithFaults();

        this->setReservoirData( eclipseCase.p() );
    }

    results(RifReaderInterface::MATRIX_RESULTS)->setReaderInterface(readerInterface.p());
    results(RifReaderInterface::FRACTURE_RESULTS)->setReaderInterface(readerInterface.p());

    progInfo.incrementProgress();

    CVF_ASSERT(this->eclipseCaseData());
    CVF_ASSERT(readerInterface.notNull());

    progInfo.setProgressDescription("Computing Case Cache");
    computeCachedData();

    m_gridAndWellDataIsReadFromFile = true;
    m_activeCellInfoIsReadFromFile = true;

    if (eclipseCaseData()->results(RifReaderInterface::MATRIX_RESULTS)->hasFlowDiagUsableFluxes())
    {
        m_flowDagSolverInterface = new RigFlowDiagSolverInterface(this);
        
        if (m_flowDiagSolutions.size() == 0)
        {
            m_flowDiagSolutions.push_back(new RimFlowDiagSolution());
        }
    }
    
    return true;
 }

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultCase::openAndReadActiveCellData(RigEclipseCaseData* mainEclipseCase)
{
    // Early exit if data is already read
    if (m_activeCellInfoIsReadFromFile) return true;

    cvf::ref<RifReaderInterface> readerInterface;
    if (caseFileName().contains("Result Mock Debug Model"))
    {
        readerInterface = this->createMockModel(this->caseFileName());
    }
    else
    {
        if (!QFile::exists(caseFileName()))
        {
            return false;
        }

        cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData;

        CVF_ASSERT(mainEclipseCase && mainEclipseCase->mainGrid());
        eclipseCase->setMainGrid(mainEclipseCase->mainGrid());

        std::vector<QDateTime> timeStepDates = mainEclipseCase->results(RifReaderInterface::MATRIX_RESULTS)->timeStepDates();
        if (timeStepDates.size() == 0)
        {
            return false;
        }

        cvf::ref<RifReaderEclipseOutput> readerEclipseOutput = new RifReaderEclipseOutput;
        if (!readerEclipseOutput->openAndReadActiveCellData(caseFileName(), timeStepDates, eclipseCase.p()))
        {
            return false;
        }

        readerEclipseOutput->close();

        this->setReservoirData( eclipseCase.p() );

        readerInterface = readerEclipseOutput;
    }

    results(RifReaderInterface::MATRIX_RESULTS)->setReaderInterface(readerInterface.p());
    results(RifReaderInterface::FRACTURE_RESULTS)->setReaderInterface(readerInterface.p());

    CVF_ASSERT(this->eclipseCaseData());
    CVF_ASSERT(readerInterface.notNull());

    eclipseCaseData()->computeActiveCellBoundingBoxes();

    m_activeCellInfoIsReadFromFile = true;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RifReaderInterface> RimEclipseResultCase::createMockModel(QString modelName)
{
    cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;
    cvf::ref<RigEclipseCaseData> reservoir = new RigEclipseCaseData;

     if (modelName == RimDefines::mockModelBasic())
    {
        // Create the mock file interface and and RigSerervoir and set them up.
        mockFileInterface->setWorldCoordinates(cvf::Vec3d(10, 10, 10), cvf::Vec3d(20, 20, 20));
        mockFileInterface->setGridPointDimensions(cvf::Vec3st(4, 5, 6));
        mockFileInterface->addLocalGridRefinement(cvf::Vec3st(0, 2, 2), cvf::Vec3st(0, 2, 2), cvf::Vec3st(3, 3, 3));
        mockFileInterface->enableWellData(false);

        mockFileInterface->open("", reservoir.p());
    }
    else if (modelName == RimDefines::mockModelBasicWithResults())
    {
        mockFileInterface->setWorldCoordinates(cvf::Vec3d(10, 10, 10), cvf::Vec3d(-20, -20, -20));
        mockFileInterface->setGridPointDimensions(cvf::Vec3st(5, 10, 20));
        mockFileInterface->addLocalGridRefinement(cvf::Vec3st(0, 3, 3), cvf::Vec3st(1, 4, 9), cvf::Vec3st(2, 2, 2));
        mockFileInterface->setResultInfo(3, 10);

        mockFileInterface->open("", reservoir.p());

        // Make a fault
        cvf::Vec3d& tmp = reservoir->mainGrid()->nodes()[1];
        tmp += cvf::Vec3d(1, 0, 0);
    }
    else if (modelName == RimDefines::mockModelLargeWithResults())
    {
        double startX = 0;
        double startY = 0;
        double startZ = 0;

        double widthX = 6000;
        double widthY = 12000;
        double widthZ = 500;

        double offsetX = 0;
        double offsetY = 0;
        double offsetZ = 0;

        // Test code to simulate UTM coordinates
        offsetX = 400000;
        offsetY = 6000000;
        offsetZ = 0;


        mockFileInterface->setWorldCoordinates(cvf::Vec3d(startX + offsetX, startY + offsetY, startZ + offsetZ), cvf::Vec3d(startX + widthX + offsetX, startY + widthY + offsetY, startZ + widthZ + offsetZ));
        mockFileInterface->setGridPointDimensions(cvf::Vec3st(50, 100, 200));
        mockFileInterface->addLocalGridRefinement(cvf::Vec3st(0, 30, 30), cvf::Vec3st(1, 40, 90), cvf::Vec3st(2, 2, 2));
        mockFileInterface->setResultInfo(3, 10);

        mockFileInterface->open("", reservoir.p());

    }
    else if (modelName == RimDefines::mockModelCustomized())
    {
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));

        RimMockModelSettings rimMockModelSettings;
        caf::PdmSettings::readFieldsFromApplicationStore(&rimMockModelSettings);

        caf::PdmUiPropertyViewDialog propertyDialog(NULL, &rimMockModelSettings, "Customize Mock Model", "");
        if (propertyDialog.exec() == QDialog::Accepted)
        {
            QApplication::restoreOverrideCursor();

            caf::PdmSettings::writeFieldsToApplicationStore(&rimMockModelSettings);

            double startX = 0;
            double startY = 0;
            double startZ = 0;

            double widthX = 6000;
            double widthY = 12000;
            double widthZ = 500;

            // Test code to simulate UTM coordinates
            double offsetX = 400000;
            double offsetY = 6000000;
            double offsetZ = 0;

            mockFileInterface->setWorldCoordinates(cvf::Vec3d(startX + offsetX, startY + offsetY, startZ + offsetZ), cvf::Vec3d(startX + widthX + offsetX, startY + widthY + offsetY, startZ + widthZ + offsetZ));
            mockFileInterface->setGridPointDimensions(cvf::Vec3st(rimMockModelSettings.cellCountX + 1, rimMockModelSettings.cellCountY + 1, rimMockModelSettings.cellCountZ + 1));
            mockFileInterface->setResultInfo(rimMockModelSettings.resultCount, rimMockModelSettings.timeStepCount);
            mockFileInterface->enableWellData(false);

            mockFileInterface->open("", reservoir.p());
        }
        else
        {
             QApplication::restoreOverrideCursor();
        }
    }

    this->setReservoirData( reservoir.p() );

    return mockFileInterface.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase::~RimEclipseResultCase()
{
    reservoirViews.deleteAllChildObjects();
    m_flowDiagSolutions.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimEclipseResultCase::locationOnDisc() const
{
    QFileInfo fi(caseFileName());
    return fi.absolutePath();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::readGridDimensions(std::vector< std::vector<int> >& gridDimensions)
{
    RifEclipseOutputFileTools::readGridDimensions(caseFileName(), gridDimensions);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    bool foundFile = false;
    std::vector<QString> searchedPaths;

    // Update filename and folder paths when opening project from a different file location
    caseFileName = RimTools::relocateFile(caseFileName(), newProjectPath, oldProjectPath, &foundFile, &searchedPaths);
    
#if 0 // Output the search path for debugging
    for (size_t i = 0; i < searchedPaths.size(); ++i)
       qDebug() << searchedPaths[i];
#endif 
    
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFlowDiagSolution* RimEclipseResultCase::defaultFlowDiagSolution()
{
    if (m_flowDiagSolutions.size() > 0)
    {
        return m_flowDiagSolutions[0];
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimFlowDiagSolution*> RimEclipseResultCase::flowDiagSolutions()
{
    std::vector<RimFlowDiagSolution*> flowSols; 
    for ( const caf::PdmPointer<RimFlowDiagSolution>& fsol: m_flowDiagSolutions ) 
    { 
        flowSols.push_back(fsol.p());
    }

    return flowSols;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagSolverInterface* RimEclipseResultCase::flowDiagSolverInterface()
{
    return m_flowDagSolverInterface.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::setGridFileName(const QString& caseFileName)
{
    this->caseFileName = caseFileName;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::setCaseInfo(const QString& userDescription, const QString& caseFileName)
{
    this->caseUserDescription = userDescription;
    this->caseFileName = caseFileName;

    RimProject* proj = RiaApplication::instance()->project();
    proj->assignCaseIdToCase(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::initAfterRead()
{
    RimEclipseCase::initAfterRead();

    // Convert from old (9.0.2) way of storing the case file
    if (caseFileName().isEmpty())
    {
        if (!this->caseName().isEmpty() && !caseDirectory().isEmpty())
        {
            caseFileName = QDir::fromNativeSeparators(caseDirectory()) + "/" + caseName() + ".EGRID";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&caseUserDescription);
    uiOrdering.add(&caseId);
    uiOrdering.add(&caseFileName);
}

