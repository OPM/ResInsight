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

#include "RifReaderInterfaceEcl.h"
#include "RimReservoir.h"
#include "RigReservoir.h"
#include "RigMainGrid.h"
#include "RigReservoirCellResults.h"

#include "RimWell.h"
#include "RimWellCollection.h"
#include "RimReservoirView.h"

#include "cvfAssert.h"

#include <QString>
#include "RifReaderInterfaceMock.h"

CAF_PDM_SOURCE_INIT(RimReservoir, "EclipseCase");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoir::RimReservoir()
{
    m_rigReservoir = NULL;
    m_fileInterface = NULL;

    CAF_PDM_InitObject("Reservoir", ":/AppLogo48x48.png", "", "");
    CAF_PDM_InitField(&caseName, "CaseName",  QString(), "Case name", "", "" ,"");
    CAF_PDM_InitField(&caseDirectory, "CaseFolder", QString(), "Directory", "", "" ,"");

    CAF_PDM_InitFieldNoDefault(&reservoirViews, "ReservoirViews", "",  "", "", "");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimReservoir::RimReservoir(RigReservoir* reservoir)
{
    m_rigReservoir = reservoir;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigReservoir* RimReservoir::reservoirData()
{
    return m_rigReservoir.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigReservoir* RimReservoir::reservoirData() const
{
    return m_rigReservoir.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderInterface* RimReservoir::fileInterface()
{
    return m_fileInterface.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RifReaderInterface* RimReservoir::fileInterface() const
{
    return m_fileInterface.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimReservoir::openEclipseGridFile()
{
    // Early exit if reservoir data is created
    if (m_rigReservoir.notNull()) return true;

    if (caseName().contains("Mock Debug Model"))
    {
        this->createMockModel(this->caseName());
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
        m_fileInterface = new RifReaderInterfaceECL;
        if (!m_fileInterface->open(fname, reservoir))
        {
            delete reservoir;
            return false;
        }

        m_rigReservoir = reservoir;
    }

    CVF_ASSERT(m_rigReservoir.notNull());
    CVF_ASSERT(m_fileInterface.notNull());

    m_rigReservoir->mainGrid()->results()->setReaderInterface(m_fileInterface.p());
    m_rigReservoir->computeFaults();
    m_rigReservoir->mainGrid()->computeCachedData();

    return true;
 }


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoir::createMockModel(QString modelName)
{
    cvf::ref<RifReaderInterfaceMock> mockFileInterface = new RifReaderInterfaceMock;
    cvf::ref<RigReservoir> reservoir = new RigReservoir;

     if (modelName == "Mock Debug Model Simple")
    {
        // Create the mock file interface and and RigSerervoir and set them up.
        mockFileInterface->setWorldCoordinates(cvf::Vec3d(10, 10, 10), cvf::Vec3d(20, 20, 20));
        mockFileInterface->setGridPointDimensions(cvf::Vec3st(4, 5, 6));
        mockFileInterface->addLocalGridRefinement(cvf::Vec3st(0, 2, 2), cvf::Vec3st(0, 2, 2), cvf::Vec3st(3, 3, 3));

        mockFileInterface->open("", reservoir.p());
        {
            size_t idx = reservoir->mainGrid()->cellIndexFromIJK(1, 3, 4);
            reservoir->mainGrid()->cell(idx).setActive(false);
        }

        {
            size_t idx = reservoir->mainGrid()->cellIndexFromIJK(2, 2, 3);
            reservoir->mainGrid()->cell(idx).setActive(false);
        }
    }
    else if (modelName == "Mock Debug Model With Results")
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
    else if (modelName =="Mock Debug Model Large With Results")
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
    m_fileInterface = mockFileInterface;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimReservoir::initAfterRead()
{
    size_t j;
    for (j = 0; j < reservoirViews().size(); j++)
    {
        RimReservoirView* riv = reservoirViews()[j];
        CVF_ASSERT(riv);

        riv->setEclipseCase(this);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoir::~RimReservoir()
{
    reservoirViews.deleteChildren();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimReservoirView* RimReservoir::createAndAddReservoirView()
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
void RimReservoir::removeReservoirView(RimReservoirView* reservoirView)
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

