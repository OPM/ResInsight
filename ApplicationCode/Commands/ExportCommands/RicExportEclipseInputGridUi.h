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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmChildArrayField.h"
#include "cafAppEnum.h"

#include <QString>
#include <QStringList>

#include <set>
#include <vector>

class RigEclipseCaseData;

//==================================================================================================
/// 
//==================================================================================================
class RicExportEclipseInputGridUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

    enum ResultExportOptions
    {
        EXPORT_NO_RESULTS,
        EXPORT_TO_GRID_FILE,
        EXPORT_TO_SINGLE_SEPARATE_FILE,
        EXPORT_TO_SEPARATE_FILE_PER_RESULT
    };
    typedef caf::AppEnum<ResultExportOptions> ResultExportOptionsEnum;

public:
    RicExportEclipseInputGridUi(RigEclipseCaseData* caseData = nullptr);
    ~RicExportEclipseInputGridUi() override;

    std::vector<QString> allSelectedKeywords() const;

    caf::PdmField<bool>                    exportGrid;
    caf::PdmField<ResultExportOptionsEnum> exportResults;
    caf::PdmField<QString>                 exportGridFilename;    
    caf::PdmField<QString>                 exportResultsFilename;
    caf::PdmField<std::vector<QString>>    exportMainKeywords;
    caf::PdmField<std::vector<QString>>    exportAdditionalKeywords;

    caf::PdmField<int> cellCountI;
    caf::PdmField<int> cellCountJ;
    caf::PdmField<int> cellCountK;

protected:
    void            defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;
    void            defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                        bool*                      useOptionsOnly) override;

    static std::set<QString> mainKeywords();
    QString defaultGridFileName() const;
    QString defaultResultsFileName() const;
private:
    RigEclipseCaseData* m_caseData;
};
