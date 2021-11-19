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
#include "RiuMdiArea.h"

#include "cafPdmPointer.h"
#include "cafPdmUiDragDropInterface.h"

#include <QPointer>

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

    QString mainWindowName() override;

    void initializeGuiNewProjectLoaded();
    void cleanupGuiBeforeProjectClose();
    void cleanUpTemporaryWidgets();

    void removeViewer( QWidget* viewer ) override;
    void initializeViewer( QMdiSubWindow* subWindow, QWidget* viewer, const RimMdiWindowGeometry& windowsGeometry ) override;
    void setActiveViewer( QWidget* subWindow ) override;

    void setDefaultWindowSize();

    void tileSubWindows() override;
    void storeSubWindowTiling( bool tiled ) override;
    void clearWindowTiling() override;
    bool subWindowsAreTiled() const override;

    bool                  isAnyMdiSubWindowVisible();
    QMdiSubWindow*        findMdiSubWindow( QWidget* viewer ) override;
    RimViewWindow*        findViewWindowFromSubWindow( QMdiSubWindow* subWindow );
    QList<QMdiSubWindow*> subWindowList( QMdiArea::WindowOrder order );

    void setWidthOfMdiWindow( QWidget* mdiWindowWidget, int newWidth );
    void addToTemporaryWidgets( QWidget* widget );

    void updateWellLogPlotToolBar();
    void updateMultiPlotToolBar();
    void updateSummaryPlotToolBar( bool forceUpdateUi = false );
    void setFocusToLineEditInSummaryToolBar();

    RicSummaryPlotEditorDialog*      summaryCurveCreatorDialog();
    RicSummaryCurveCalculatorDialog* summaryCurveCalculatorDialog();

    RiuMessagePanel* messagePanel();

    void showAndSetKeyboardFocusToSummaryPlotManager();

protected:
    void closeEvent( QCloseEvent* event ) override;
    void keyPressEvent( QKeyEvent* ) override;

private:
    void setPdmRoot( caf::PdmObject* pdmRoot );

    void createMenus();
    void createToolBars();
    void createDockPanels();

    void restoreTreeViewState();

    void refreshToolbars();

    static QStringList toolbarCommandIds( const QString& toolbarName = "" );

private slots:

    friend class RiuMdiSubWindow;

    void slotBuildWindowActions();

    void slotSubWindowActivated( QMdiSubWindow* subWindow );

    void selectedObjectsChanged();
    void customMenuRequested( const QPoint& pos );

private:
    QByteArray m_initialDockAndToolbarLayout; // Initial dock window and toolbar layout, used to reset GUI

    RiuMdiArea*                    m_mdiArea;
    caf::PdmPointer<RimViewWindow> m_activePlotViewWindow;
    QPointer<RiuMessagePanel>      m_messagePanel;

    QMenu* m_windowMenu;

    caf::PdmUiToolBarEditor*                     m_wellLogPlotToolBarEditor;
    caf::PdmUiToolBarEditor*                     m_multiPlotToolBarEditor;
    caf::PdmUiToolBarEditor*                     m_summaryPlotToolBarEditor;
    std::unique_ptr<caf::PdmUiDragDropInterface> m_dragDropInterface;

    caf::PdmUiPropertyView* m_pdmUiPropertyView;
    caf::PdmUiPropertyView* m_summaryPlotManagerView;

    QPointer<RicSummaryPlotEditorDialog>      m_summaryCurveCreatorDialog;
    QPointer<RicSummaryCurveCalculatorDialog> m_summaryCurveCalculatorDialog;
    std::unique_ptr<caf::PdmObject>           m_summaryPlotManager;

    std::vector<QWidget*> m_temporaryWidgets;
};
