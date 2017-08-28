/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "cafPdmObject.h"
#include "cafPdmField.h"

#include <vector>

class QDateTime;
class RimEclipseResultCase;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RimTimeStepFilter : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum TimeStepFilterTypeEnum
    {
        TS_ALL,
        TS_INTERVAL_DAYS,
        TS_INTERVAL_WEEKS,
        TS_INTERVAL_MONTHS,
        TS_INTERVAL_QUARTERS,
        TS_INTERVAL_YEARS
    };

public:
    RimTimeStepFilter();

    void                    setTimeStepsFromFile(const std::vector<QDateTime>& timeSteps);
    void                    clearTimeStepsFromFile();
    
    std::vector<size_t>     selectedTimeStepIndices() const;

private:
    QString                 filteredTimeStepsAsText() const;

    void                    updateDerivedData();
    void                    updateSelectedTimeStepIndices();

    std::vector<QDateTime>  allTimeSteps() const;
    std::vector<int>        selectedTimeStepIndicesFromUi() const;

    void                    updateFieldVisibility();

    RimEclipseResultCase*   parentEclipseResultCase() const;

    // PDM overrides
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    virtual void defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute) override;

private:
    caf::PdmField< std::vector<int> > m_selectedTimeStepIndices; 

    caf::PdmField< caf::AppEnum< TimeStepFilterTypeEnum > > m_filterType;

    caf::PdmField<int>      m_firstTimeStep;
    caf::PdmField<int>      m_lastTimeStep;
    caf::PdmField<int>      m_interval;

    caf::PdmField<QString>  m_filteredTimeStepsText;

    caf::PdmField<bool>     m_applyReloadOfCase;

    std::vector<QDateTime>  m_timeStepsFromFile;
};
