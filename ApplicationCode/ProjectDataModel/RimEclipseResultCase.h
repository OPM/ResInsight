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

#include "RiaEclipseUnitTools.h"

#include "RimEclipseCase.h"

#include <cafPdmProxyValueField.h>

class RifReaderInterface;
class RigMainGrid;
class RimFlowDiagSolution;
class RigFlowDiagSolverInterface;
class RifReaderEclipseRft;

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
    ~RimEclipseResultCase() override;

    void                        setGridFileName(const QString& fileName);
    void                        setCaseInfo(const QString& userDescription, const QString& fileName);
    void                        setSourSimFileName(const QString& fileName);
    bool                        hasSourSimFile();

    bool                openEclipseGridFile() override;

    bool                        importGridAndResultMetaData(bool showTimeStepFilter);

    void                reloadEclipseGridFile() override;
    bool                        openAndReadActiveCellData(RigEclipseCaseData* mainEclipseCase);
    void                        readGridDimensions(std::vector< std::vector<int> >& gridDimensions);

    // Overrides from RimCase
    QString             locationOnDisc() const override;
    QString             gridFileName() const override { return caseFileName();}
    void                updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath) override;

    RimFlowDiagSolution*              defaultFlowDiagSolution();
    std::vector<RimFlowDiagSolution*> flowDiagSolutions();
    RigFlowDiagSolverInterface*       flowDiagSolverInterface();

    RifReaderEclipseRft*        rftReader();

protected:
    void                fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

private:
    void                        loadAndUpdateSourSimData();

private:
    cvf::ref<RifReaderInterface> createMockModel(QString modelName);

    void                initAfterRead() override;

    void                defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    cvf::ref<RigFlowDiagSolverInterface>        m_flowDagSolverInterface;

    cvf::ref<RifReaderEclipseRft> m_readerEclipseRft;

    // Fields:                        
    caf::PdmField<QString>      caseFileName;
    caf::PdmProxyValueField<RiaEclipseUnitTools::UnitSystemType> m_unitSystem;
    caf::PdmChildArrayField<RimFlowDiagSolution*> m_flowDiagSolutions;
    caf::PdmField<QString>      m_sourSimFileName;

    // Obsolete field
    caf::PdmField<QString>      caseDirectory; 

    bool                        m_gridAndWellDataIsReadFromFile;
    bool                        m_activeCellInfoIsReadFromFile;
};
