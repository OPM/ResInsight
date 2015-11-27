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

#include <QEvent>
#include <QMainWindow>
#include <QPointer>

class QActionGroup;
class QComboBox;
class QFrame;
class QItemSelection;
class QLabel;
class QLineEdit;
class QMdiArea;
class QMdiSubWindow;
class QSpinBox;
class QTreeView;
class QUndoView;

class RimCase;
class RimEclipseCase;
class RiuProcessMonitor;
class RiuResultInfoPanel;
class RiuViewer;
class RiuWellLogPlot;
class RiuResultQwtPlot;

namespace caf
{
    class PdmUiTreeView;
    class AnimationToolBar;
    class FrameAnimationControl;
    class PdmObject;
    class PdmUiPropertyView;
    class UiPropertyCreatorPdm;
    class PdmUiItem;
    class PdmUiDragDropInterface;
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
class RiuMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    RiuMainWindow();
    static RiuMainWindow* instance();
    
    void            initializeGuiNewProjectLoaded();
    void            cleanupGuiBeforeProjectClose();

    void            removeViewer( QWidget* viewer );
    void            addViewer(QWidget* viewer, const std::vector<int>& windowsGeometry);
    void            setActiveViewer(QWidget* subWindow);

    void            setResultInfo(const QString& info) const;

    void            refreshAnimationActions();
    void            updateScaleValue();

    caf::PdmUiTreeView* projectTreeView() { return m_projectTreeView;}
    RiuProcessMonitor* processMonitor();

    void            hideAllDockWindows();
    void            loadWinGeoAndDockToolBarLayout();
    void            showWindow();

    void            setCurrentObjectInTreeView(caf::PdmObject* object);

    void            selectedCases(std::vector<RimCase*>& cases);

    void            setDefaultWindowSize();

    void            refreshDrawStyleActions();
    
    void            setExpanded(const caf::PdmUiItem* uiItem, bool expanded);

    void            addRecentFiles(const QString& file);
    void            removeRecentFiles(const QString& file);

    std::vector<int>    windowGeometryForViewer(QWidget* viewer);
    std::vector<int>    windowGeometryForWidget(QWidget* widget);

    void            tileWindows();
    bool            isAnyMdiSubWindowVisible();
    QMdiSubWindow*  findMdiSubWindow(QWidget* viewer);

    RiuResultQwtPlot* resultPlot();

protected:
    virtual void    closeEvent(QCloseEvent* event);

private:
    void            createActions();
    void            createMenus();
    void            createToolBars();
    void            createDockPanels();
    void            saveWinGeoAndDockToolBarLayout();

    bool            checkForDocumentModifications();

    void            updateRecentFileActions();

    void            storeTreeViewState();
    void            restoreTreeViewState();

private:
    static RiuMainWindow*    sm_mainWindowInstance;
    
    QByteArray                m_initialDockAndToolbarLayout;    // Initial dock window and toolbar layout, used to reset GUI

private:
    // File actions
    QAction*            m_importGeoMechCaseAction;
    QAction*            m_openProjectAction;
    QAction*            m_openLastUsedProjectAction;
    QAction*            m_saveProjectAction;
    QAction*            m_saveProjectAsAction;
    QAction*            m_closeProjectAction;
    QAction*            m_exitAction;

    // Recent files
    enum { MaxRecentFiles = 5 };
    QAction*            m_recentFilesSeparatorAction;
    QMenu*              m_recentFilesMenu;
    QAction*            m_recentFileActions[MaxRecentFiles];


    // Edit actions
    QAction*            m_editPreferences;
    QAction*            m_newPropertyView;

    // View actions
    QAction*            m_viewFromNorth;
    QAction*            m_viewFromSouth;
    QAction*            m_viewFromEast;
    QAction*            m_viewFromWest;
    QAction*            m_viewFromAbove;
    QAction*            m_viewFromBelow;
    QAction*            m_zoomAll;

