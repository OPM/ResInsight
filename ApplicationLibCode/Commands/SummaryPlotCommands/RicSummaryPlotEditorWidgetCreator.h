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

#include <memory>
#include <vector>

class RimSummaryPlot;
class RimSummaryCase;
class RicSummaryPlotEditorUi;

class QMinimizePanel;
class QSplitter;
class QString;
class QVBoxLayout;
class QHBoxLayout;
class QBoxLayout;

namespace caf
{
class PdmObject;
class PdmUiItem;
class PdmUiTreeView;
} // namespace caf

//==================================================================================================
///
///
//==================================================================================================
class RicSummaryPlotEditorWidgetCreator : public caf::PdmUiFormLayoutObjectEditor
{
    Q_OBJECT

public:
    RicSummaryPlotEditorWidgetCreator( QWidget* parent );
    ~RicSummaryPlotEditorWidgetCreator() override;

    void updateFromSummaryPlot( RimSummaryPlot* summaryPlot );
    void updateFromDefaultSources( const std::vector<caf::PdmObject*> defaultSources );

private:
    void recursivelyConfigureAndUpdateTopLevelUiOrdering( const caf::PdmUiOrdering& topLevelUiOrdering,
                                                          const QString&            uiConfigName ) override;

    QWidget* createWidget( QWidget* parent ) override;

    QMinimizePanel* getOrCreateCurveTreeGroup();
    QWidget*        getOrCreatePlotWidget();

    static caf::PdmUiGroup* findGroupByKeyword( const std::vector<caf::PdmUiItem*>& topLevelUiItems,
                                                const QString&                      keyword,
                                                const QString&                      uiConfigName );

    void configureAndUpdateFields( int                                 widgetStartIndex,
                                   QBoxLayout*                         layout,
                                   const std::vector<caf::PdmUiItem*>& topLevelUiItems,
                                   const QString&                      uiConfigName );

    QMinimizePanel* createGroupBoxWithContent( caf::PdmUiGroup* group, const QString& uiConfigName );
signals:
    void signalCloseButtonPressed();

private:
    QPointer<QVBoxLayout> m_layout;
    QPointer<QSplitter>   m_firstColumnSplitter;

    QPointer<QMinimizePanel> m_curvesPanel;

    QPointer<QHBoxLayout> m_firstRowLayout;
    QPointer<QHBoxLayout> m_secondRowLayout;
    QPointer<QVBoxLayout> m_lowerLeftLayout;
    QPointer<QVBoxLayout> m_lowerRightLayout;

    QPointer<QHBoxLayout> m_bottomFieldLayout;

    QPointer<caf::PdmUiTreeView> m_curveTreeView;

    QWidget* m_parentWidget;

    std::unique_ptr<RicSummaryPlotEditorUi> m_summaryCurveCreator;
};
