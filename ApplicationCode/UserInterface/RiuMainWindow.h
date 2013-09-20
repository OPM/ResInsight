/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cafUiTreeModelPdm.h"

#include <QMainWindow>
#include <QEvent>
#include <QPointer>

class QTreeView;
class QMdiArea;
class QFrame;
class QMdiSubWindow;
class QComboBox;
class QLabel;
class QLineEdit;
class QItemSelection;
class QActionGroup;
class QSpinBox;

class RiuViewer;
class RiuResultInfoPanel;
class RiuProcessMonitor;
class RimUiTreeModelPdm;
class RimUiTreeView;
class RimCase;

namespace caf
{
    class UiPropertyCreatorPdm;
    class UiTreeModelPdm;
    class PdmObject;
    class FrameAnimationControl;
    class AnimationToolBar;
    class PdmUiPropertyView;
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
    
    void		    initializeGuiNewProjectLoaded();
    void		    cleanupGuiBeforeProjectClose();

    void            removeViewer( RiuViewer* viewer );
    void            addViewer(RiuViewer* viewer);
    void            setActiveViewer(RiuViewer* subWindow);

    void            setResultInfo(const QString& info) const;

    void            refreshAnimationActions();
    void            updateScaleValue();

    RimUiTreeModelPdm* uiPdmModel() { return m_treeModelPdm;}

    RiuProcessMonitor* processMonitor();

    void            hideAllDockWindows();
    void			loadWinGeoAndDockToolBarLayout();

    void            setCurrentObjectInTreeView(caf::PdmObject* object);

    void            selectedCases(std::vector<RimCase*>& cases);

protected:
    virtual void	closeEvent(QCloseEvent* event);

private:
    void            createActions();
    void            createMenus();
    void            createToolBars();
    void            createDockPanels();
    void			saveWinGeoAndDockToolBarLayout();

    bool            checkForDocumentModifications();

    void            updateMRUList(const QString &fileName, bool remove = false);
    
    QMdiSubWindow*  findMdiSubWindow(RiuViewer* viewer);

    void            storeTreeViewState();
    void            restoreTreeViewState();

private:
    static RiuMainWindow*    sm_mainWindowInstance;
    
    QByteArray			    m_initialDockAndToolbarLayout;	// Initial dock window and toolbar layout, used to reset GUI

private:
    // File actions
    QAction*		    m_openEclipseCaseAction;
    QAction*		    m_openInputEclipseFileAction;
    QAction*		    m_openMultipleEclipseCasesAction;
    QAction*		    m_openProjectAction;
    QAction*		    m_openLastUsedProjectAction;
    QAction*		    m_importWellPathsFromFileAction;
    QAction*		    m_importWellPathsFromSSIHubAction;
    QAction*		    m_saveProjectAction;
    QAction*		    m_saveProjectAsAction;
    QAction*            m_closeProjectAction;
    QAction*		    m_exitAction;

    // Edit actions
    QAction*		    m_editPreferences;
    QAction*		    m_newPropertyView;

    // View actions
    QAction*		    m_viewFromNorth;
    QAction*		    m_viewFromSouth;
    QAction*		    m_viewFromEast;
    QAction*		    m_viewFromWest;
    QAction*		    m_viewFromAbove;
    QAction*		    m_viewFromBelow;
    QAction*		    m_zoomAll;

    // Mock actions
    QAction*		    m_mockModelAction;
    QAction*		    m_mockResultsModelAction;
    QAction*		    m_mockLargeResultsModelAction;
    QAction*		    m_mockInputModelAction;

    QAction*		    m_snapshotToFile;
    QAction*		    m_snapshotToClipboard;
    QAction*		    m_snapshotAllViewsToFile;

    QAction*            m_createCommandObject;

    // Help actions
    QAction*            m_aboutAction;
    QAction*            m_commandLineHelpAction;

    // Animation
    caf::AnimationToolBar* m_animationToolBar;

    // Toolbars
    QToolBar*           m_viewToolBar;
    QToolBar*           m_standardToolBar;
    QToolBar*           m_snapshotToolbar;


    QFrame*             m_CentralFrame;
    QMdiArea*           m_mdiArea;
    RiuViewer*           m_mainViewer;
    RiuResultInfoPanel*  m_resultInfoPanel;
    RiuProcessMonitor*   m_processMonitor;
    
    QMenu*              m_windowMenu;


// Menu and action slots
private slots:

    // File slots
    void    slotOpenBinaryGridFiles();
    void    slotOpenInputFiles();
    void    slotOpenMultipleCases();
    void    slotOpenProject();
    void    slotOpenLastUsedProject();
    void    slotImportWellPathsFromFile();
    void    slotImportWellPathsFromSSIHub();
    void    slotSaveProject();
    void    slotSaveProjectAs();
    void    slotCloseProject();

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
    void slotToggleFaultsAction(bool);

    // Debug slots
    void    slotRefreshDebugActions();
    void    slotUseShaders(bool enable);
    void    slotShowPerformanceInfo(bool enable);
    
    void    slotSnapshotToFile();
    void    slotSnapshotToClipboard();
    void    slotSnapshotAllViewsToFile();

    void    slotCreateCommandObject();

    // Mock models
    void    slotMockModel();
    void    slotMockResultsModel();
    void    slotMockLargeResultsModel();
    void    slotInputMockModel();

    // Windows slots
    void    slotBuildWindowActions();

    // Help slots
    void    slotAbout();
    void    slotShowCommandLineHelp();

    void    slotSubWindowActivated(QMdiSubWindow* subWindow);
    void    slotCurrentChanged(const QModelIndex & current, const QModelIndex & previous);

    // Animation slots
    void    slotSetCurrentFrame(int frameIndex);
    void    slotFramerateChanged(double frameRate);

    // Pdm System :
public:
    void setPdmRoot(caf::PdmObject* pdmRoot);

private:
    RimUiTreeView*              m_treeView;
    RimUiTreeModelPdm*          m_treeModelPdm;
    caf::PdmObject*             m_pdmRoot;
    caf::PdmUiPropertyView*     m_pdmUiPropertyView;

    QSpinBox*                   m_scaleFactor;

    QActionGroup*               m_dsActionGroup;
    QAction*                    m_drawStyleToggleFaultsAction;
    QAction*                    m_drawStyleLinesAction;
    QAction*                    m_drawStyleLinesSolidAction;
    QAction*                    m_drawStyleSurfOnlyAction;
    void                        refreshDrawStyleActions();

    std::vector<QPointer<QDockWidget> > additionalProjectTrees;
    std::vector<QPointer<QDockWidget> > additionalPropertyEditors;
};
