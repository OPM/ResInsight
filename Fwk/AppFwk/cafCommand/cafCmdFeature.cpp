//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2014 Ceetron Solutions AS
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

#include "cafCmdFeature.h"

#include "cafCmdExecCommandManager.h"
#include "cafCmdFeatureManager.h"

#include "cafPdmUiModelChangeDetector.h"

#include <QAction>


namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CmdFeature::CmdFeature()
    : m_triggerModelChange(true)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CmdFeature::~CmdFeature()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QAction* CmdFeature::action()
{
    return this->action(QString(""));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QAction* CmdFeature::action(QString customText)
{
    QAction* action = NULL;

    std::map<QString, QAction*>::iterator it;
    it = m_customTextToActionMap.find(customText);

    if (it != m_customTextToActionMap.end() && it->second != NULL)
    {
        action = it->second;
    }
    else
    {
        action = new QAction(this);
        connect(action, SIGNAL(triggered(bool)), SLOT(actionTriggered(bool)));
        m_customTextToActionMap[customText]= action;
    }

    this->setupActionLook(action);
    if (!customText.isEmpty())
    {
        action->setText(customText);
    }

    return action;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdFeature::refreshEnabledState()
{
    std::map<QString, QAction*>::iterator it;
    bool isEnabled = this->isCommandEnabled();

    for (it = m_customTextToActionMap.begin(); it != m_customTextToActionMap.end(); ++it)
    {
        it->second->setEnabled(isEnabled);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdFeature::refreshCheckedState()
{
    std::map<QString, QAction*>::iterator it;
    bool isChecked = this->isCommandChecked();

    for (it = m_customTextToActionMap.begin(); it != m_customTextToActionMap.end(); ++it)
    {
        QAction* act = it->second;
        if (act->isCheckable())
        {
            it->second->setChecked(isChecked);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CmdFeature::canFeatureBeExecuted()
{
    return this->isCommandEnabled();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdFeature::actionTriggered(bool isChecked)
{
    this->onActionTriggered(isChecked);

    if (m_triggerModelChange)
    {
        caf::PdmUiModelChangeDetector::instance()->setModelChanged();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool CmdFeature::isCommandChecked()
{
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdFeature::disableModelChangeContribution()
{
    m_triggerModelChange = false;
}

} // end namespace caf
