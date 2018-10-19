/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "cafPdmUiDragDropInterface.h"
#include "cafPdmObjectHandle.h"

#include <QEvent>
#include <QLabel>
#include <QMdiArea>
#include <QPointer>

#include <memory>
#include <vector>

class QActionGroup;
class QMdiSubWindow;
class QToolButton;
class QSpinBox;
class QTimer;
class QUndoView;

class RimCase;

class RiuMessagePanel;
class RiuProcessMonitor;
class RiuResultInfoPanel;
class RiuResultQwtPlot;
class RiuViewer;
class RiuRelativePermeabilityPlotPanel;
class RiuPvtPlotPanel;
class RiuMohrsCirclePlot;

struct RimMdiWindowGeometry;

namespace caf
{
    class PdmUiTreeView;
    class AnimationToolBar;
    class PdmObject;
    class PdmUiPropertyView;
    class PdmUiItem;
}

namespace ssihub
{
    class Interface;
}

//==================================================================================================
//
// 
//
//==================================================================================================
class RiuMainWindow : public RiuMainWindowBase
{
    Q_OBJECT

public:
    RiuMainWindow();

    static RiuMainWindow* instance();

    QString         mainWindowName() override;
    
    void            initializeGuiNewProjectLoaded();
    void            cleanupGuiCaseClose();
    void            cleanupGuiBeforeProjectClose();

    void            removeViewer( QWidget* viewer ) override;
    void            addViewer(QWidget* viewer, const RimMdiWindowGeometry& windowsGeometry) override;
    void            setActiveViewer(QWidget* subWindow) override;

    void            setResultInfo(const QString& info) const;

    void            refreshAnimationActions();
    void            updateScaleValue();

    RiuProcessMonitor* processMonitor();

    void            hideAllDockWindows();


    void            selectedCases(std::vector<RimCase*>& cases);

    void            setDefaultWindowSize();

    void            refreshDrawStyleActions();
    
    void            setExpanded(const caf::PdmUiItem* uiItem, bool expanded = true);

    RimMdiWindowGeometry    windowGeometryForViewer(QWidget* viewer) override;

    void            tileWindows();
    bool            isAnyMdiSubWindowVisible();
    QMdiSubWindow*  findMdiSubWindow(QWidget* viewer);
    QList<QMdiSubWindow*> subWindowList(QMdiArea::WindowOrder order);

    RiuResultQwtPlot*                   resultPlot();
    RiuRelativePermeabilityPlotPanel*   relativePermeabilityPlotPanel();
    RiuPvtPlotPanel*                    pvtPlotPanel();
    RiuMohrsCirclePlot*                 mohrsCirclePlot();
    RiuMessagePanel*                    messagePanel();

    void            showProcessMonitorDockPanel();
    void            setDefaultToolbarVisibility();

protected:
    void    closeEvent(QCloseEvent* event) override;

private:
    void            createActions();
    void            createMenus();
    void            createToolBars();
    void            createDockPanels();

    void            restoreTreeViewState();

    void            showDockPanel(const QString& dockPanelName);

    void            updateUiFieldsFromActiveResult(caf::PdmObjectHandle* objectToUpdate);

private:
    static RiuMainWindow*    sm_mainWindowInstance;
    
    QByteArray                m_initialDockAndToolbarLayout;    // Initial dock window and toolbar layout, used to reset GUI

private:
    // Edit actions
    QAction*            m_newPropertyView;

    // View actions
    QAction*            m_viewFromNorth;
    QAction*            m_viewFromSouth;
    QAction*            m_viewFromEast;
    QAction*            m_viewFromWest;
    QAction*            m_viewFromAbove;
    QAction*            m_viewFromBelow;

    // Mock actions
    QAction*            m_mockModelAction;
    QAction*            m_mockResultsModelAction;
    QAction*            m_mockLargeResultsModelAction;
    QAction*            m_mockModelCustomizedAction;
    QAction*            m_mockInputModelAction;

    QAction*            m_snapshotAllViewsToFile;

    QAction*            m_createCommandObject;
    QAction*            m_showRegressionTestDialog;
    QAction*            m_executePaintEventPerformanceTest;

    caf::AnimationToolBar* m_animationToolBar;

    QMdiArea*           m_mdiArea;
    RiuViewer*          m_mainViewer;
    RiuResultInfoPanel* m_resultInfoPanel;
    RiuProcessMonitor*  m_processMonitor;
    QPointer<RiuMessagePanel>               m_messagePanel;
    
    RiuResultQwtPlot*                   m_resultQwtPlot;
    RiuMohrsCirclePlot*                 m_mohrsCirclePlot;
    RiuRelativePermeabilityPlotPanel*   m_relPermPlotPanel;
    RiuPvtPlotPanel*                    m_pvtPlotPanel;

    QMenu*              m_windowMenu;
    QLabel*             m_memoryCriticalWarning;
    QToolButton*        m_memoryUsedButton;
    QLabel*             m_memoryTotalStatus;
    QTimer*             m_memoryRefreshTimer;

// Menu and action slots
private slots:

    friend class RiuMdiSubWindow;

    // Memory update slot
    void    updateMemoryUsage();

    // File slots
    void    slotRefreshFileActions();

    // Edit slots
    void    slotRefreshEditActions();
    void    slotNewObjectPropertyView();

    // View slots
    void    slotRefreshViewActions();
    void    slotViewFromNorth();
    void    slotViewFromSouth();
    void    slotViewFromEast();
    void    slotViewFromWest();
    void    slotViewFromAbove();
    void    slotViewFromBelow();
    void    slotScaleChanged(int scaleValue);

    void    slotDrawStyleChanged(QAction* activatedAction);
    void    slotToggleHideGridCellsAction(bool);
    void    slotToggleFaultLabelsAction(bool);
    void    slotDisableLightingAction(bool);

    void    slotShowWellCellsAction(bool doAdd);

    // Debug slots
    void    slotSnapshotAllViewsToFile();

    void    slotCreateCommandObject();

    void    slotShowRegressionTestDialog();
    void    slotExecutePaintEventPerformanceTest();

    // Mock models
    void    slotMockModel();
    void    slotMockResultsModel();
    void    slotMockLargeResultsModel();
    void    slotMockModelCustomized();
    void    slotInputMockModel();

    // Windows slots
    void    slotBuildWindowActions();

    void    slotSubWindowActivated(QMdiSubWindow* subWindow);

    void    selectedObjectsChanged();
    void    customMenuRequested(const QPoint& pos);

    // Pdm System :
public:
    void setPdmRoot(caf::PdmObject* pdmRoot);
private:
    
    std::unique_ptr<caf::PdmUiDragDropInterface> m_dragDropInterface;
    
    QUndoView*                  m_undoView;

    caf::PdmObject*             m_pdmRoot;
    caf::PdmUiPropertyView*     m_pdmUiPropertyView;

    QSpinBox*                   m_scaleFactor;

    QActionGroup*               m_dsActionGroup;
    QAction*                    m_disableLightingAction;
    QAction*                    m_drawStyleHideGridCellsAction;
    QAction*                    m_toggleFaultsLabelAction;
    QAction*                    m_drawStyleLinesAction;
    QAction*                    m_drawStyleLinesSolidAction;
    QAction*                    m_drawStyleFaultLinesSolidAction;
    QAction*                    m_drawStyleSurfOnlyAction;
    QAction*                    m_showWellCellsAction;

    QToolBar*                   m_holoLensToolBar;

    std::vector<QPointer<QDockWidget> > additionalProjectViews;

    bool                        m_blockSlotSubWindowActivated;
};
