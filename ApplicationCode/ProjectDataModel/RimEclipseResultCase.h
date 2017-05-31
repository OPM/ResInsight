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

#pragma once

#include "RimEclipseCase.h"

class RifReaderInterface;
class RigMainGrid;
class RimFlowDiagSolution;
class RigFlowDiagSolverInterface;

//==================================================================================================
//
// 
//
//==================================================================================================
class RimEclipseResultCase : public RimEclipseCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseResultCase();
    virtual ~RimEclipseResultCase();

    void                        setGridFileName(const QString& caseFileName);
    void                        setCaseInfo(const QString& userDescription, const QString& caseFileName);

    virtual bool                openEclipseGridFile();
    virtual void                reloadEclipseGridFile();
    bool                        openAndReadActiveCellData(RigEclipseCaseData* mainEclipseCase);
    void                        readGridDimensions(std::vector< std::vector<int> >& gridDimensions);

    // Overrides from RimCase
    virtual QString             locationOnDisc() const;
    virtual QString             gridFileName() const { return caseFileName();}
    virtual void                updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath);

    RimFlowDiagSolution*              defaultFlowDiagSolution();
    std::vector<RimFlowDiagSolution*> flowDiagSolutions();
    RigFlowDiagSolverInterface*       flowDiagSolverInterface();

private:
    cvf::ref<RifReaderInterface> createMockModel(QString modelName);

    virtual void                initAfterRead();

    virtual void                defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering );

    cvf::ref<RigFlowDiagSolverInterface>        m_flowDagSolverInterface;

    // Fields:                        
    caf::PdmField<QString>      caseFileName;
    caf::PdmChildArrayField<RimFlowDiagSolution*> m_flowDiagSolutions;


    // Obsolete field
    caf::PdmField<QString>      caseDirectory; 

    bool                        m_gridAndWellDataIsReadFromFile;
    bool                        m_activeCellInfoIsReadFromFile;
};
