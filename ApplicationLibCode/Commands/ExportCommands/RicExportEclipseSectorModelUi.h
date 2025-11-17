/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafVecIjk.h"

#include "cvfVector3.h"

#include <QString>
#include <QStringList>

#include <set>
#include <vector>

class RigEclipseCaseData;
class RigSimWellData;
class RimEclipseView;
class RimKeywordBcprop;

//==================================================================================================
///
//==================================================================================================
class RicExportEclipseSectorModelUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    enum ResultExportOptions
    {
        EXPORT_NO_RESULTS,
        EXPORT_TO_GRID_FILE,
        EXPORT_TO_SINGLE_SEPARATE_FILE,
        EXPORT_TO_SEPARATE_FILE_PER_RESULT
    };
    using ResultExportOptionsEnum = caf::AppEnum<ResultExportOptions>;

    enum GridBoxSelection
    {
        VISIBLE_CELLS_BOX,
        ACTIVE_CELLS_BOX,
        VISIBLE_WELLS_BOX,
        FULL_GRID_BOX,
        MANUAL_SELECTION
    };
    using GridBoxSelectionEnum = caf::AppEnum<GridBoxSelection>;

    enum BoundaryCondition
    {
        OPERNUM_OPERATER,
        BCCON_BCPROP
    };
    using BoundaryConditionEnum = caf::AppEnum<BoundaryCondition>;

public:
    RicExportEclipseSectorModelUi();
    ~RicExportEclipseSectorModelUi() override;
    const QStringList& tabNames() const;

    void setCaseData( RigEclipseCaseData* caseData    = nullptr,
                      RimEclipseView*     eclipseView = nullptr,
                      const caf::VecIjk0& visibleMin  = caf::VecIjk0::ZERO,
                      const caf::VecIjk0& visibleMax  = caf::VecIjk0::ZERO );

    caf::VecIjk0 min() const;
    caf::VecIjk0 max() const;
    void         setMin( const caf::VecIjk0& min );
    void         setMax( const caf::VecIjk0& max );
    void         applyBoundaryDefaults();
    void         removeInvalidKeywords();
    cvf::Vec3st  refinement() const;

    QString exportFaultsFilename() const;
    QString exportGridFilename() const;
    QString exportParametersFilename() const;
    bool    writeEchoKeywords() const;

    static std::vector<const RigSimWellData*> getVisibleSimulationWells( RimEclipseView* view );
    static std::pair<caf::VecIjk0, caf::VecIjk0>
        computeVisibleWellCells( RimEclipseView* view, RigEclipseCaseData* caseData, int visibleWellsPadding );

    caf::PdmField<bool> exportGrid;
    caf::PdmField<bool> exportInLocalCoordinates;
    caf::PdmField<bool> makeInvisibleCellsInactive;

    caf::PdmField<ResultExportOptionsEnum> exportFaults;

    caf::PdmField<ResultExportOptionsEnum> exportParameters;

    caf::PdmField<std::vector<QString>> selectedKeywords;

    caf::PdmField<GridBoxSelectionEnum> exportGridBox;

    caf::PdmChildArrayField<RimKeywordBcprop*> m_bcpropKeywords;

    caf::PdmField<bool> m_exportSimulationInput;

    caf::PdmField<BoundaryConditionEnum> m_boundaryCondition;

    caf::PdmField<double> m_porvMultiplier;

    caf::PdmField<int> m_visibleWellsPadding;

    caf::PdmField<int> refinementCountI;
    caf::PdmField<int> refinementCountJ;
    caf::PdmField<int> refinementCountK;

protected:
    caf::PdmField<int> minI;
    caf::PdmField<int> maxI;
    caf::PdmField<int> minJ;
    caf::PdmField<int> maxJ;
    caf::PdmField<int> minK;
    caf::PdmField<int> maxK;

    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    static std::set<QString> mainKeywords();
    QString                  defaultFolder() const;
    QString                  defaultGridFileName() const;
    QString                  defaultResultsFileName() const;
    QString                  defaultFaultsFileName() const;

private:
    caf::PdmField<caf::FilePath> m_exportFolder;
    caf::PdmField<QString>       m_exportFaultsFilename;
    caf::PdmField<QString>       m_exportParametersFilename;
    caf::PdmField<QString>       m_exportGridFilename;
    caf::PdmField<bool>          m_writeEchoInGrdeclFiles;

    RigEclipseCaseData* m_caseData;
    RimEclipseView*     m_eclipseView;
    caf::VecIjk0        m_visibleMin;
    caf::VecIjk0        m_visibleMax;
    QStringList         m_tabNames;
};
