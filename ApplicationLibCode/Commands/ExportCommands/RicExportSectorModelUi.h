/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025   Equinor ASA
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

#include "cafFilePath.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cafVecIjk.h"

#include "RiaModelExportDefines.h"

#include <QString>
#include <QStringList>

#include <map>

class RimKeywordBcprop;
class RimEclipseCase;
class RimEclipseView;

//==================================================================================================
///
//==================================================================================================
class RicExportSectorModelUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    using GridBoxSelectionEnum  = caf::AppEnum<RiaModelExportDefines::GridBoxSelection>;
    using BoundaryConditionEnum = caf::AppEnum<RiaModelExportDefines::BoundaryCondition>;

public:
    RicExportSectorModelUi();
    ~RicExportSectorModelUi() override;

    const QStringList& pageNames() const;
    const QStringList& pageSubTitles() const;

    void setEclipseView( RimEclipseView* view );

    // data access
    caf::VecIjk0 min() const;
    caf::VecIjk0 max() const;
    void         setMin( const caf::VecIjk0& min );
    void         setMax( const caf::VecIjk0& max );

    cvf::Vec3st refinement() const;

    std::vector<RimKeywordBcprop*>           bcpropKeywords() const;
    RiaModelExportDefines::BoundaryCondition boundaryCondition() const;
    double                                   porvMultiplier() const;
    QString                                  exportDeckFilename() const;

    RiaModelExportDefines::GridBoxSelection gridBoxSelection() const;
    int                                     wellPadding() const;

    bool    shouldCreateSimulationJob() const;
    bool    startSimulationJobAfterExport() const;
    QString newSimulationJobFolder() const;
    QString newSimulationJobName() const;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    std::map<QString, QString> validate( const QString& configName ) const override;

private:
    enum WizardPageEnum : unsigned int
    {
        ExportSettings     = 0,
        GridBoxSelection   = 1,
        GridRefinement     = 2,
        BoundaryConditions = 3,
        SimulationJob      = 4,
        TotalPages         = 5
    };

    void           applyBoundaryDefaults();
    static QString defaultFolder();

private:
    caf::PdmField<caf::FilePath>               m_exportFolder;
    caf::PdmField<QString>                     m_exportDeckName;
    caf::PdmField<double>                      m_porvMultiplier;
    caf::PdmField<int>                         m_visibleWellsPadding;
    caf::PdmField<BoundaryConditionEnum>       m_boundaryCondition;
    caf::PdmField<GridBoxSelectionEnum>        m_gridBoxSelection;
    caf::PdmChildArrayField<RimKeywordBcprop*> m_bcpropKeywords;

    caf::PdmField<bool> m_refineGrid;
    caf::PdmField<int>  m_refinementCountI;
    caf::PdmField<int>  m_refinementCountJ;
    caf::PdmField<int>  m_refinementCountK;

    caf::PdmField<int> m_minI;
    caf::PdmField<int> m_maxI;
    caf::PdmField<int> m_minJ;
    caf::PdmField<int> m_maxJ;
    caf::PdmField<int> m_minK;
    caf::PdmField<int> m_maxK;

    caf::PdmField<bool>          m_createSimulationJob;
    caf::PdmField<caf::FilePath> m_simulationJobFolder;
    caf::PdmField<QString>       m_simulationJobName;
    caf::PdmField<bool>          m_startSimulationJobAfterExport;

    caf::PdmPtrField<RimEclipseCase*> m_eclipseCase;
    caf::PdmPtrField<RimEclipseView*> m_eclipseView;

    QStringList  m_pageNames;
    QStringList  m_pageSubtitles;
    caf::VecIjk0 m_visibleMin;
    caf::VecIjk0 m_visibleMax;
    int          m_totalCells;
};
