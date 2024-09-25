/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RiuMainWindowBase.h"

#include "cafPdmPointer.h"

#include <QMdiArea>
#include <QPointer>
#include <QString>

#include <memory>

struct RimMdiWindowGeometry;

class QMdiSubWindow;
class RiuViewer;
class RimViewWindow;
class RicSummaryPlotEditorDialog;
class RicSummaryCurveCalculatorDialog;
class RiuMessagePanel;

namespace caf
{
class PdmUiTreeView;
class PdmObject;
class PdmUiPropertyView;
class PdmUiItem;
class PdmUiToolBarEditor;
} // namespace caf

//==================================================================================================
//
//
//
//==================================================================================================
class RiuPlotMainWindow : public RiuMainWindowBase
{
    Q_OBJECT

public:
    RiuPlotMainWindow();
    ~RiuPlotMainWindow() override;

    static RiuPlotMainWindow* instance();
    static void               onWellSelected( const QString& wellName, int timeStep );

    QString mainWindowName() override;

    void initializeGuiNewProjectLoaded();
    void cleanupGuiBeforeProjectClose();
    void cleanUpTemporaryWidgets();

    void removeViewer( QWidget* viewer ) override;
    void initializeViewer( QMdiSubWindow* subWindow, QWidget* viewer, const RimMdiWindowGeometry& windowsGeometry ) override;
    void setActiveViewer( QWidget* subWindow ) override;

    void setDefaultWindowSize();
    void enable3DSelectionLink( bool enable );
    bool selection3DLinkEnabled();

    bool                  isAnyMdiSubWindowVisible();
    QMdiSubWindow*        findMdiSubWindow( QWidget* viewer ) override;
    RimViewWindow*        findViewWindowFromSubWindow( QMdiSubWindow* subWindow );
    QList<QMdiSubWindow*> subWindowList( QMdiArea::WindowOrder order );

    void setWidthOfMdiWindow( QWidget* mdiWindowWidget, int newWidth );
    void addToTemporaryWidgets( QWidget* widget );

    void updateWellLogPlotToolBar();
    void updateMultiPlotToolBar();

    RicSummaryPlotEditorDialog*      summaryCurveCreatorDialog( bool createIfNotPresent );
    RicSummaryCurveCalculatorDialog* summaryCurveCalculatorDialog( bool createIfNotPresent );

    RiuMessagePanel* messagePanel();

    void showAndSetKeyboardFocusToSummaryPlotManager();

protected:
    void closeEvent( QCloseEvent* event ) override;
    void keyPressEvent( QKeyEvent* ) override;
    void dragEnterEvent( QDragEnterEvent* event ) override;
    void dropEvent( QDropEvent* event ) override;

    QStringList defaultDockStateNames() override;
    QStringList windowsMenuFeatureNames() override;

private:
    void setPdmRoot( caf::PdmObject* pdmRoot );

    void createMenus();
    void createToolBars();
    void createDockPanels();

    void restoreTreeViewState();

    void refreshToolbars();

    static QStringList toolbarCommandIds( const QString& toolbarName = "" );

private slots:
    void slotToggleSelectionLink();

    friend class RiuMdiSubWindow;

    void slotBuildWindowActions();

    void slotSubWindowActivated( QMdiSubWindow* subWindow );

    void selectedObjectsChanged( caf::PdmUiTreeView* projectTree, caf::PdmUiPropertyView* propertyView );
    void customMenuRequested( const QPoint& pos );

private:
    caf::PdmPointer<RimViewWindow> m_activePlotViewWindow;
    QPointer<RiuMessagePanel>      m_messagePanel;

    std::unique_ptr<caf::PdmUiToolBarEditor> m_wellLogPlotToolBarEditor;
    std::unique_ptr<caf::PdmUiToolBarEditor> m_multiPlotToolBarEditor;
    std::unique_ptr<caf::PdmUiToolBarEditor> m_multiPlotLayoutToolBarEditor;

    std::unique_ptr<caf::PdmUiPropertyView> m_summaryPlotManagerView;

    std::unique_ptr<RicSummaryPlotEditorDialog>      m_summaryCurveCreatorDialog;
    std::unique_ptr<RicSummaryCurveCalculatorDialog> m_summaryCurveCalculatorDialog;
    std::unique_ptr<caf::PdmObject>                  m_summaryPlotManager;

    std::vector<QWidget*> m_temporaryWidgets;

    QAction* m_toggleSelectionLinkAction;
    bool     m_selection3DLinkEnabled;
};
