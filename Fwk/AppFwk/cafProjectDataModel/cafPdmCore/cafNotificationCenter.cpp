/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2014 Ceetron Solutions AS
// 
//  <APPLICATION_NAME> is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  <APPLICATION_NAME> is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "cafNotificationCenter.h"

#include "cafAssert.h"

#include <QtGlobal>

#include <algorithm>


namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
NotificationCenter::NotificationCenter()
{
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
NotificationCenter::~NotificationCenter()
{
    CAF_ASSERT(m_observers.size() == 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void NotificationCenter::registerObserver(DataModelObserver* observer)
{
    std::vector<DataModelObserver*>::iterator it = std::find(m_observers.begin(), m_observers.end(), observer);
    if (it == m_observers.end())
    {
        m_observers.push_back(observer);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void NotificationCenter::removeObserver(DataModelObserver* observer)
{
    std::vector<DataModelObserver*>::iterator it = std::find(m_observers.begin(), m_observers.end(), observer);
    if (it != m_observers.end())
    {
        m_observers.erase(it);
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void NotificationCenter::notifyObserversOfDataChange(caf::PdmObjectHandle* itemThatChanged)
{
    CAF_ASSERT(itemThatChanged);

    foreach(DataModelObserver* o, m_observers)
    {
        o->handleModelNotification(itemThatChanged);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void NotificationCenter::notifyObservers()
{
    foreach(DataModelObserver* o, m_observers)
    {
        o->handleModelNotification(nullptr);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void NotificationCenter::notifyObserversOfSelectionChange()
{
    foreach(DataModelObserver* o, m_observers)
    {
        o->handleModelSelectionChange();
    }
}





} // end namespace caf
