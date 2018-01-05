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

#include "cafPdmUiWidgetBasedObjectEditor.h"

#include <vector>
#include <memory>

class RicSummaryCurveCalculator;

class QMinimizePanel;
class QString;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;

namespace caf {
    class PdmUiItem;
    class PdmUiTableView;
}


//==================================================================================================
///  
///  
//==================================================================================================
class RicSummaryCurveCalculatorEditor : public caf::PdmUiWidgetBasedObjectEditor
{
    Q_OBJECT

public:
    RicSummaryCurveCalculatorEditor();
    ~RicSummaryCurveCalculatorEditor();

    RicSummaryCurveCalculator* calculator() const;

private:
    virtual void        recursivelyConfigureAndUpdateTopLevelUiItems(const std::vector<caf::PdmUiItem *>& topLevelUiItems,
                                                                         const QString& uiConfigName) override;
    
    virtual QWidget*    createWidget(QWidget* parent) override;

    QMinimizePanel*     updateGroupBoxWithContent(caf::PdmUiGroup* group, const QString& uiConfigName);

private slots:
    void                slotCalculate();
    void                slotParseExpression();

private:
    QPointer<QHBoxLayout>   m_firstRowLeftLayout;
    QPointer<QVBoxLayout>   m_firstRowRightLayout;

    QPointer<QHBoxLayout>   m_parseButtonLayout;
    QPointer<QHBoxLayout>   m_calculateButtonLayout;

    caf::PdmUiTableView* m_pdmTableView;

    std::unique_ptr<RicSummaryCurveCalculator> m_calculator;
};
