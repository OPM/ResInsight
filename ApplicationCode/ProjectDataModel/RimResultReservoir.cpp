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
#include "RimResultReservoir.h"
#include "RigReservoir.h"
#include "RifReaderEclipseOutput.h"
#include "RigReservoirCellResults.h"
#include "RimReservoirView.h"
#include "RifReaderMockModel.h"
#include "RifReaderEclipseInput.h"
#include "cafProgressInfo.h"


CAF_PDM_SOURCE_INIT(RimResultReservoir, "EclipseCase");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimResultReservoir::RimResultReservoir()
    : RimReservoir()
{
    CAF_PDM_InitField(&caseFileName, "CaseFileName",  QString(), "Case file name", "", "" ,"");
    CAF_PDM_InitField(&caseDirectory, "CaseFolder", QString(), "Directory", "", "" ,"");
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimResultReservoir::openEclipseGridFile()
{
    caf::ProgressInfo progInfo(20, "Reading Eclipse Grid File");

    progInfo.setProgressDescription("Open Grid File");
    progInfo.setNextProgressIncrement(19);
    // Early exit if reservoir data is created
    if (m_rigReservoir.notNull()) return true;

    cvf::ref<RifReaderInterface> readerInterface;

    if (caseName().contains("Result Mock Debug Model"))
    {
        readerInterface = this->createMockModel(this->caseName());
    }
    else
    {
        QString fullCaseName = caseName + ".EGRID";

        QDir dir(caseDirectory.v());
        if (!dir.exists(fullCaseName))
        {
            fullCaseName = caseName + ".GRID";
            if (!dir.exists(fullCaseName))
            {
                return false;
            }
        }

        QString fname = dir.absoluteFilePath(fullCaseName);

        RigReservoir* reservoir = new RigReservoir;
        readerInterface = new RifReaderEclipseOutput;
        if (!readerInterface->open(fname, reservoir))
        {
            delete reservoir;
            return false;
        }

        m_rigReservoir = reservoir;
    }

    progInfo.setProgress(19);

    CVF_ASSERT(m_rigReservoir.notNull());
    CVF_ASSERT(readerInterface.notNull());

    m_rigReservoir->mainGrid()->results(RifReaderInterface::MATRIX_RESULTS)->setReaderInterface(readerInterface.p());
    m_rigReservoir->mainGrid()->results(RifReaderInterface::FRACTURE_RESULTS)->setReaderInterface(readerInterface.p());

    progInfo.setProgressDescription("Computing Faults");
    m_rigReservoir->computeFaults();
    progInfo.setProgressDescription("Computing Cache");
    m_rigReservoir->mainGrid()->computeCachedData();

    return true;
 }


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<RifReaderInterface> RimResultReservoir::createMockModel(QString modelName)
{
    cvf::ref<RifReaderMockModel> mockFileInterface = new RifReaderMockModel;
    cvf::ref<RigReservoir> reservoir = new RigReservoir;

     if (modelName == "Result Mock Debug Model Simple")
    {
        // Create the mock file interface and and RigSerervoir and set them up.
        mockFileInterface->setWorldCoordinates(cvf::Vec3d(10, 10, 10), cvf::Vec3d(20, 20, 20));
        mockFileInterface->setGridPointDimensions(cvf::Vec3st(4, 5, 6));
        mockFileInterface->addLocalGridRefinement(cvf::Vec3st(0, 2, 2), cvf::Vec3st(0, 2, 2), cvf::Vec3st(3, 3, 3));

        mockFileInterface->open("", reservoir.p());
        {
            size_t idx = reservoir->mainGrid()->cellIndexFromIJK(1, 3, 4);
            reservoir->mainGrid()->cell(idx).setActiveIndexInMatrixModel(cvf::UNDEFINED_SIZE_T);
        }

        {
            size_t idx = reservoir->mainGrid()->cellIndexFromIJK(2, 2, 3);
            reservoir->mainGrid()->cell(idx).setActiveIndexInMatrixModel(cvf::UNDEFINED_SIZE_T);
        }
    }
    else if (modelName == "Result Mock Debug Model With Results")
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
    else if (modelName =="Result Mock Debug Model Large With Results")
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

    m_rigReservoir = reservoir;

    return mockFileInterface.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimResultReservoir::~RimResultReservoir()
{
    reservoirViews.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimResultReservoir::locationOnDisc() const
{
    return caseDirectory;
}

