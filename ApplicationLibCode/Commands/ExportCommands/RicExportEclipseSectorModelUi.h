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

#include "cvfVector3.h"

#include <QString>
#include <QStringList>

#include <set>
#include <vector>

class RigEclipseCaseData;

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
        FULL_GRID_BOX,
        MANUAL_SELECTION
    };
    using GridBoxSelectionEnum = caf::AppEnum<GridBoxSelection>;

public:
    RicExportEclipseSectorModelUi();
    ~RicExportEclipseSectorModelUi() override;
    const QStringList& tabNames() const;

    void setCaseData( RigEclipseCaseData* caseData   = nullptr,
                      const cvf::Vec3i&   visibleMin = cvf::Vec3i::ZERO,
                      const cvf::Vec3i&   visibleMax = cvf::Vec3i::ZERO );

    cvf::Vec3i min() const;
    cvf::Vec3i max() const;
    void       setMin( const cvf::Vec3i& min );
    void       setMax( const cvf::Vec3i& max );
    void       applyBoundaryDefaults();
    void       removeInvalidKeywords();

    QString exportFaultsFilename() const;
    QString exportGridFilename() const;
    QString exportParametersFilename() const;

    caf::PdmField<bool> exportGrid;
    caf::PdmField<bool> exportInLocalCoordinates;
    caf::PdmField<bool> makeInvisibleCellsInactive;

    caf::PdmField<ResultExportOptionsEnum> exportFaults;

    caf::PdmField<ResultExportOptionsEnum> exportParameters;

    caf::PdmField<std::vector<QString>> selectedKeywords;

    caf::PdmField<GridBoxSelectionEnum> exportGridBox;

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

    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
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

    RigEclipseCaseData* m_caseData;
    cvf::Vec3i          m_visibleMin;
    cvf::Vec3i          m_visibleMax;
    QStringList         m_tabNames;
};
