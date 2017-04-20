/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA 2016
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

#include "RiuRecentFileActionProvider.h"

#include "RiaApplication.h"

#include <QAction>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuRecentFileActionProvider::RiuRecentFileActionProvider(int maxActionCount)
    : m_maxActionCount(maxActionCount)
{
    createActions();
    updateActions();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuRecentFileActionProvider::~RiuRecentFileActionProvider()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRecentFileActionProvider::addFileName(const QString& fileName)
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > m_maxActionCount)
        files.removeLast();

    settings.setValue("recentFileList", files);

    updateActions();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRecentFileActionProvider::removeFileName(const QString& fileName)
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);

    settings.setValue("recentFileList", files);

    updateActions();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRecentFileActionProvider::updateActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), m_maxActionCount);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        m_recentFileActions[i]->setText(text);
        m_recentFileActions[i]->setData(files[i]);
        m_recentFileActions[i]->setToolTip(files[i]);
        m_recentFileActions[i]->setVisible(true);
    }

    for (int j = numRecentFiles; j < m_maxActionCount; ++j)
        m_recentFileActions[j]->setVisible(false);

    m_separatorAction->setVisible(numRecentFiles > 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QAction*> RiuRecentFileActionProvider::actions() const
{
    std::vector<QAction*> actionItems;
    actionItems.push_back(m_separatorAction);

    for (auto act : m_recentFileActions)
    {
        actionItems.push_back(act);
    }

    return actionItems;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRecentFileActionProvider::slotOpenRecentFile()
{
    QAction* action = qobject_cast<QAction *>(sender());
    if (action)
    {
        QString fileName = action->data().toString();

        RiaApplication* app = RiaApplication::instance();
        if (RiaApplication::hasValidProjectFileExtension(fileName))
        {
            if (!app->askUserToSaveModifiedProject()) return;
        }

        bool loadingSucceded = RiaApplication::instance()->openFile(fileName);
        if (loadingSucceded)
        {
            addFileName(fileName);
        }
        else
        {
            QMessageBox::warning(NULL, "File open", "Failed to import file located at\n" + fileName);

            removeFileName(fileName);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuRecentFileActionProvider::createActions()
{
    for (int i = 0; i < m_maxActionCount; ++i)
    {
        QAction* act = new QAction(this);
        act->setVisible(false);
        connect(act, SIGNAL(triggered()), this, SLOT(slotOpenRecentFile()));
        
        m_recentFileActions.push_back(act);
    }

    m_separatorAction = new QAction(this);
    m_separatorAction->setSeparator(true);
}
