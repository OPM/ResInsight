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

#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "RifEclipseOutputFileTools.h"
#include "RifReaderEclipseOutput.h"
#include "RifReaderEclipseRft.h"
#include "RifReaderMockModel.h"
#include "RifReaderSettings.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigFlowDiagSolverInterface.h"
#include "RigMainGrid.h"

#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFlowDiagSolution.h"
#include "RimMockModelSettings.h"
#include "RimProject.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimTimeStepFilter.h"
#include "RimTools.h"

#include "cafPdmSettings.h"
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafUtils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <string>
#include <fstream>



CAF_PDM_SOURCE_INIT(RimEclipseResultCase, "EclipseCase");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseResultCase::RimEclipseResultCase()
    : RimEclipseCase()
{
    CAF_PDM_InitObject("Eclipse Case", ":/Case48x48.png", "", "");

    CAF_PDM_InitField(&caseFileName, "CaseFileName",  QString(), "Case File Name", "", "" ,"");
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

    CAF_PDM_InitField(&m_sourSimFileName, "SourSimFileName", QString(), "SourSim File Name", "", "", "");
    m_sourSimFileName.uiCapability()->setUiEditorTypeName(caf::PdmUiFilePathEditor::uiEditorTypeName());
#ifndef USE_HDF5
    m_sourSimFileName.uiCapability()->setUiHidden(true);
#endif

    m_activeCellInfoIsReadFromFile = false;
    m_gridAndWellDataIsReadFromFile = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultCase::openEclipseGridFile()
{
    return importGridAndResultMetaData(false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultCase::importGridAndResultMetaData(bool showTimeStepFilter)
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
        if (!caf::Utils::fileExists(caseFileName()))
        {
            return false;
        }

        cvf::ref<RifReaderEclipseOutput> readerEclipseOutput = new RifReaderEclipseOutput;
        readerEclipseOutput->setFilenamesWithFaults(this->filesContainingFaults());

        if (showTimeStepFilter)
        {
            cvf::ref<RifEclipseRestartDataAccess> restartDataAccess = RifEclipseOutputFileTools::createDynamicResultAccess(caseFileName());
            if (restartDataAccess.isNull())
            {
                return false;
            }

            {
                std::vector<QDateTime> timeSteps;
                std::vector<double> daysSinceSimulationStart;

                restartDataAccess->timeSteps(&timeSteps, &daysSinceSimulationStart);

                // Restore cursor as the progress dialog is active
                QApplication::restoreOverrideCursor();

                // Show GUI to select time steps 

                m_timeStepFilter->setTimeStepsFromFile(timeSteps);

                caf::PdmUiPropertyViewDialog propertyDialog(nullptr, m_timeStepFilter, "Time Step Filter", "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                propertyDialog.resize(QSize(400, 400));

                if (propertyDialog.exec() != QDialog::Accepted)
                {
                    return false;
                }
                m_timeStepFilter->updateFilteredTimeStepsFromUi();
                // Set cursor in wait state to continue display of progress dialog including
                // wait cursor
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            }
            readerEclipseOutput->setFileDataAccess(restartDataAccess.p());
        }
        readerEclipseOutput->setTimeStepFilter(m_timeStepFilter->filteredTimeSteps());
        
        cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData(this);
        if (!readerEclipseOutput->open(caseFileName(), eclipseCase.p()))
        {
            return false;
        }

        this->setFilesContainingFaults(readerEclipseOutput->filenamesWithFaults());

        this->setReservoirData(eclipseCase.p());

         readerInterface = readerEclipseOutput; 
    }

    results(RiaDefines::MATRIX_MODEL)->setReaderInterface(readerInterface.p());
    results(RiaDefines::FRACTURE_MODEL)->setReaderInterface(readerInterface.p());

    progInfo.incrementProgress();

    CVF_ASSERT(this->eclipseCaseData());
    CVF_ASSERT(readerInterface.notNull());

    progInfo.setProgressDescription("Computing Case Cache");
    computeCachedData();

    m_gridAndWellDataIsReadFromFile = true;
    m_activeCellInfoIsReadFromFile = true;

    m_flowDagSolverInterface = new RigFlowDiagSolverInterface(this);

    QFileInfo eclipseCaseFileInfo(caseFileName());
    QString rftFileName = eclipseCaseFileInfo.path() + "/" + eclipseCaseFileInfo.completeBaseName() + ".RFT";
    QFileInfo rftFileInfo(rftFileName);

    if (rftFileInfo.exists())
    {
        RiaLogging::info(QString("RFT file found"));
        m_readerEclipseRft = new RifReaderEclipseRft(rftFileInfo.filePath());
    }


    if (m_flowDiagSolutions.size() == 0)
    {
        m_flowDiagSolutions.push_back(new RimFlowDiagSolution());
    }

    if (!m_sourSimFileName().isEmpty())
    {
        RifReaderEclipseOutput* outReader = dynamic_cast<RifReaderEclipseOutput*>(readerInterface.p());
        outReader->setHdf5FileName(m_sourSimFileName());
    }
    
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::reloadEclipseGridFile()
{
    m_gridAndWellDataIsReadFromFile = false;
    m_activeCellInfoIsReadFromFile = false;
    setReservoirData(nullptr);
    openReserviorCase();
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
        if (!caf::Utils::fileExists(caseFileName()))
        {
            return false;
        }

        cvf::ref<RigEclipseCaseData> eclipseCase = new RigEclipseCaseData(this);

        CVF_ASSERT(mainEclipseCase && mainEclipseCase->mainGrid());
        eclipseCase->setMainGrid(mainEclipseCase->mainGrid());

        std::vector<QDateTime> timeStepDates = mainEclipseCase->results(RiaDefines::MATRIX_MODEL)->timeStepDates();
        if (timeStepDates.size() == 0)
        {
            return false;
        }

        cvf::ref<RifReaderEclipseOutput> readerEclipseOutput = new RifReaderEclipseOutput;
        if (!readerEclipseOutput->openAndReadActiveCellData(caseFileName(), timeStepDates, eclipseCase.p()))
        {
            return false;
        }

        this->setReservoirData( eclipseCase.p() );

        readerInterface = readerEclipseOutput;
    }

    results(RiaDefines::MATRIX_MODEL)->setReaderInterface(readerInterface.p());
    results(RiaDefines::FRACTURE_MODEL)->setReaderInterface(readerInterface.p());

    CVF_ASSERT(this->eclipseCaseData());
    CVF_ASSERT(readerInterface.notNull());

    eclipseCaseData()->computeActiveCellBoundingBoxes();

    m_activeCellInfoIsReadFromFile = true;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::loadAndUpdateSourSimData()
{
    if (!results(RiaDefines::MATRIX_MODEL)) return;

    results(RiaDefines::MATRIX_MODEL)->setHdf5Filename(m_sourSimFileName);

    if (!hasSourSimFile())
    {
        // Deselect SourSimRL cell results
        for (Rim3dView* view : views())
        {
            RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>(view);
            if (eclipseView != nullptr)
            {
                if (eclipseView->cellResult()->resultType() == RiaDefines::SOURSIMRL)
                {
                    eclipseView->cellResult()->setResultType(RiaDefines::DYNAMIC_NATIVE);
                    eclipseView->cellResult()->setResultVariable("SOIL");
                    eclipseView->loadDataAndUpdate();
                }
            }
        }

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RifReaderInterface> RimEclipseResultCase::createMockModel(QString modelName)
{
    cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;
    cvf::ref<RigEclipseCaseData> reservoir = new RigEclipseCaseData(this);

     if (modelName == RiaDefines::mockModelBasic())
    {
        // Create the mock file interface and and RigSerervoir and set them up.
        mockFileInterface->setWorldCoordinates(cvf::Vec3d(10, 10, 10), cvf::Vec3d(20, 20, 20));
        mockFileInterface->setGridPointDimensions(cvf::Vec3st(4, 5, 6));
        mockFileInterface->addLocalGridRefinement(cvf::Vec3st(0, 2, 2), cvf::Vec3st(0, 2, 2), cvf::Vec3st(3, 3, 3));
        mockFileInterface->enableWellData(false);

        mockFileInterface->open("", reservoir.p());
    }
    else if (modelName == RiaDefines::mockModelBasicWithResults())
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
    else if (modelName == RiaDefines::mockModelLargeWithResults())
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
    else if (modelName == RiaDefines::mockModelCustomized())
    {
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));

        RimMockModelSettings rimMockModelSettings;
        caf::PdmSettings::readFieldsFromApplicationStore(&rimMockModelSettings);

        caf::PdmUiPropertyViewDialog propertyDialog(nullptr, &rimMockModelSettings, "Customize Mock Model", "");
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

    std::vector<QString> relocatedFaultFiles;
    const std::vector<QString>& orgFilesContainingFaults = filesContainingFaults();
    for (auto faultFileName : orgFilesContainingFaults)
    {
        QString relocatedFaultFile = RimTools::relocateFile(faultFileName, newProjectPath, oldProjectPath, &foundFile, &searchedPaths);
        relocatedFaultFiles.push_back(relocatedFaultFile);
    }

    setFilesContainingFaults(relocatedFaultFiles);
    
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
RifReaderEclipseRft* RimEclipseResultCase::rftReader()
{
    return m_readerEclipseRft.p();
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
void RimEclipseResultCase::setSourSimFileName(const QString& fileName)
{
    m_sourSimFileName = fileName;

    loadAndUpdateSourSimData();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimEclipseResultCase::hasSourSimFile()
{
   return !m_sourSimFileName().isEmpty();
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
        if (!this->m_caseName_OBSOLETE().isEmpty() && !caseDirectory().isEmpty())
        {
            caseFileName = QDir::fromNativeSeparators(caseDirectory()) + "/" + m_caseName_OBSOLETE() + ".EGRID";
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

    auto group = uiOrdering.addNewGroup("Case Options");
    group->add(&activeFormationNames);
    group->add(&flipXAxis);
    group->add(&flipYAxis);

    if (eclipseCaseData()
        && eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)
        && eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->maxTimeStepCount() > 0)
    {
        auto group1 = uiOrdering.addNewGroup("Time Step Filter");
        group1->setCollapsedByDefault(true);
        m_timeStepFilter->uiOrdering(uiConfigName, *group1);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_sourSimFileName)
    {
        loadAndUpdateSourSimData();
    }

    return RimEclipseCase::fieldChangedByUi(changedField, oldValue, newValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimEclipseResultCase::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_sourSimFileName)
    {
        caf::PdmUiFilePathEditorAttribute* myAttr = dynamic_cast<caf::PdmUiFilePathEditorAttribute*>(attribute);
        if (myAttr)
        {
            myAttr->m_fileSelectionFilter = "SourSim (*.sourres)";
            myAttr->m_defaultPath = QFileInfo(caseFileName()).absolutePath();
        }
    }
}
