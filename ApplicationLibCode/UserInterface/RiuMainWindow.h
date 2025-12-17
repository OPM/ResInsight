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

#include "cafPdmObjectHandle.h"

#include <QEvent>
#include <QLabel>
#include <QMdiArea>
#include <QPointer>
#include <QString>

#include <memory>
#include <vector>

class QActionGroup;
class QMdiSubWindow;
class QToolButton;
class QComboBox;
class QTimer;
class QUndoView;
class QSlider;

class Rim3dView;
class RimCase;
class RimViewWindow;

class RiuMessagePanel;
class RiuProcessMonitor;
class RiuResultInfoPanel;
class RiuResultQwtPlot;
class RiuDepthQwtPlot;
class RiuRelativePermeabilityPlotPanel;
class RiuPvtPlotPanel;
class RiuMohrsCirclePlot;
class RiuMdiArea;
class RiuSeismicHistogramPanel;
class RiuCellSelectionTool;

class RicGridCalculatorDialog;

struct RimMdiWindowGeometry;

namespace caf
{
class PdmUiTreeView;
class AnimationToolBar;
class PdmObject;
class PdmUiPropertyView;
class PdmUiItem;
} // namespace caf

namespace ads
{
class CDockWidget;
};

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
    ~RiuMainWindow() override;

    static RiuMainWindow* instance();
    static void           closeIfOpen();

    QString mainWindowName() override;

    void initializeGuiNewProjectLoaded();
    void cleanupGuiCaseClose();
    void cleanupGuiBeforeProjectClose();

    void removeViewer( QWidget* viewer ) override;
    void initializeViewer( QMdiSubWindow* subWindow, QWidget* viewer, const RimMdiWindowGeometry& windowsGeometry ) override;
    void setActiveViewer( QWidget* subWindow ) override;

    void setResultInfo( const QString& info ) const;

    void refreshViewActions();
    void refreshAnimationActions();
    void updateScaleValue();

    RiuProcessMonitor* processMonitor();

    void selectedCases( std::vector<RimCase*>& cases );

    void setDefaultWindowSize();

    void refreshDrawStyleActions();

    bool                  isAnyMdiSubWindowVisible();
    QMdiSubWindow*        findMdiSubWindow( QWidget* viewer ) override;
    RimViewWindow*        findViewWindowFromSubWindow( QMdiSubWindow* lhs );
    QList<QMdiSubWindow*> subWindowList( QMdiArea::WindowOrder order );

    RiuResultQwtPlot*                 resultPlot();
    RiuDepthQwtPlot*                  depthPlot();
    RiuRelativePermeabilityPlotPanel* relativePermeabilityPlotPanel();
    RiuPvtPlotPanel*                  pvtPlotPanel();
    RiuMohrsCirclePlot*               mohrsCirclePlot();
    RiuSeismicHistogramPanel*         seismicHistogramPanel();
    RiuMessagePanel*                  messagePanel();

    void showProcessMonitorDockPanel();
    void setDefaultToolbarVisibility();
    void applyFontSizesToDockedPlots();

    RicGridCalculatorDialog* gridCalculatorDialog( bool createIfNotPresent );

protected:
    void        closeEvent( QCloseEvent* event ) override;
    QStringList defaultDockStateNames() override;
    QStringList windowsMenuFeatureNames() override;

    void dragEnterEvent( QDragEnterEvent* event ) override;
    void dropEvent( QDropEvent* event ) override;

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createDockPanels();

    void restoreTreeViewState();

    void updateUiFieldsFromActiveResult( caf::PdmObjectHandle* objectToUpdate );

