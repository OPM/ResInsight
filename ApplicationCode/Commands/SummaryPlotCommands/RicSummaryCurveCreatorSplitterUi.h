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

class RicSummaryCurveCreator;

class QHBoxLayout;
class QMinimizePanel;
class QSplitter;
class QString;
class QVBoxLayout;

namespace caf {
    class PdmUiItem;
}


class RicSummaryCurveCreatorSplitterUi : public caf::PdmUiWidgetBasedObjectEditor
{
public:
    RicSummaryCurveCreatorSplitterUi(QWidget* parent);
    ~RicSummaryCurveCreatorSplitterUi();

private:
    virtual void            recursivelyConfigureAndUpdateTopLevelUiItems(const std::vector<caf::PdmUiItem *>& topLevelUiItems,
                                                                         const QString& uiConfigName) override;
    
    virtual QWidget*        createWidget(QWidget* parent) override;

    QWidget*                getOrCreateCurveTreeWidget();
    QWidget*                getOrCreatePlotWidget();

    static caf::PdmUiGroup* findGroupByKeyword(const std::vector<caf::PdmUiItem *>& topLevelUiItems,
                                               const QString& keyword,
                                               const QString& uiConfigName);

private:
    QPointer<QVBoxLayout>       m_layout;
    QPointer<QSplitter>         m_firstColumnSplitter;

    QPointer<QMinimizePanel>    m_curvesPanel;

    QPointer<QHBoxLayout>       m_firstRowLayout;
    QPointer<QHBoxLayout>       m_secondRowLayout;
    QPointer<QVBoxLayout>       m_lowerLeftLayout;
};
