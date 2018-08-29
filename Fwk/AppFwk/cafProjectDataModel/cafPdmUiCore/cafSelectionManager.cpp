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


#include "cafSelectionManager.h"

#include "cafPdmReferenceHelper.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiObjectHandle.h"



namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SelectionManager* SelectionManager::instance()
{
    static SelectionManager* singleton = new SelectionManager;
    return singleton;
}

//--------------------------------------------------------------------------------------------------
/// Obsolete.  Do not use this method.
//--------------------------------------------------------------------------------------------------
caf::NotificationCenter* SelectionManager::notificationCenter()
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SelectionManager::selectedItems(std::vector<PdmUiItem*>& items, int selectionLevel /*= 0*/)
{
    const auto& levelSelectionPairIt = m_selectionPrLevel.find(selectionLevel);

    if (levelSelectionPairIt == m_selectionPrLevel.end()) return ;

    std::vector< std::pair<PdmPointer<PdmObjectHandle>, PdmUiItem*> >& selection = levelSelectionPairIt->second;

    for (size_t i = 0; i < selection.size(); i++)
    {
        if (selection[i].first.notNull())
        {
            items.push_back(selection[i].second);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SelectionManager::setSelectedItems(const std::vector<PdmUiItem*>& items, int selectionLevel /*= 0*/)
{
    std::vector< std::pair<PdmPointer<PdmObjectHandle>, PdmUiItem*> >& selection = m_selectionPrLevel[selectionLevel];
    std::vector< std::pair<PdmPointer<PdmObjectHandle>, PdmUiItem*> > newSelection;

    for (size_t i = 0; i < items.size(); i++)
    {
        PdmUiFieldHandle* fieldHandle = dynamic_cast<PdmUiFieldHandle*>(items[i]);
        if (fieldHandle)
        {
            PdmObjectHandle* obj = fieldHandle->fieldHandle()->ownerObject();

            newSelection.push_back(std::make_pair(obj, fieldHandle));
        }
        else
        {
            PdmUiObjectHandle* obj = dynamic_cast<PdmUiObjectHandle*>(items[i]);
            if (obj)
            {
                newSelection.push_back(std::make_pair(obj->objectHandle(), obj));
            }
        }
    }

    if (newSelection != selection)
    {
        selection = newSelection;
        notifySelectionChanged(selectionLevel);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmUiItem* SelectionManager::selectedItem(int selectionLevel /*= 0*/)
{
    const auto& levelSelectionPairIt = m_selectionPrLevel.find(selectionLevel);
    if (levelSelectionPairIt == m_selectionPrLevel.end()) return nullptr;
     
    std::vector< std::pair<PdmPointer<PdmObjectHandle>, PdmUiItem*> >& selection = levelSelectionPairIt->second;

    if (selection.size() == 1)
    {
        if (selection[0].first.notNull())
        {
            return selection[0].second;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SelectionManager::setSelectedItem(PdmUiItem* item, int selectionLevel /*= 0*/)
{
    std::vector<PdmUiItem*> singleSelection;
    singleSelection.push_back(item);

    setSelectedItems(singleSelection, selectionLevel);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SelectionManager::SelectionManager()
{
    m_activeChildArrayFieldHandle = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SelectionManager::selectionAsReferences(std::vector<QString>& referenceList, int selectionLevel /*= 0*/) const
{
    const auto& levelSelectionPairIt = m_selectionPrLevel.find(selectionLevel);

    if (levelSelectionPairIt == m_selectionPrLevel.end()) return;

    const std::vector< std::pair<PdmPointer<PdmObjectHandle>, PdmUiItem*> >& selection = levelSelectionPairIt->second;

    for (size_t i = 0; i < selection.size(); i++)
    {
        if (!selection[i].first.isNull())
        {
            PdmUiObjectHandle* pdmObj = dynamic_cast<PdmUiObjectHandle*>(selection[i].second);
            if (pdmObj)
            {
                QString itemRef = PdmReferenceHelper::referenceFromRootToObject(m_rootObject, pdmObj->objectHandle());

                referenceList.push_back(itemRef);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SelectionManager::setSelectionFromReferences(const std::vector<QString>& referenceList, int selectionLevel /*= 0*/)
{
    std::vector<PdmUiItem*> uiItems;

    for (size_t i = 0; i < referenceList.size(); i++)
    {
        QString reference = referenceList[i];

        PdmObjectHandle* pdmObj = PdmReferenceHelper::objectFromReference(m_rootObject, reference);
        if (pdmObj)
        {
            caf::PdmUiObjectHandle* uiObject = uiObj(pdmObj);
            if (uiObject)
            {
                uiItems.push_back(uiObject);
            }
        }
    }

    setSelectedItems(uiItems, selectionLevel);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool SelectionManager::isSelected(PdmUiItem* item, int selectionLevel) const
{
    auto levelIter = m_selectionPrLevel.find(selectionLevel);
    
    if (levelIter == m_selectionPrLevel.end()) return false;

    const std::vector< std::pair<PdmPointer<PdmObjectHandle>, PdmUiItem*> >& selection = levelIter->second;

    auto iter = selection.begin();
    while ( iter != selection.end() )
    {
        if ( iter->second == item )
        {
            return true;
        }
        else
        {
            ++iter;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SelectionManager::clearAll()
{
    std::set<int> changedSelectionLevels;

    for ( auto & levelSelectionPair :  m_selectionPrLevel)
    {
        if ( !levelSelectionPair.second.empty())
        {
            levelSelectionPair.second.clear();
            changedSelectionLevels.insert(levelSelectionPair.first);
        }
    }
    
    m_selectionPrLevel.clear();

    for (int level: changedSelectionLevels)
    {
        notifySelectionChanged(level); 
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SelectionManager::clear(int selectionLevel)
{
    const auto& levelSelectionPairIt = m_selectionPrLevel.find(selectionLevel);

    if (levelSelectionPairIt == m_selectionPrLevel.end()) return;

    if ( !levelSelectionPairIt->second.empty() )
    {
        m_selectionPrLevel[selectionLevel].clear();

        notifySelectionChanged(selectionLevel);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SelectionManager::notifySelectionChanged( int selectionLevel)
{
    for (auto receiver: m_selectionReceivers)
    {
        receiver->onSelectionManagerSelectionChanged(selectionLevel);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SelectionManager::removeObjectFromAllSelections(PdmObjectHandle* pdmObject)
{
    std::set<int> changedSelectionLevels;

    for ( auto & levelSelectionPair :  m_selectionPrLevel)
    {
        std::vector< std::pair<PdmPointer<PdmObjectHandle>, PdmUiItem*> >& selection = levelSelectionPair.second;

        std::vector< std::pair<PdmPointer<PdmObjectHandle>, PdmUiItem*> >::iterator iter = selection.begin();
        while (iter != selection.end())
        {
            if (iter->first.notNull())
            {
                PdmObjectHandle* selectionObj = dynamic_cast<PdmObjectHandle*>(iter->second);
                if (selectionObj == pdmObject)
                {
                    iter = selection.erase(iter);

                    changedSelectionLevels.insert(levelSelectionPair.first);
                }
                else
                {
                    ++iter;
                }
            }
            else
            {
                ++iter;
            }
        }
    }

    for (int level: changedSelectionLevels)
    {
        notifySelectionChanged(level); 
    }
}

void SelectionManager::setPdmRootObject(PdmObjectHandle* root)
{
    m_rootObject = root;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SelectionManager::setActiveChildArrayFieldHandle(PdmChildArrayFieldHandle* childArray)
{
    m_activeChildArrayFieldHandle = childArray;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmChildArrayFieldHandle* SelectionManager::activeChildArrayFieldHandle()
{
    return m_activeChildArrayFieldHandle;
}

} // end namespace caf