private:
    // Edit actions
    QAction* m_newPropertyView;

    // View actions
    QAction* m_viewFromNorth;
    QAction* m_viewFromSouth;
    QAction* m_viewFromEast;
    QAction* m_viewFromWest;
    QAction* m_viewFromAbove;
    QAction* m_viewFromBelow;
    QAction* m_viewFullScreen;

    // Mock actions
    QAction* m_mockModelAction;
    QAction* m_mockResultsModelAction;
    QAction* m_mockLargeResultsModelAction;
    QAction* m_mockModelCustomizedAction;
    QAction* m_mockInputModelAction;

    QAction* m_snapshotAllViewsToFile;

    QAction* m_showRegressionTestDialog;
    QAction* m_executePaintEventPerformanceTest;
    QAction* m_sendTestTelemetryAction;

    caf::AnimationToolBar* m_animationToolBar;

    RiuResultInfoPanel*       m_resultInfoPanel;
    RiuProcessMonitor*        m_processMonitor;
    QPointer<RiuMessagePanel> m_messagePanel;

    RiuResultQwtPlot*                 m_resultQwtPlot;
    RiuDepthQwtPlot*                  m_depthQwtPlot;
    RiuMohrsCirclePlot*               m_mohrsCirclePlot;
    RiuRelativePermeabilityPlotPanel* m_relPermPlotPanel;
    RiuSeismicHistogramPanel*         m_seismicHistogramPanel;
    RiuPvtPlotPanel*                  m_pvtPlotPanel;
    RiuCellSelectionTool*             m_cellSelectionTool;

    std::unique_ptr<RicGridCalculatorDialog> m_gridCalculatorDialog;

    QLabel*      m_memoryCriticalWarning;
    QToolButton* m_memoryUsedButton;
    QLabel*      m_memoryTotalStatus;
    QTimer*      m_memoryRefreshTimer;
    QLabel*      m_versionInfo;

    // Menu and action slots
private slots:

    friend class RiuMdiSubWindow;

    // Memory update slot
    void updateMemoryUsage();

    // File slots
    void slotRefreshFileActions();

    // Edit slots
    void slotNewObjectPropertyView();

    // View slots
    void slotRefreshViewActions();
    void slotViewFullScreen( bool );
    void slotViewFromNorth();
    void slotViewFromSouth();
    void slotViewFromEast();
    void slotViewFromWest();
    void slotViewFromAbove();
    void slotViewFromBelow();
    void slotScaleChanged( int scaleValue );

    void slotDrawStyleChanged( QAction* activatedAction );
    void slotToggleHideGridCellsAction( bool );
    void slotToggleFaultLabelsAction( bool );
    void slotToggleLightingAction( bool );

    void slotShowWellCellsAction( bool doAdd );

    void slotAnimationSliderMoved( int newValue );
    void slotAnimationControlFrameChanged( int newValue );

    // Debug slots
    void slotSnapshotAllViewsToFile();

    void slotShowRegressionTestDialog();
    void slotExecutePaintEventPerformanceTest();
    void slotSendTestTelemetry();

    // Mock models
    void slotMockModel();
    void slotMockResultsModel();
    void slotMockLargeResultsModel();
    void slotMockModelCustomized();
    void slotInputMockModel();

    // Windows slots
    void slotBuildWindowActions();
    void slotSubWindowActivated( QMdiSubWindow* subWindow );

    void selectedObjectsChanged();
    void customMenuRequested( const QPoint& pos );

private:
    void selectViewInProjectTreePreservingSubItemSelection( const Rim3dView* previousActiveReservoirView, Rim3dView* activatedView );

    // Pdm System :
public:
    void setPdmRoot( caf::PdmObject* pdmRoot );

private:
    caf::PdmObject*         m_pdmRoot;
    caf::PdmUiPropertyView* m_pdmUiPropertyView;
    caf::PdmUiPropertyView* m_quickAccessView;

    QComboBox* m_scaleFactor;

    QActionGroup* m_dsActionGroup;
    QAction*      m_enableLightingAction;
    QAction*      m_drawStyleHideGridCellsAction;
    QAction*      m_toggleFaultsLabelAction;
    QAction*      m_drawStyleLinesAction;
    QAction*      m_drawStyleLinesSolidAction;
    QAction*      m_drawStyleFaultLinesSolidAction;
    QAction*      m_drawStyleSurfOnlyAction;
    QAction*      m_showWellCellsAction;
    QAction*      m_drawStyleDeformationsAction;
    QAction*      m_animationSliderAction;

    QSlider* m_animationSlider;

    QToolBar* m_holoLensToolBar;

    std::vector<QPointer<ads::CDockWidget>> m_additionalProjectViews;
};
