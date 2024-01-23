//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#include "cvfLibCore.h"
#include "cvfLibRender.h"
#include "cvfLibGeometry.h"
#include "cvfLibViewing.h"

#include "QTBMainWindow.h"
#include "QTBVizWidget.h"
#include "QTBSceneFactory.h"

#include "cvfqtUtils.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QDockWidget>
#include <QSettings>


//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTBMainWindow::QTBMainWindow()
{
    QFrame* mainFrame = new QFrame(this);
    QVBoxLayout* layout = new QVBoxLayout(mainFrame);
    mainFrame->setLayout(layout);
    setCentralWidget(mainFrame);

    m_dockWidget_1 = new QDockWidget("MyDockWidget_1", this);
    m_dockWidget_1->setObjectName("myDockWidget_1");
    addDockWidget(Qt::LeftDockWidgetArea, m_dockWidget_1);

    m_dockWidget_2 = new QDockWidget("MyDockWidget_2", this);
    m_dockWidget_2->setObjectName("myDockWidget_2");
    addDockWidget(Qt::LeftDockWidgetArea, m_dockWidget_2);

    resizeDocks({ m_dockWidget_1, m_dockWidget_2 }, { 200, 200 }, Qt::Horizontal);
    //loadWinGeoAndDockToolBarLayout();


    // The common context group must be created before launching any widgets
    // If we don't create one, each widget will get its own context group
    m_commonContextGroup = new cvf::OpenGLContextGroup; 

    addVizWidget(AddToCentralWidget);
    addVizWidget(InDockWidget_1);
    addVizWidget(InDockWidget_2);
    addVizWidget(AsNewTopLevelWidget);

    {
        QMenu* menu = menuBar()->addMenu("&Widgets");
        menu->addAction("Add VizWidget into CentralWidget", this, SLOT(slotAddVizWidget()))->setData(AddToCentralWidget);
        menu->addAction("Add VizWidget as TopLevel", this, SLOT(slotAddVizWidget()))->setData(AsNewTopLevelWidget);
        menu->addAction("Add VizWidget into DockWidget_1", this, SLOT(slotAddVizWidget()))->setData(InDockWidget_1);
        menu->addAction("Add VizWidget into DockWidget_2", this, SLOT(slotAddVizWidget()))->setData(InDockWidget_2);

        menu->addSeparator();
        menu->addAction("Delete VizWidget", this, SLOT(slotDeleteVizWidget()));

        menu->addSeparator();
        menu->addAction("Reparent VizWidget into CentralWidget", this, SLOT(slotReparentVizWidgetIntoCentralWidget()));
        menu->addAction("Reparent VizWidget as Top Level", this, SLOT(slotReparentVizWidgetAsTopLevel()));
        menu->addAction("Move VizWidget to DockWidget_1", this, SLOT(slotMoveVizWidgetToDockWidget_1()));
        menu->addAction("Move VizWidget to DockWidget_2", this, SLOT(slotMoveVizWidgetToDockWidget_2()));
        menu->addSeparator();
        menu->addAction("Reparent ALL VizWidgets into CentralWidget", this, SLOT(slotReparentAllVizWidgetsIntoCentralWidget()));

        menu->addSeparator();
        menu->addAction("Refresh VizWidget arr", this, SLOT(slotRefreshVizWidgetArr()));
    }
    {
        QMenu* menu = menuBar()->addMenu("&Activate");
        connect(menu, SIGNAL(aboutToShow()), SLOT(slotAboutToShowActivateMenu()));
    }
    {
        QMenu* menu = menuBar()->addMenu("&Scenes");

        menu->addAction("Create test scene", this, SLOT(slotCreateTestScene()));
        menu->addAction("Clear scene", this, SLOT(slotClearScene()));
    }
    {
        QMenu* menu = menuBar()->addMenu("&Test");

        menu->addAction("Save layout", this, SLOT(slotSaveLayout()));
        menu->addAction("Restore layout", this, SLOT(slotRestoreLayout()));
        menu->addSeparator();
        menu->addAction("Grab a native handle", this, SLOT(slotGrabNativeHandle()));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTBMainWindow::~QTBMainWindow()
{
    cvf::Trace::show("QTBMainWindow::~QTBMainWindow()");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::addVizWidget(NewWidgetPlacement newWidgetPlacement)
{
    // Guard against creating duplicate viewers inside dock widgets
    if (newWidgetPlacement == InDockWidget_1 && m_dockWidget_1->widget())
    {
        cvf::Trace::show("DockWidget_1 already contains a viewer");
        return;
    }
    if (newWidgetPlacement == InDockWidget_2 && m_dockWidget_2->widget())
    {
        cvf::Trace::show("DockWidget_2 already contains a viewer");
        return;
    }

    removeStaleEntriesFromVizWidgetArr();

    QWidget* parentToUseOnConstruction = this;
    //QWidget* parentToUseOnConstruction = NULL;

    // Use the common context group if we have one - otherwise create a new context group per widget
    cvf::ref<cvf::OpenGLContextGroup> contextGroupToUse = m_commonContextGroup;
    if (contextGroupToUse.isNull())
    {
        contextGroupToUse = new cvf::OpenGLContextGroup;
    }

    QTBVizWidget* newVizWidget = newVizWidget = new QTBVizWidget(contextGroupToUse.p(), parentToUseOnConstruction, this);

    m_vizWidgetArr.push_back(newVizWidget);

    // Place the widget where instructed
    if (newWidgetPlacement == AddToCentralWidget)
    {
        centralWidget()->layout()->addWidget(newVizWidget);
    }
    else if (newWidgetPlacement == AsNewTopLevelWidget)
    {
        newVizWidget->setParent(nullptr);
        newVizWidget->resize(400, 300);
        newVizWidget->move(50, 200);
        newVizWidget->show();
    }
    else if (newWidgetPlacement == InDockWidget_1)
    {
        m_dockWidget_1->setWidget(newVizWidget);
        newVizWidget->show();
    }
    else if (newWidgetPlacement == InDockWidget_2)
    {
        m_dockWidget_2->setWidget(newVizWidget);
        newVizWidget->show();
    }

    assignViewTitlesToVizWidgetsInArr();
    decideNewActiveVizWidgetAndHighlight();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::handleVizWidgetIsOpenGLReady(QTBVizWidget* vizWidget)
{
    cvf::Trace::show("QTBMainWindow::handleVizWidgetIsOpenGLReady()");
    
    CVF_ASSERT(vizWidget->isValid());
    cvf::OpenGLContext* oglContext = vizWidget->cvfOpenGLContext();
    cvf::OpenGLContextGroup* oglContextGroup = oglContext->group();
    CVF_ASSERT(oglContextGroup->isContextGroupInitialized());

    if (m_commonContextGroup.notNull())
    {
        CVF_ASSERT(m_commonContextGroup == oglContextGroup);
        CVF_ASSERT(m_commonContextGroup->containsContext(oglContext));
        populateAllValidVizWidgetsWithTestScene(true);
    }
    else
    {
        populateAllValidVizWidgetsWithTestScene(false);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::populateAllValidVizWidgetsWithTestScene(bool allWidgetsShareSameScene)
{
    if (allWidgetsShareSameScene)
    {
        cvf::ref<cvf::Scene> sceneToShow;

        // Look for an existing scene
        for (auto vizWidget : m_vizWidgetArr)
        {
            if (vizWidget->isValid() && vizWidget->scene())
            {
                sceneToShow = vizWidget->scene();
                break;
            }
        }

        if (sceneToShow.isNull())
        {
            QTBVizWidget* firstValidVizWidget = nullptr;
            for (auto vizWidget : m_vizWidgetArr)
            {
                if (vizWidget->isValid())
                {
                    firstValidVizWidget = vizWidget;
                    break;
                }
            }

            if (!firstValidVizWidget)
            {
                cvf::Trace::show("Could not find any valid VizWidgets");
                return;
            }

            cvf::OpenGLContext* oglContext = firstValidVizWidget->cvfOpenGLContext();
            CVF_ASSERT(oglContext);
            const cvf::OpenGLCapabilities capabilities = *(oglContext->capabilities());
            const bool useShaders = capabilities.supportsOpenGL2();
            QTBSceneFactory sceneFactory(useShaders);
            sceneToShow = sceneFactory.createTestScene(capabilities);
        }

        for (auto vizWidget : m_vizWidgetArr)
        {
            vizWidget->setScene(sceneToShow.p());
            vizWidget->update();
        }
    }
    else
    {
        for (auto vizWidget : m_vizWidgetArr)
        {
            if (vizWidget->isValid() && !vizWidget->scene())
            {
                cvf::OpenGLContext* oglContext = vizWidget->cvfOpenGLContext();
                CVF_ASSERT(oglContext);
                const cvf::OpenGLCapabilities capabilities = *(oglContext->capabilities());
                const bool useShaders = capabilities.supportsOpenGL2();
                QTBSceneFactory sceneFactory(useShaders);
                cvf::ref<cvf::Scene> scene = sceneFactory.createTestScene(capabilities);

                vizWidget->setScene(scene.p());
                vizWidget->update();
            }
        }
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::removeStaleEntriesFromVizWidgetArr()
{
    std::vector<QPointer<QTBVizWidget> > oldArr = m_vizWidgetArr;
    m_vizWidgetArr.clear();

    for (size_t i = 0; i < oldArr.size(); i++)
    {
        QTBVizWidget* vizWidget = oldArr[i];
        if (vizWidget)
        {
            m_vizWidgetArr.push_back(vizWidget);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::assignViewTitlesToVizWidgetsInArr()
{
    for (size_t i = 0; i < m_vizWidgetArr.size(); i++)
    {
        QTBVizWidget* vizWidget = m_vizWidgetArr[i];
        if (vizWidget)
        {
            vizWidget->setViewTitle(cvf::String("view_%1").arg(static_cast<int>(i)));
            vizWidget->update();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> QTBMainWindow::vizWidgetNames()
{
    std::vector<cvf::String> nameArr;
    for (auto vizWidget : m_vizWidgetArr)
    {
        nameArr.push_back(vizWidget->viewTitle());
    }

    return nameArr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::decideNewActiveVizWidgetAndHighlight()
{
    // For now, simply choose the last one
    m_activeVizWidgetName = "";
    const std::vector<cvf::String> nameArr = vizWidgetNames();
    if (nameArr.size() > 0)
    {
        m_activeVizWidgetName = nameArr.back();
    }

    highlightActiveVizWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::highlightActiveVizWidget()
{
    for (auto vizWidget : m_vizWidgetArr)
    {
        const bool isActive = vizWidget->viewTitle() == m_activeVizWidgetName;
        vizWidget->setViewTitleHighlighted(isActive);
        vizWidget->update();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTBVizWidget* QTBMainWindow::getActiveVizWidget()
{
    if (m_vizWidgetArr.size() == 0)
    {
        cvf::Trace::show("Could not get active VizWidget, no widgets present");
        return nullptr;
    }

    for (auto vizWidget : m_vizWidgetArr)
    {
        if (vizWidget->viewTitle() == m_activeVizWidgetName)
        {
            return vizWidget;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::saveWinGeoAndDockToolBarLayout()
{
    QSettings settings;
    settings.setValue("winGeometry", saveGeometry());
    settings.setValue("dockAndToolBarLayout", saveState(0));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::loadWinGeoAndDockToolBarLayout()
{
    QSettings settings;
    QVariant winGeo = settings.value("winGeometry");
    QVariant layout = settings.value("dockAndToolBarLayout");

    if (winGeo.isValid())
    {
        if (restoreGeometry(winGeo.toByteArray()))
        {
            if (layout.isValid())
            {
                restoreState(layout.toByteArray(), 0);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::closeEvent(QCloseEvent* event)
{
    cvf::Trace::show("QTBMainWindow::closeEvent()");
    QMainWindow::closeEvent(event);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotAddVizWidget()
{
    QAction* senderAction = dynamic_cast<QAction*>(sender());
    const int newWidgetPlacementInt = senderAction->data().toInt();
    NewWidgetPlacement newWidgetPlacement = static_cast<NewWidgetPlacement>(newWidgetPlacementInt);

    addVizWidget(newWidgetPlacement);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotDeleteVizWidget()
{
    removeStaleEntriesFromVizWidgetArr();
    
    QTBVizWidget* vizWidget = getActiveVizWidget();
    if (!vizWidget)
    {
        return;
    }

    delete vizWidget;

    removeStaleEntriesFromVizWidgetArr();
    assignViewTitlesToVizWidgetsInArr();
    decideNewActiveVizWidgetAndHighlight();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotReparentVizWidgetIntoCentralWidget()
{
    QTBVizWidget* vizWidget = getActiveVizWidget();
    if (!vizWidget)
    {
        return;
    }

    CVF_ASSERT(vizWidget->isValid());

    centralWidget()->layout()->addWidget(vizWidget);
    vizWidget->show();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotReparentVizWidgetAsTopLevel()
{
    QTBVizWidget* vizWidget = getActiveVizWidget();
    if (!vizWidget)
    {
        return;
    }

    CVF_ASSERT(vizWidget->isValid());

    vizWidget->setParent(nullptr);
    vizWidget->move(100, 100);
    vizWidget->resize(600, 400);
    vizWidget->show();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotMoveVizWidgetToDockWidget_1()
{
    if (m_dockWidget_1->widget())
    {
        cvf::Trace::show("DockWidget_1 already contains a vizWiget");
        return;
    }

    QTBVizWidget* vizWidget = getActiveVizWidget();
    if (!vizWidget)
    {
        return;
    }

    CVF_ASSERT(vizWidget->isValid());

    m_dockWidget_1->setWidget(vizWidget);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotMoveVizWidgetToDockWidget_2()
{
    if (m_dockWidget_2->widget())
    {
        cvf::Trace::show("DockWidget_2 already contains a vizWiget");
        return;
    }

    QTBVizWidget* vizWidget = getActiveVizWidget();
    if (!vizWidget)
    {
        return;
    }

    CVF_ASSERT(vizWidget->isValid());

    m_dockWidget_2->setWidget(vizWidget);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotReparentAllVizWidgetsIntoCentralWidget()
{
    for (size_t i = 0; i < m_vizWidgetArr.size(); i++)
    {
        QTBVizWidget* vizWidget = m_vizWidgetArr[i];
        if (vizWidget)
        {
            centralWidget()->layout()->addWidget(vizWidget);
            vizWidget->show();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotRefreshVizWidgetArr()
{
    const size_t numWidgetsBefore = m_vizWidgetArr.size();

    removeStaleEntriesFromVizWidgetArr();
    assignViewTitlesToVizWidgetsInArr();

    const size_t numWidgetsAfter = m_vizWidgetArr.size();

    cvf::Trace::show("Refreshed VizWidgetArr, oldCount=%d newCount=%d", numWidgetsBefore, numWidgetsAfter);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotAboutToShowActivateMenu()
{
    QMenu* senderMenu = dynamic_cast<QMenu*>(sender());
    CVF_ASSERT(senderMenu);
    senderMenu->clear();

    const std::vector<cvf::String> nameArr = vizWidgetNames();
    for (auto widgetName : nameArr)
    {
        const QString qstrWidgetName(cvfqt::Utils::toQString(widgetName));
        senderMenu->addAction(qstrWidgetName, this, SLOT(slotSomeVizWidgetActivated()))->setData(qstrWidgetName);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotSomeVizWidgetActivated()
{
    QAction* senderAction = dynamic_cast<QAction*>(sender());
    CVF_ASSERT(senderAction);
    const QString qstrWidgetName = senderAction->data().toString();
    m_activeVizWidgetName = cvfqt::Utils::toString(qstrWidgetName);

    highlightActiveVizWidget();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotCreateTestScene()
{
    if (m_vizWidgetArr.size() == 0)
    {
        return;
    }

    const bool allWidgetsShareSameScene = m_commonContextGroup.notNull() ? true : false;
    populateAllValidVizWidgetsWithTestScene(allWidgetsShareSameScene);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotClearScene()
{
    for (size_t i = 0; i < m_vizWidgetArr.size(); i++)
    {
        QTBVizWidget* vizWidget = m_vizWidgetArr[i];
        CVF_ASSERT(vizWidget);
        vizWidget->setScene(nullptr);
        vizWidget->update();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotSaveLayout()
{
    saveWinGeoAndDockToolBarLayout();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotRestoreLayout()
{
    loadWinGeoAndDockToolBarLayout();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QTBMainWindow::slotGrabNativeHandle()
{
    QTBVizWidget* vizWidget = getActiveVizWidget();
    if (!vizWidget)
    {
        return;
    }

    CVF_ASSERT(vizWidget->isValid());
    cvf::String vizWidgetName = vizWidget->viewTitle();
    cvf::Trace::show("Trying to get native handle for %s by calling QWidget::winId()", vizWidgetName.toAscii().ptr());

    // Calling this seems to wreak havoc for Qt itself.
    // For Qt5 it seems that under certain circumstances it will work, BUT for Qt6 it basically breaks the entire app
    WId theWinId = vizWidget->winId();
    cvf::Trace::show("Got WinId = 0x%x", (cvf::uint64)(theWinId));
}

