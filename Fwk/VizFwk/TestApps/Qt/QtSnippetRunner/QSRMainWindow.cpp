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


#include "QSRStdInclude.h"
#include "QSRMainWindow.h"
#include "QSRSnippetWidget.h"
#include "QSRRunPanel.h"
#include "QSRPropertiesPanel.h"

#include "cvfqtUtils.h"

#include <QAction>
#include <QDockWidget>
#include <QFrame>
#include <QKeyEvent>
#include <QInputDialog>
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>

using cvfu::TestSnippet;
using cvfu::SnippetInfo;
using cvfu::SnippetRegistry;



//==================================================================================================
//
// QSRMainWindow
//
//==================================================================================================

QSRMainWindow* QSRMainWindow::sm_mainWindowInstance = NULL;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSRMainWindow::QSRMainWindow()
{
    // Registry already initialized in main
	m_availableSnippets = SnippetRegistry::instance()->availableSnippets(cvfu::SC_ALL);

    m_currentSnippetWidget = NULL;
    m_currentSnippetId = "";
    m_runPanel = NULL;
    m_propertiesPanel = NULL;

    QFrame* mainFrame = new QFrame;
    QGridLayout* frameLayout = new QGridLayout;
    mainFrame->setLayout(frameLayout);
    setCentralWidget(mainFrame);

    createActions();
    createMenus();
    createDockPanels();

    updateWidowTitleFromCurrentSnippet();

    m_contextGroup = new cvf::OpenGLContextGroup;

    sm_mainWindowInstance = this;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QSRMainWindow* QSRMainWindow::instance()
{
    CVF_ASSERT(sm_mainWindowInstance);
    return sm_mainWindowInstance;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::createActions()
{
    // Snippet menu
    size_t numSnippets = m_availableSnippets.size();
    size_t i;
    for (i = 0; i < numSnippets; i++)
    {
        SnippetInfo si = m_availableSnippets[i];
        String entry = String("%1\t(%2)").arg(si.id).arg(si.name);
        QAction* act = new QAction(cvfqt::Utils::toQString(entry), this);
        
        connect(act, SIGNAL(triggered()), SLOT(slotRunTestSnippet()));
        m_snippetActions.push_back(act);
    }

    m_activateLastUsedSnippetAction = new QAction("Load last used snippet", this);
    m_activateLastUsedSnippetAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
    connect(m_activateLastUsedSnippetAction, SIGNAL(triggered()), SLOT(slotRunLastUsedSnippet()));

    m_closeCurrentSnippetAction = new QAction("Close Current Snippet", this);
    connect(m_closeCurrentSnippetAction, SIGNAL(triggered()), SLOT(slotCloseCurrentSnippet()));

    // View menu
    m_showHUDAction = new QAction("Show HUD", this);
    m_showHUDAction->setCheckable(true);
    m_showHUDAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    connect(m_showHUDAction, SIGNAL(triggered()), SLOT(slotShowHUD()));

    m_redrawAction = new QAction("Redraw view", this);
    m_redrawAction ->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    connect(m_redrawAction, SIGNAL(triggered()), SLOT(slotViewRedraw()));

    m_multipleRedrawAction = new QAction("Redraw 10 times", this);
    m_multipleRedrawAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
    connect(m_multipleRedrawAction, SIGNAL(triggered()), SLOT(slotViewMultipleRedraw()));

    m_multipleRedrawManyAction = new QAction("Redraw 50 times", this);
    m_multipleRedrawManyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_M));
    connect(m_multipleRedrawManyAction, SIGNAL(triggered()), SLOT(slotViewMultipleRedrawMany()));


    m_saveFrameBufferToFileAction = new QAction("Save FrameBuffer to File", this);
    connect(m_saveFrameBufferToFileAction, SIGNAL(triggered()), SLOT(slotSaveFrameBufferToFile()));

    // Configure menu
    m_setPixelSizeCullThresholdAction = new QAction("Change Pixel Size Culling Threshold", this);
    connect(m_setPixelSizeCullThresholdAction, SIGNAL(triggered()), SLOT(slotConfigureSetPixelSizeCullThreshold()));

    m_forceImmediateModeRenderingAction = new QAction("Force Immediate Mode (glBegin) Rendering", this);
    m_forceImmediateModeRenderingAction->setCheckable(true);
    connect(m_forceImmediateModeRenderingAction, SIGNAL(toggled(bool)), SLOT(slotConfigureForceImmediateModeRendering(bool)));

    m_useVertexArrayRenderingAction = new QAction("Vertex Array Rendering", this);
    m_useVertexArrayRenderingAction->setCheckable(true);
    connect(m_useVertexArrayRenderingAction, SIGNAL(triggered()), SLOT(slotConfigureVertexArrayRendering()));

    m_useVBORenderingAction = new QAction("Vertex Buffer Objects (VBO) Rendering", this);
    m_useVBORenderingAction->setCheckable(true);
    connect(m_useVBORenderingAction, SIGNAL(triggered()), SLOT(slotConfigureVBORendering()));

    m_viewFrustumCullingAction = new QAction("View Frustum Culling", this);
    m_viewFrustumCullingAction->setCheckable(true);
    connect(m_viewFrustumCullingAction, SIGNAL(triggered()), SLOT(slotConfigureViewFrustumCulling()));

    m_pixelSizeCullingAction = new QAction("Pixel Size Culling", this);
    m_pixelSizeCullingAction->setCheckable(true);
    connect(m_pixelSizeCullingAction, SIGNAL(triggered()), SLOT(slotConfigurePixelSizeCulling()));

    m_itemCountUpdateAction = new QAction("Item Count update", this);
    m_itemCountUpdateAction->setCheckable(true);
    connect(m_itemCountUpdateAction, SIGNAL(triggered()), SLOT(slotConfigureItemCountUpdate()));

    m_renderDrawablesDisabledAction = new QAction("Disable Render Drawables", this);
    m_renderDrawablesDisabledAction->setCheckable(true);
    connect(m_renderDrawablesDisabledAction, SIGNAL(triggered()), SLOT(slotConfigureRenderDrawablesDisabled()));

    m_applyEffectsDisabledAction = new QAction("Disable Apply Effects", this);
    m_applyEffectsDisabledAction->setCheckable(true);
    connect(m_applyEffectsDisabledAction, SIGNAL(triggered()), SLOT(slotConfigureApplyEffectsDisabled()));

    m_useTBBAction = new QAction("Use TBB", this);
    m_useTBBAction->setCheckable(true);
    connect(m_useTBBAction, SIGNAL(triggered()), SLOT(slotUseTBB()));

    m_convertDrawableToShortAction = new QAction("Convert primitives to unsigned short", this);
    connect(m_convertDrawableToShortAction, SIGNAL(triggered()), SLOT(slotConfigureConvertToShort()));

    m_showModelStatisticsAction = new QAction("Show model statistics", this);
    connect(m_showModelStatisticsAction, SIGNAL(triggered()), SLOT(slotShowModelStatistics()));

    // Format menu
    m_formatSoftwareAction = new QAction("Software", this);
    m_formatSoftwareAction->setCheckable(true);
    m_formatMultisampleAction = new QAction("Multisample", this);
    m_formatMultisampleAction->setCheckable(true);
    
    // Help menu
    m_showHelpTextAction = new QAction("Show Snippet Help...", this);
    connect(m_showHelpTextAction, SIGNAL(triggered()), SLOT(slotShowHelp()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::createMenus()
{
    QMenu* snippetsMenu = menuBar()->addMenu("&Snippets");

    size_t numActions = m_snippetActions.size();
    size_t i;
    for (i = 0; i < numActions; i++)
    {
        snippetsMenu->addAction(m_snippetActions[i]);
    }

    snippetsMenu->addSeparator();
    snippetsMenu->addAction(m_activateLastUsedSnippetAction);
    snippetsMenu->addSeparator();
    snippetsMenu->addAction(m_closeCurrentSnippetAction);

    QMenu* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(m_showHUDAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_redrawAction);
    viewMenu->addAction(m_multipleRedrawAction);
    viewMenu->addAction(m_multipleRedrawManyAction);
    viewMenu->addSeparator();
    viewMenu->addAction(m_saveFrameBufferToFileAction);
    connect(viewMenu, SIGNAL(aboutToShow()), SLOT(slotUpdateViewMenu()));

    QMenu* configureMenu = menuBar()->addMenu("&Configure");
    configureMenu->addAction(m_forceImmediateModeRenderingAction);
    configureMenu->addAction(m_useVertexArrayRenderingAction);
    configureMenu->addAction(m_useVBORenderingAction);
    configureMenu->addSeparator();
    configureMenu->addAction(m_setPixelSizeCullThresholdAction);
    configureMenu->addSeparator();
    configureMenu->addAction(m_viewFrustumCullingAction);
    configureMenu->addAction(m_pixelSizeCullingAction);
    configureMenu->addSeparator();
    configureMenu->addAction(m_itemCountUpdateAction);
    configureMenu->addSeparator();
    configureMenu->addAction(m_renderDrawablesDisabledAction);
    configureMenu->addAction(m_applyEffectsDisabledAction);
    configureMenu->addSeparator();
    configureMenu->addAction(m_useTBBAction);
    configureMenu->addSeparator();
    configureMenu->addAction(m_convertDrawableToShortAction);
    configureMenu->addAction(m_showModelStatisticsAction);

    connect(configureMenu, SIGNAL(aboutToShow()), SLOT(slotUpdateConfigureMenu()));

    QMenu* formatMenu = menuBar()->addMenu("F&ormat");
    formatMenu->addAction(m_formatSoftwareAction);
    formatMenu->addAction(m_formatMultisampleAction);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(m_showHelpTextAction);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::createDockPanels()
{
    {
        QDockWidget* dockPropertiesPanel = new QDockWidget("Properties", this);
        dockPropertiesPanel->setObjectName("dockPropertiesPanel");
        dockPropertiesPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        m_propertiesPanel = new QSRPropertiesPanel(dockPropertiesPanel);
        dockPropertiesPanel->setWidget(m_propertiesPanel);

        QDockWidget* dockRunPanel = new QDockWidget("Run", this);
        dockRunPanel->setObjectName("dockRunPanel");
        dockRunPanel->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        m_runPanel = new QSRRunPanel(dockRunPanel);
        dockRunPanel->setWidget(m_runPanel);

        addDockWidget(Qt::RightDockWidgetArea, dockPropertiesPanel);
        addDockWidget(Qt::RightDockWidgetArea, dockRunPanel);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::executeTestSnippetInNewWidget(const cvf::String& snippetId, TestSnippet* snippet)
{
    closeCurrentSnippet();

    QGLFormat glFormat;

    glFormat.setDirectRendering(!m_formatSoftwareAction->isChecked());
    glFormat.setSampleBuffers(m_formatMultisampleAction->isChecked());

    // To get software renderer
    //glFormat.setDirectRendering(false);

    // Force a Core (forward) profile not allowing deprecated stuff
    //glFormat.setVersion(3,3);
    //glFormat.setProfile(QGLFormat::CoreProfile);
    //glFormat.setProfile(QGLFormat::CompatibilityProfile);

    // For FSAA, use with glEnable(GL_MULTISAMPLE);
    //glFormat.setSampleBuffers(true);

    QWidget* parentWidget = centralWidget();
    CVF_ASSERT(m_contextGroup.notNull());
    m_currentSnippetWidget = new QSRSnippetWidget(snippet, m_contextGroup.p(), glFormat, parentWidget);
    m_currentSnippetWidget->setFocus();

    if (m_formatMultisampleAction->isChecked())
    {
        m_currentSnippetWidget->enableMultisampleWhenDrawing(true);
    }

    m_currentSnippetId = snippetId;

    QLayout* layout = parentWidget->layout();
    layout->addWidget(m_currentSnippetWidget);

    if (m_propertiesPanel)
    {
        m_propertiesPanel->connectToSnippet(m_currentSnippetWidget);
    }

    updateWidowTitleFromCurrentSnippet();

    // Store ID of this 'last run' snippet in registry
    QSettings settings("Ceetron", "SnippetRunner");
    settings.setValue("LastUsedSnippetID", snippetId.toAscii().ptr());

	repaint();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::closeCurrentSnippet()
{
    if (m_propertiesPanel)
    {
        m_propertiesPanel->disconnectFromSnippet();
    }

    if (m_currentSnippetWidget)
    {
        QWidget* parentWidget = centralWidget();
        QLayout* layout = parentWidget->layout();
        layout->removeWidget(m_currentSnippetWidget);

        delete m_currentSnippetWidget;
        m_currentSnippetWidget = NULL;
    }

    CVF_ASSERT(m_contextGroup.notNull());
    CVF_ASSERT(m_contextGroup->contextCount() == 0);
    CVF_ASSERT(!m_contextGroup->resourceManager()->hasAnyOpenGLResources());

    m_currentSnippetId = "";

    updateWidowTitleFromCurrentSnippet();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::closeEvent(QCloseEvent* event)
{
    // Close snippet to clean up resources before we go
    closeCurrentSnippet();

    event->accept();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::updateWidowTitleFromCurrentSnippet()
{
    cvf::String snipText("No Snippet");

    // Locate snippet info based on current snippet id
    size_t i;
    for (i = 0; i < m_availableSnippets.size(); i++)
    {
        const SnippetInfo& si = m_availableSnippets[i];
        if (si.id == m_currentSnippetId)
        {
            snipText = cvf::String("%1 (%2)").arg(si.id).arg(si.name);
        }
    }

    QString platform = cvf::System::is64Bit() ? "(64bit)" : "(32bit)";
    QString appName = QString("Qt Snippet Runner ") + platform + QString(" - ");
    QString title = appName + cvfqt::Utils::toQString(snipText);

    setWindowTitle(title);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotRunTestSnippet()
{
    QAction* act = dynamic_cast<QAction*>(sender());

    size_t numActions = m_snippetActions.size();
    size_t i;
    for (i = 0; i < numActions; i++)
    {
        if (m_snippetActions[i] == act)
        {
            SnippetInfo si = m_availableSnippets[i];

            ref<TestSnippet> snippet = SnippetRegistry::instance()->createSnippet(si.id);
            executeTestSnippetInNewWidget(si.id, snippet.p());
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotRunLastUsedSnippet()
{
	QSettings settings("Ceetron", "SnippetRunner");

	cvf::String lastUsedSnippetId = cvfqt::Utils::toString(settings.value("LastUsedSnippetID").toString());

	// If no last used ID is found, use first snippet in list
	if (lastUsedSnippetId.isEmpty() && m_availableSnippets.size() > 0)
	{
		SnippetInfo si = m_availableSnippets[0];
		lastUsedSnippetId = si.id;
	}

    size_t i;
    for (i = 0; i < m_availableSnippets.size(); i++)
    {
        SnippetInfo si = m_availableSnippets[i];
        if (si.id == lastUsedSnippetId)
        {
            ref<TestSnippet> snippet = SnippetRegistry::instance()->createSnippet(si.id);
            executeTestSnippetInNewWidget(si.id, snippet.p());
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotCloseCurrentSnippet()
{
    closeCurrentSnippet();
    repaint();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotShowHUD()
{
    if (!m_currentSnippetWidget) return;

    m_currentSnippetWidget->enablePerfInfoHUD(!m_currentSnippetWidget->isPerfInfoHudEnabled());
    m_currentSnippetWidget->update();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotViewRedraw()
{
    if (!m_currentSnippetWidget) return;

    m_currentSnippetWidget->update();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotViewMultipleRedraw()
{
    if (!m_currentSnippetWidget) return;

    int i;
    for (i = 0; i < 10; i++)
    {
        m_currentSnippetWidget->repaint();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotViewMultipleRedrawMany()
{
    if (!m_currentSnippetWidget) return;

    int i;
    for (i = 0; i < 50; i++)
    {
        m_currentSnippetWidget->repaint();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotSaveFrameBufferToFile()
{
    if (!m_currentSnippetWidget) 
    {
        cvf::Trace::show("No current widget");
        return;
    }
    
    QImage img = m_currentSnippetWidget->grabFrameBuffer();

    QDateTime now = QDateTime::currentDateTime();
    QString fileName = QString("SR_Image_%1.png").arg(now.toString("yyyyMMddThhmmss"));
    
    if (img.save(fileName))
    {
        cvf::Trace::show("Image saved to: %s", (const char*)fileName.toLatin1());
    }
    else
    {
        cvf::Trace::show("FAILED to saved image: %s", (const char*)fileName.toLatin1());
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotConfigureForceImmediateModeRendering(bool force)
{
    if (!m_currentSnippetWidget) return;

    m_currentSnippetWidget->enableForcedImmediateMode(force);
    m_currentSnippetWidget->update();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotConfigureVertexArrayRendering()
{
    if (!m_currentSnippetWidget) return;

    m_currentSnippetWidget->setRenderMode(DrawableGeo::VERTEX_ARRAY);
    m_currentSnippetWidget->update();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotConfigureVBORendering()
{
    if (!m_currentSnippetWidget) return;

    m_currentSnippetWidget->setRenderMode(DrawableGeo::BUFFER_OBJECT);
    m_currentSnippetWidget->update();
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotConfigureSetPixelSizeCullThreshold()
{
    if (!m_currentSnippetWidget) return;

    QInputDialog dlg;
    m_currentSnippetWidget->setPixelSizeCullingThreshold(dlg.getDouble(this, "Pixel Size Culling Settings", "Threshold area in pixels:", m_currentSnippetWidget->pixelSizeCullingThreshold()));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotConfigureViewFrustumCulling()
{
    if (!m_currentSnippetWidget) return;

    m_currentSnippetWidget->enableViewFrustumCulling(!m_currentSnippetWidget->isViewFrustumCullingEnabled());
    m_currentSnippetWidget->update();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotConfigurePixelSizeCulling()
{
    if (!m_currentSnippetWidget) return;

    m_currentSnippetWidget->enablePixelSizeCulling(!m_currentSnippetWidget->isPixelSizeCullingEnabled());
    m_currentSnippetWidget->update();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotConfigureItemCountUpdate()
{
    if (!m_currentSnippetWidget) return;

    m_currentSnippetWidget->enableItemCountUpdates(!m_currentSnippetWidget->isItemCountUpdatesEnabled());
    m_currentSnippetWidget->update();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotConfigureRenderDrawablesDisabled()
{
    if (!m_currentSnippetWidget) return;

    m_currentSnippetWidget->setRenderDrawablesDisabled(!m_currentSnippetWidget->isRenderDrawablesDisabled());
    m_currentSnippetWidget->update();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotConfigureApplyEffectsDisabled()
{
    if (!m_currentSnippetWidget) return;

    m_currentSnippetWidget->setApplyEffectsDisabled(!m_currentSnippetWidget->isApplyEffectsDisabled());
    m_currentSnippetWidget->update();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotUseTBB()
{
    cvf::TBBControl::enable(m_useTBBAction->isChecked());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotConfigureConvertToShort()
{
    if (!m_currentSnippetWidget) return;

    m_currentSnippetWidget->convertDrawablesToShort();
    m_currentSnippetWidget->update();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotShowModelStatistics()
{
    if (!m_currentSnippetWidget) return;

    m_currentSnippetWidget->showModelStatistics();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotShowHelp()
{
    QString dlgTitle = "No snippet";
    QString helpText = "N/A";
    QString oglInfo = "";


    if (m_currentSnippetWidget && m_currentSnippetId != "")
    {
        // The dialog title
        size_t i;
        for (i = 0; i < m_availableSnippets.size(); i++)
        {
            const SnippetInfo& si = m_availableSnippets[i];
            if (si.id == m_currentSnippetId)
            {
                dlgTitle = QString("%1 (%2)").arg(cvfqt::Utils::toQString(si.id)).arg(cvfqt::Utils::toQString(si.name));
            }
        }

        // Query snippet for help text
        {
            cvfu::TestSnippet* snippet = m_currentSnippetWidget->snippet();
            CVF_ASSERT(snippet);
            
            std::vector<String> ceeHelpTextEntries = snippet->helpText();
            QStringList qtHelpTextEntries;
            for (i = 0; i < ceeHelpTextEntries.size(); i++)
            {
                qtHelpTextEntries.append(cvfqt::Utils::toQString(ceeHelpTextEntries[i]));
            }

            helpText = qtHelpTextEntries.join("\n");
        }

        // OpenGL info
        {
            oglInfo  = QString("OpenGL info:");
            oglInfo += QString("\nversion:\t") + reinterpret_cast<const char*>(glGetString(GL_VERSION));
            oglInfo += QString("\nrenderer:\t") + reinterpret_cast<const char*>(glGetString(GL_RENDERER));
            oglInfo += QString("\nvendor:\t") + reinterpret_cast<const char*>(glGetString(GL_VENDOR));
            oglInfo += QString("\nglsl ver.:\t") + reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
        }

        {
            oglInfo += "\n\nReported by Qt:";
            
            QGLFormat currrentFormat = m_currentSnippetWidget->format();

#if QT_VERSION >= 0x040700
            oglInfo += QString("\nOpenGL version:\t%1.%2").arg(currrentFormat.majorVersion()).arg(currrentFormat.minorVersion());

            switch (currrentFormat.profile())
            {
                case QGLFormat::NoProfile:              oglInfo += "\nProfile:\t\tNoProfile (GLver < 3.3)"; break;
                case QGLFormat::CoreProfile:            oglInfo += "\nProfile:\t\tCoreProfile"; break;
                case QGLFormat::CompatibilityProfile:   oglInfo += "\nProfile:\t\tCompatibilityProfile"; break;
            }
#endif

            oglInfo += QString("\nColor buffer size:\t<%1 %2 %3 %4>").arg(currrentFormat.redBufferSize()).arg(currrentFormat.greenBufferSize()).arg(currrentFormat.blueBufferSize()).arg(currrentFormat.alphaBufferSize());
            oglInfo += QString("\nDepth buffer size:\t%1").arg(currrentFormat.depthBufferSize());
        }

    }

    QMessageBox dlg(this);
    dlg.setStandardButtons(QMessageBox::Ok);
    dlg.setWindowTitle(dlgTitle);
    dlg.setText(helpText);
    dlg.setInformativeText(oglInfo);
    
    dlg.exec();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotUpdateConfigureMenu()
{
    if (!m_currentSnippetWidget) return;

    m_forceImmediateModeRenderingAction->blockSignals(true);
    m_useVertexArrayRenderingAction->blockSignals(true);
    m_useVBORenderingAction->blockSignals(true);
    m_viewFrustumCullingAction->blockSignals(true);
    m_pixelSizeCullingAction->blockSignals(true);
    m_itemCountUpdateAction->blockSignals(true);
    m_renderDrawablesDisabledAction->blockSignals(true);
    m_applyEffectsDisabledAction->blockSignals(true);
    m_useTBBAction->blockSignals(true);

    m_forceImmediateModeRenderingAction->setChecked(m_currentSnippetWidget->isForcedImmediateModeEnabled());
    m_useVertexArrayRenderingAction->setChecked(m_currentSnippetWidget->renderMode() == DrawableGeo::VERTEX_ARRAY);
    m_useVBORenderingAction->setChecked(m_currentSnippetWidget->renderMode() == DrawableGeo::BUFFER_OBJECT);
    m_viewFrustumCullingAction->setChecked(m_currentSnippetWidget->isViewFrustumCullingEnabled());
    m_pixelSizeCullingAction->setChecked(m_currentSnippetWidget->isPixelSizeCullingEnabled());
    m_itemCountUpdateAction->setChecked(m_currentSnippetWidget->isItemCountUpdatesEnabled());
    m_renderDrawablesDisabledAction->setChecked(m_currentSnippetWidget->isRenderDrawablesDisabled());
    m_applyEffectsDisabledAction->setChecked(m_currentSnippetWidget->isApplyEffectsDisabled());
    m_useTBBAction->setChecked(cvf::TBBControl::isEnabled());

    m_forceImmediateModeRenderingAction->blockSignals(false);
    m_useVertexArrayRenderingAction->blockSignals(false);
    m_useVBORenderingAction->blockSignals(false);
    m_viewFrustumCullingAction->blockSignals(false);
    m_pixelSizeCullingAction->blockSignals(false);
    m_itemCountUpdateAction->blockSignals(false);
    m_renderDrawablesDisabledAction->blockSignals(false);
    m_applyEffectsDisabledAction->blockSignals(false);
    m_useTBBAction->blockSignals(false);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void QSRMainWindow::slotUpdateViewMenu()
{
    if (!m_currentSnippetWidget) return;

    m_showHUDAction->blockSignals(true);

    m_showHUDAction->setChecked(m_currentSnippetWidget->isPerfInfoHudEnabled());

    m_showHUDAction->blockSignals(false);
}


// -------------------------------------------------------
#ifndef CVF_USING_CMAKE
#include "qt-generated/moc_QSRMainWindow.cpp"
#endif
