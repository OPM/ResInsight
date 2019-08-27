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


#pragma once

// Needed for moc file
#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfOpenGLContextGroup.h"

#include "cvfuTestSnippet.h"
#include "cvfuSnippetFactory.h"

#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QMainWindow>
#else
#include <QtGui/QMainWindow>
#endif

class QAction;
class QSRSnippetWidget;
class QSRRunPanel;
class QSRPropertiesPanel;


//==================================================================================================
//
// 
//
//==================================================================================================
class QSRMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QSRMainWindow();
    static QSRMainWindow* instance();

    void            executeTestSnippetInNewWidget(const cvf::String& snippetId, cvfu::TestSnippet* snippet);

private:
    void            createActions();
    void            createMenus();
    void            createDockPanels();
    void            closeCurrentSnippet();
	virtual void	closeEvent(QCloseEvent* event);
    void            updateWidowTitleFromCurrentSnippet();

private:
    static QSRMainWindow*               sm_mainWindowInstance;
    std::vector<cvfu::SnippetInfo>      m_availableSnippets;
    cvf::ref<cvf::OpenGLContextGroup>   m_contextGroup;         
    QSRSnippetWidget*                   m_currentSnippetWidget; // Pointer to the currently active snippet widget
    cvf::String                         m_currentSnippetId;     // String ID of the currently active snippet
    QSRRunPanel*                        m_runPanel;
    QSRPropertiesPanel*                 m_propertiesPanel;

    std::vector<QAction*>               m_snippetActions;
    QAction*                            m_activateLastUsedSnippetAction;
    QAction*                            m_closeCurrentSnippetAction;
    QAction*                            m_redrawAction;
    QAction*                            m_multipleRedrawAction;
    QAction*                            m_multipleRedrawManyAction;
    QAction*                            m_showHUDAction;
    QAction*                            m_saveFrameBufferToFileAction;
    QAction*                            m_setPixelSizeCullThresholdAction;
    QAction*                            m_forceImmediateModeRenderingAction;
    QAction*                            m_useVertexArrayRenderingAction;
    QAction*                            m_useVBORenderingAction;
    QAction*                            m_viewFrustumCullingAction;
    QAction*                            m_pixelSizeCullingAction;
    QAction*                            m_itemCountUpdateAction;
    QAction*                            m_renderDrawablesDisabledAction;
    QAction*                            m_applyEffectsDisabledAction;
    QAction*                            m_useTBBAction;
    QAction*                            m_convertDrawableToShortAction;
    QAction*                            m_showModelStatisticsAction;
    QAction*                            m_formatSoftwareAction;
    QAction*                            m_formatMultisampleAction;
    QAction*                            m_showHelpTextAction;

private slots:
    void    slotRunTestSnippet();
    void    slotRunLastUsedSnippet();
    void    slotCloseCurrentSnippet();

    void    slotShowHUD();
    void    slotViewRedraw();
    void    slotViewMultipleRedraw();
    void    slotViewMultipleRedrawMany();
    void    slotSaveFrameBufferToFile();

    void    slotConfigureSetPixelSizeCullThreshold();
    void    slotConfigureForceImmediateModeRendering(bool);
    void    slotConfigureVertexArrayRendering();
    void    slotConfigureVBORendering();
    void    slotConfigureViewFrustumCulling();
    void    slotConfigurePixelSizeCulling();
    void    slotConfigureItemCountUpdate();
    void    slotConfigureRenderDrawablesDisabled();
    void    slotConfigureApplyEffectsDisabled();
    void    slotUseTBB();
    void    slotConfigureConvertToShort();
    void    slotShowModelStatistics();
    
    void    slotUpdateViewMenu();
    void    slotUpdateConfigureMenu();

    void    slotShowHelp();
};