    // Mock actions
    QAction*            m_mockModelAction;
    QAction*            m_mockResultsModelAction;
    QAction*            m_mockLargeResultsModelAction;
    QAction*            m_mockModelCustomizedAction;
    QAction*            m_mockInputModelAction;

    QAction*            m_snapshotToFile;
    QAction*            m_snapshotToClipboard;
    QAction*            m_snapshotAllViewsToFile;

    QAction*            m_createCommandObject;
    QAction*            m_showRegressionTestDialog;
    QAction*            m_executePaintEventPerformanceTest;

    // Help actions
    QAction*            m_aboutAction;
    QAction*            m_commandLineHelpAction;
    QAction*            m_openUsersGuideInBrowserAction;

    // Animation
    caf::AnimationToolBar* m_animationToolBar;

    // Toolbars
    QToolBar*           m_viewToolBar;
    QToolBar*           m_standardToolBar;
    QToolBar*           m_snapshotToolbar;


    QMdiArea*           m_mdiArea;
    RiuViewer*          m_mainViewer;
    RiuResultInfoPanel* m_resultInfoPanel;
    RiuProcessMonitor*  m_processMonitor;
    
    RiuResultQwtPlot*   m_resultQwtPlot;
    
    QMenu*              m_windowMenu;


// Menu and action slots
private slots:

    friend class RiuMdiSubWindow;

    // File slots
    void    slotImportGeoMechModel();
    void    slotOpenProject();
    void    slotOpenLastUsedProject();
    void    slotSaveProject();
    void    slotSaveProjectAs();
    void    slotCloseProject();

    void    slotOpenRecentFile();

    void    slotRefreshFileActions();

    // Edit slots
    void    slotRefreshEditActions();
    void    slotEditPreferences();
    void    slotNewObjectPropertyView();

    // View slots
    void    slotRefreshViewActions();
    void    slotViewFromNorth();
    void    slotViewFromSouth();
    void    slotViewFromEast();
    void    slotViewFromWest();
    void    slotViewFromAbove();
    void    slotViewFromBelow();
    void    slotZoomAll();
    void    slotScaleChanged(int scaleValue);

    void slotDrawStyleChanged(QAction* activatedAction);
    void slotToggleHideGridCellsAction(bool);
    void slotToggleFaultLabelsAction(bool);
    void slotDisableLightingAction(bool);

    void slotAddWellCellsToRangeFilterAction(bool doAdd);

    // Debug slots
    void    slotUseShaders(bool enable);
    void    slotShowPerformanceInfo(bool enable);
    
    void    slotSnapshotToFile();
    void    slotSnapshotToClipboard();
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

    // Help slots
    void    slotAbout();
    void    slotShowCommandLineHelp();
    void    slotOpenUsersGuideInBrowserAction();

    void    slotSubWindowActivated(QMdiSubWindow* subWindow);

    void    selectedObjectsChanged();
    void    customMenuRequested(const QPoint& pos);


    // Animation slots
    void    slotFramerateChanged(double frameRate);

    // Pdm System :
public:
    void setPdmRoot(caf::PdmObject* pdmRoot);
private:
    caf::PdmUiTreeView*            m_projectTreeView;
    
    caf::PdmUiDragDropInterface* m_dragDropInterface;
    
    QUndoView*                  m_undoView;

    caf::PdmObject*             m_pdmRoot;
    caf::PdmUiPropertyView*     m_pdmUiPropertyView;

    QSpinBox*                   m_scaleFactor;

    QActionGroup*               m_dsActionGroup;
    QAction*                    m_disableLightingAction;
    QAction*                    m_drawStyleToggleFaultsAction;
    QAction*                    m_toggleFaultsLabelAction;
    QAction*                    m_drawStyleLinesAction;
    QAction*                    m_drawStyleLinesSolidAction;
    QAction*                    m_drawStyleFaultLinesSolidAction;
    QAction*                    m_drawStyleSurfOnlyAction;
    QAction*                    m_addWellCellsToRangeFilterAction;

    std::vector<QPointer<QDockWidget> > additionalProjectViews;

    bool                        m_blockSlotSubWindowActivated;
};
