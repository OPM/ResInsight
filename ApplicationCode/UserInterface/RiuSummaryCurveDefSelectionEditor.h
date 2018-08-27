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

#include "cafPdmUiFormLayoutObjectEditor.h"

#include <vector>
#include <memory>

class RimSummaryCase;
class RiuSummaryCurveDefSelection;
class RifEclipseSummaryAddress;

class QMinimizePanel;
class QSplitter;
class QString;
class QVBoxLayout;
class QHBoxLayout;
class QBoxLayout;

namespace caf {
    class PdmUiItem;
}


//==================================================================================================
///  
///  
//==================================================================================================
class RiuSummaryCurveDefSelectionEditor : public caf::PdmUiFormLayoutObjectEditor
{
public:
    RiuSummaryCurveDefSelectionEditor();
    ~RiuSummaryCurveDefSelectionEditor();

    RiuSummaryCurveDefSelection* summaryAddressSelection() const;

private:
    virtual void        recursivelyConfigureAndUpdateTopLevelUiItems(const std::vector<caf::PdmUiItem *>& topLevelUiItems,
                                                                         const QString& uiConfigName) override;
    
    virtual QWidget*    createWidget(QWidget* parent) override;

    void                configureAndUpdateFields(int widgetStartIndex, 
                                                 QBoxLayout* layout,
                                                 const std::vector<caf::PdmUiItem *>& topLevelUiItems,
                                                 const QString& uiConfigName);

    QMinimizePanel*     createGroupBoxWithContent(caf::PdmUiGroup* group, const QString& uiConfigName);

private:
    QPointer<QHBoxLayout>   m_firstRowLeftLayout;
    QPointer<QHBoxLayout>   m_firstRowRightLayout;

    std::unique_ptr<RiuSummaryCurveDefSelection>  m_summaryAddressSelection;
};
