////////////////////////////    /////////////////////////////////////////////////////
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

#include "RiuCalculationsContextMenuManager.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include <memory>

class RimSummaryCalculationCollection;
class RimSummaryCalculation;

//==================================================================================================
///
//==================================================================================================
class RicSummaryCurveCalculator : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicSummaryCurveCalculator();

    static QString calculatedSummariesGroupName();
    static QString calulationGroupName();

    RimSummaryCalculation* currentCalculation() const;
    void                   setCurrentCalculation( RimSummaryCalculation* calculation );

    bool parseExpression() const;
    bool calculate() const;

    static RimSummaryCalculationCollection* calculationCollection();

private:
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                         QString                    uiConfigName,
                                                         caf::PdmUiEditorAttribute* attribute ) override;
    void                          onEditorWidgetsCreated() override;

private:
    // TODO : Move to a common caf helper class
    static void assignPushButtonEditor( caf::PdmFieldHandle* fieldHandle );
    static void assignPushButtonEditorText( caf::PdmUiEditorAttribute* attribute, const QString& text );

private:
    caf::PdmPtrField<RimSummaryCalculation*> m_currentCalculation;

    caf::PdmField<bool> m_newCalculation;
    caf::PdmField<bool> m_deleteCalculation;

    std::unique_ptr<RiuCalculationsContextMenuManager> m_calcContextMenuMgr;
};
