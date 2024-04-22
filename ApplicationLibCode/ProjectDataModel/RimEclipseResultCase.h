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

#include "RiaDefines.h"

#include "RimEclipseCase.h"

#include "cafFilePath.h"
#include "cafPdmProxyValueField.h"

class RifReaderRftInterface;
class RifReaderEclipseRft;
class RifReaderOpmRft;
class RifReaderInterface;
class RigFlowDiagSolverInterface;
class RigMainGrid;
class RimEclipseInputProperty;
class RimEclipseInputPropertyCollection;
class RimFlowDiagSolution;

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

    void setCaseInfo( const QString& userDescription, const QString& fileName );
    void setSourSimFileName( const QString& fileName );
    bool hasSourSimFile();

    bool openEclipseGridFile() override;

    bool importGridAndResultMetaData( bool showTimeStepFilter );
    bool importAsciiInputProperties( const QStringList& fileNames ) override;

    void reloadEclipseGridFile() override;
    bool openAndReadActiveCellData( RigEclipseCaseData* mainEclipseCase );
    void readGridDimensions( std::vector<std::vector<int>>& gridDimensions );

    caf::AppEnum<RiaDefines::EclipseUnitSystem> unitSystem();

    QString locationOnDisc() const override;

    RimFlowDiagSolution*              defaultFlowDiagSolution();
    std::vector<RimFlowDiagSolution*> flowDiagSolutions();
    RigFlowDiagSolverInterface*       flowDiagSolverInterface();

    RifReaderRftInterface* rftReader();

    // A multi segment well can have multiple well paths. Valves can be modeled using short branches. This threshold defines the limit for
    // merging branches into the upstream branch.
    int mswMergeThreshold() const;

protected:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    void loadAndUpdateSourSimData();
    void ensureRftDataIsImported();

    cvf::ref<RifReaderInterface> createMockModel( QString modelName );
    void                         defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    cvf::ref<RigFlowDiagSolverInterface> m_flowDagSolverInterface;

    cvf::ref<RifReaderEclipseRft> m_readerEclipseRft;
    cvf::ref<RifReaderOpmRft>     m_readerOpmRft;

    caf::PdmProxyValueField<caf::AppEnum<RiaDefines::EclipseUnitSystem>> m_unitSystem;
    caf::PdmChildArrayField<RimFlowDiagSolution*>                        m_flowDiagSolutions;
    caf::PdmField<caf::FilePath>                                         m_sourSimFileName;

    caf::PdmField<std::pair<bool, int>> m_mswMergeThreshold;

    bool m_gridAndWellDataIsReadFromFile;
    bool m_activeCellInfoIsReadFromFile;
    bool m_useOpmRftReader;
    bool m_rftDataIsReadFromFile;
};
