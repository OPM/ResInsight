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
class RicSummaryCurveCreatorSplitterUi : public caf::PdmUiWidgetBasedObjectEditor
{
    Q_OBJECT

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

    void            configureAndUpdateFields(int widgetStartIndex, 
                                             QBoxLayout* layout,
                                             const std::vector<caf::PdmUiItem *>& topLevelUiItems,
                                             const QString& uiConfigName);

    QMinimizePanel*         createGroupBoxWithContent(caf::PdmUiGroup* group,
                                                      const QString& uiConfigName);
signals:
    void    signalCloseButtonPressed();


private:
    QPointer<QVBoxLayout>       m_layout;
    QPointer<QSplitter>         m_firstColumnSplitter;

    QPointer<QMinimizePanel>    m_curvesPanel;

    QPointer<QHBoxLayout>       m_firstRowLayout;
    QPointer<QHBoxLayout>       m_secondRowLayout;
    QPointer<QVBoxLayout>       m_lowerLeftLayout;

    QPointer<QHBoxLayout>       m_bottomFieldLayout;
};
