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

class RimUserDefinedCalculationCollection;
class RimUserDefinedCalculation;

//==================================================================================================
///
//==================================================================================================
class RicUserDefinedCalculatorUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicUserDefinedCalculatorUi();

    RimUserDefinedCalculation* currentCalculation() const;
    void                       setCurrentCalculation( RimUserDefinedCalculation* calculation );

    bool parseExpression() const;
    bool calculate() const;

    virtual QString                              calculationsGroupName() const                                       = 0;
    virtual QString                              calulationGroupName() const                                         = 0;
    virtual RimUserDefinedCalculationCollection* calculationCollection() const                                       = 0;
    virtual void                                 notifyCalculatedNameChanged( int id, const QString& newName ) const = 0;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          onEditorWidgetsCreated() override;

    // TODO : Move to a common caf helper class
    static void assignPushButtonEditor( caf::PdmFieldHandle* fieldHandle );
    static void assignPushButtonEditorText( caf::PdmUiEditorAttribute* attribute, const QString& text );

private:
    void onVariableUpdated( const SignalEmitter* emitter );
    void connectSignals( RimUserDefinedCalculation* calculation );

private:
    caf::PdmPtrField<RimUserDefinedCalculation*> m_currentCalculation;

    caf::PdmField<bool> m_newCalculation;
    caf::PdmField<bool> m_deleteCalculation;

    std::unique_ptr<RiuCalculationsContextMenuManager> m_calcContextMenuMgr;
};
