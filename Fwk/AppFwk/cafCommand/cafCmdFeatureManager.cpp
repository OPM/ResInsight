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


#include "cafCmdFeatureManager.h"

#include "cafCmdFeature.h"
#include "cafCmdSelectionHelper.h"
#include "cafFactory.h" 

#include "defaultfeatures/cafCmdDeleteItemFeature.h"
#include "defaultfeatures/cafCmdAddItemFeature.h"

#include <QAction>

#include <assert.h>

namespace caf
{

    typedef Factory<CmdFeature, std::string> CommandFeatureFactory;
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CmdFeatureManager::CmdFeatureManager()
{
    CmdDeleteItemFeature::idNameStatic();
    CmdAddItemFeature::idNameStatic();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CmdFeatureManager::~CmdFeatureManager()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CmdFeatureManager* CmdFeatureManager::instance()
{
    static CmdFeatureManager* singleton = new CmdFeatureManager;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
/// Get action for the specified command.
/// The action is owned by the PdmCommandItemManager
//--------------------------------------------------------------------------------------------------
QAction* CmdFeatureManager::action(const QString& commandId)
{
    std::pair<CmdFeature*, size_t> featurePair = createFeature(commandId.toStdString());

    QAction* act = featurePair.first->action();
    m_actionToFeatureIdxMap[act] = featurePair.second;

    return act;
}

//--------------------------------------------------------------------------------------------------
/// Get action for the specified command, with custom action text 
/// The action is owned by the PdmCommandItemManager
//--------------------------------------------------------------------------------------------------
QAction* CmdFeatureManager::action(const QString& commandId, const QString& customActionText)
{
    std::pair<CmdFeature*, size_t> featurePair = createFeature(commandId.toStdString());

    QAction* act = featurePair.first->action(customActionText);
    m_actionToFeatureIdxMap[act] = featurePair.second;

    return act;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<CmdFeature*, size_t> CmdFeatureManager::createFeature(const std::string& commandId)
{
    std::pair<CmdFeature*, size_t> featurePair = this->findExistingCmdFeature(commandId);

    if (featurePair.first)
    {
        return featurePair;
    }
    
    CmdFeature* feature = CommandFeatureFactory::instance()->create(commandId);
    assert(feature); // The command ID is not known in the factory

    feature->setParent(this);
    size_t index = m_commandFeatures.size();
    m_commandFeatures.push_back(feature);
    m_commandIdToFeatureIdxMap[commandId] = index;

    return std::make_pair(feature, index);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::pair<CmdFeature*, size_t>  CmdFeatureManager::findExistingCmdFeature(const std::string& commandId)
{
    std::map<std::string, size_t>::const_iterator it;
    it = m_commandIdToFeatureIdxMap.find(commandId);

    if (it != m_commandIdToFeatureIdxMap.end())
    {
        size_t itemIndex = it->second;

        CmdFeature* item = m_commandFeatures[itemIndex];
        item->refreshEnabledState();

        return std::make_pair(item, itemIndex);
    }
    else
    {
        return std::make_pair(static_cast<CmdFeature*>(NULL), -1);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdFeatureManager::refreshStates(const QStringList& commandIdList)
{
    if (commandIdList.size() == 0)
    {
        for (size_t i = 0; i < m_commandFeatures.size(); i++)
        {
            CmdFeature* cmdFeature = m_commandFeatures[i];

            if (cmdFeature)
            {
                cmdFeature->refreshEnabledState();
                cmdFeature->refreshCheckedState();
            }
        }
    }
    else
    {
        for (int i = 0; i < commandIdList.size(); i++)
        {
            std::pair<CmdFeature*, size_t>  featurePair = findExistingCmdFeature(commandIdList.at(i).toStdString());

            if (featurePair.first)
            {
                featurePair.first->refreshEnabledState();
                featurePair.first->refreshCheckedState();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdFeatureManager::refreshEnabledState(const QStringList& commandIdList)
{
    if (commandIdList.size() == 0)
    {
        for (size_t i = 0; i < m_commandFeatures.size(); i++)
        {
            m_commandFeatures[i]->refreshEnabledState();
        }
    }
    else
    {
        for (int i = 0; i < commandIdList.size(); i++)
        {
            std::pair<CmdFeature*, size_t>  featurePair = findExistingCmdFeature(commandIdList.at(i).toStdString());

            if (featurePair.first)
            {
                featurePair.first->refreshEnabledState();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void CmdFeatureManager::refreshCheckedState(const QStringList& commandIdList)
{
    if (commandIdList.size() == 0)
    {
        for (size_t i = 0; i < m_commandFeatures.size(); i++)
        {
            m_commandFeatures[i]->refreshCheckedState();
        }
    }
    else
    {
        for (int i = 0; i < commandIdList.size(); i++)
        {
            std::pair<CmdFeature*, size_t>  featurePair = findExistingCmdFeature(commandIdList.at(i).toStdString());

            if (featurePair.first)
            {
                featurePair.first->refreshCheckedState();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
CmdFeature* CmdFeatureManager::getCommandFeature(const std::string& commandId)
{
    std::pair<CmdFeature*, size_t> featurePair = createFeature(commandId);

    return featurePair.first;

}


} // end namespace caf
