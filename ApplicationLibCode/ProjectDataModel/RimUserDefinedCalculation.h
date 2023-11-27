/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RiuExpressionContextMenuManager.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <memory>

class RimUserDefinedCalculationVariable;

//==================================================================================================
///
///
//==================================================================================================
class RimUserDefinedCalculation : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<> variableUpdated;

public:
    RimUserDefinedCalculation();

    void    setDescription( const QString& description );
    QString description() const;

    void setId( int id );
    int  id() const;

    bool isDirty() const;

    caf::PdmChildArrayFieldHandle* variables();

    std::vector<RimUserDefinedCalculationVariable*> allVariables() const;

    void         setExpression( const QString& expr );
    QString      expression() const;
    void         setUnit( const QString& unit );
    QString      unitName() const;
    bool         parseExpression();
    virtual bool preCalculate() const;
    virtual bool calculate()              = 0;
    virtual void updateDependentObjects() = 0;
    virtual void removeDependentObjects() = 0;

    caf::PdmFieldHandle* userDescriptionField() override;

    static QString findLeftHandSide( const QString& expression );
    void           attachToWidget();

    QString shortName() const;

    RimUserDefinedCalculationVariable* addVariable( const QString& name );

protected:
    virtual RimUserDefinedCalculationVariable* createVariable() = 0;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    RimUserDefinedCalculationVariable* findByName( const QString& name ) const;
    void                               deleteVariable( RimUserDefinedCalculationVariable* calcVariable );

    virtual QString buildCalculationName() const;

protected:
    caf::PdmField<QString> m_description;
    caf::PdmField<QString> m_expression;
    caf::PdmField<bool>    m_helpButton;
    caf::PdmField<QString> m_helpText;
    caf::PdmField<QString> m_unit;

    caf::PdmChildArrayField<RimUserDefinedCalculationVariable*> m_variables;

    caf::PdmField<std::vector<double>> m_calculatedValues_OBSOLETE;
    caf::PdmField<std::vector<time_t>> m_timesteps_OBSOLETE;
    caf::PdmField<int>                 m_id;

    std::unique_ptr<RiuExpressionContextMenuManager> m_exprContextMenuMgr;

    bool m_isDirty;
};
