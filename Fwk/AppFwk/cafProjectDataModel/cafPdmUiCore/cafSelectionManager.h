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

#include "cafSelectionChangedReceiver.h"

#include "cafPdmPointer.h"
#include "cafPdmField.h"

#include <QString>
#include <vector>
#include <set>

namespace caf 
{

class PdmUiItem;
class NotificationCenter;
class PdmChildArrayFieldHandle;

//==================================================================================================
/// 
//==================================================================================================
class SelectionManager
{
public:
    enum SelectionLevel 
    {                       
        UNDEFINED = -1,
        BASE_LEVEL = 0, 
        FIRST_LEVEL = 1, 
        SECOND_LEVEL = 2
    };

public:
    static SelectionManager*  instance();

    // OBSOLETE ! Remove when time to refactor the command system 
    NotificationCenter*       notificationCenter();

    void                      setActiveChildArrayFieldHandle(PdmChildArrayFieldHandle* childArray);
    PdmChildArrayFieldHandle* activeChildArrayFieldHandle();

    void                      setPdmRootObject(PdmObjectHandle* root);
    PdmObjectHandle*          pdmRootObject() { return m_rootObject; }

    PdmUiItem*                selectedItem(int selectionLevel = 0);
    void                      setSelectedItem(PdmUiItem* item, int selectionLevel = 0);

    void                      selectedItems(std::vector<PdmUiItem*>& items, int selectionLevel = 0);
    void                      setSelectedItems(const std::vector<PdmUiItem*>& items, int selectionLevel = 0);

    void                      selectionAsReferences(std::vector<QString>& referenceList, int selectionLevel = 0) const;
    void                      setSelectionFromReferences(const std::vector<QString>& referenceList, int selectionLevel = 0);

    void                      clearAll();
    void                      clear(int selectionLevel);
    void                      removeObjectFromAllSelections(PdmObjectHandle* pdmObject);

    template <typename T>
    void objectsByType(std::vector<T*>* typedObjects, int selectionLevel = 0)
    {
        std::vector<PdmUiItem*> items;
        this->selectedItems(items, selectionLevel);
        for (size_t i = 0; i < items.size(); i++)
        {
            T* obj = dynamic_cast<T*>(items[i]);
            if (obj) typedObjects->push_back(obj);
        }
    }
    
    /// Returns the selected objects of the requested type if _all_ the selected objects are of the requested type

    template <typename T>
    void objectsByTypeStrict(std::vector<T*>* typedObjects, int selectionLevel = 0)
    {
        std::vector<PdmUiItem*> items;
        this->selectedItems(items, selectionLevel);
        for (size_t i = 0; i < items.size(); i++)
        {
            T* obj = dynamic_cast<T*>(items[i]);
            if (!obj)
            {
                typedObjects->clear();
                break;
            }
            typedObjects->push_back(obj);
        }
    }

private:
    SelectionManager();

    void notifySelectionChanged( int selectionLevel);

    friend class SelectionChangedReceiver;
    void registerSelectionChangedReceiver  ( SelectionChangedReceiver* receiver) { m_selectionReceivers.insert(receiver);}
    void unregisterSelectionChangedReceiver( SelectionChangedReceiver* receiver) { m_selectionReceivers.erase(receiver);}

private:
    std::map <int,  std::vector< std::pair<PdmPointer<PdmObjectHandle>, PdmUiItem*> > >  m_selectionPrLevel;

    PdmChildArrayFieldHandle*   m_activeChildArrayFieldHandle;
    PdmPointer<PdmObjectHandle> m_rootObject;

    std::set< SelectionChangedReceiver*> m_selectionReceivers;
};



} // end namespace caf
