/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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
#include <QPointer>
#include <memory>

class RimSummaryCalculationVariable;
class QTextEdit;

class RimSummaryCalculationBase : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    void    setDescription( const QString& description );
    QString description() const;

    void setId( int id );
    int  id() const;

    const std::vector<double>& values() const;
    const std::vector<time_t>& timeSteps() const;

    virtual bool isCumulative() const = 0;

protected:
    caf::PdmFieldHandle* userDescriptionField() override;

    void setValues( const std::vector<double>& values );
    void setTimeSteps( const std::vector<time_t>& timeSteps );

public:
    RimSummaryCalculationBase();

private:
    caf::PdmField<int>                 m_id;
    caf::PdmField<QString>             m_description;
    caf::PdmField<std::vector<double>> m_calculatedValues;
    caf::PdmField<std::vector<time_t>> m_timesteps;
};

class RimSummaryValuesFromScript : public RimSummaryCalculationBase
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryValuesFromScript();

    void setValuesAndTimeSteps( const std::vector<double>& values, const std::vector<time_t>& timeSteps );
    void setCumulative( bool enable );

    bool isCumulative() const override;

private:
    caf::PdmField<bool> m_isCumulative;
};

class RimSummaryCalculation : public RimSummaryCalculationBase
{
    CAF_PDM_HEADER_INIT;

public:
    RimSummaryCalculation();

    bool isDirty() const;

    caf::PdmChildArrayFieldHandle* variables();

    std::vector<RimSummaryCalculationVariable*> allVariables() const;

    void    setExpression( const QString& expr );
    QString expression() const;
    QString unitName() const;

    bool parseExpression();
    bool calculate();
    void updateDependentCurvesAndPlots();

    static QString findLeftHandSide( const QString& expression );
    void           attachToWidget();

    bool isCumulative() const override;

private:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void                           defineEditorAttribute( const caf::PdmFieldHandle* field,
                                                          QString                    uiConfigName,
                                                          caf::PdmUiEditorAttribute* attribute ) override;
    RimSummaryCalculationVariable* findByName( const QString& name ) const;
    RimSummaryCalculationVariable* addVariable( const QString& name );
    void                           deleteVariable( RimSummaryCalculationVariable* calcVariable );

    QString buildCalculationName() const;

private:
    caf::PdmField<QString> m_expression;
    caf::PdmField<QString> m_unit;

    caf::PdmChildArrayField<RimSummaryCalculationVariable*> m_variables;

    std::unique_ptr<RiuExpressionContextMenuManager> m_exprContextMenuMgr;

    bool m_isDirty;
};
