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


#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfTrace.h"

#include <map>

namespace cvf {



//==================================================================================================
///
/// \class cvf::Object
/// \ingroup Core
///
/// Base class for all reference counted objects
///
/// Very often used in conjunction with the ref<> class (smart pointer) which wraps the 
/// reference counting. Thus direct use of addRef() and release() is seldom needed.
///
/// The copy constructor and assignment operator is disabled by default so you will get compiler errors 
/// instead of unexpected behavior if you pass objects (that derive from Object) by value or assign objects.
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Get the set of currently active/allocated object instances
/// 
/// Note that unless the define CVF_TRACK_ACTIVE_OBJECT_INSTANCES is set to 1, this function 
/// will always return an empty set.
//--------------------------------------------------------------------------------------------------
std::set<Object*>* Object::activeObjectInstances()
{ 
    static std::set<Object*>*  sl_activeObjects = NULL;
    if (!sl_activeObjects)
    {
        sl_activeObjects = new std::set<Object*>;
    }

    return sl_activeObjects;
}


//--------------------------------------------------------------------------------------------------
/// Dump list of active object instances using Trace class
/// 
/// Currently this function utilizes RTTI and typeid to get the class names. 
/// Data on active instances will only be present if CVF_TRACK_ACTIVE_OBJECT_INSTANCES is defined
//--------------------------------------------------------------------------------------------------
void Object::dumpActiveObjectInstances()
{
    Trace::show("Dumping active object instances:");

    std::set<Object*>* objInstances = Object::activeObjectInstances();
    size_t numInstances = objInstances->size();
    
#if ((CVF_TRACK_ACTIVE_OBJECT_INSTANCES >= 1 && defined(_DEBUG)) || CVF_TRACK_ACTIVE_OBJECT_INSTANCES == 2)

    // Use a map to count the number of occurences of each class
    std::map<std::string, int> occurences;

    std::set<Object*>::iterator it;
    for (it = objInstances->begin(); it != objInstances->end(); ++it)
    {
        Object* obj = *it;

        const type_info& typeInfo = typeid(*obj);
        const char* className = typeInfo.name();
        if (occurences.find(className) != occurences.end())
        {
            occurences[className] += 1;
        }
        else
        {
            occurences[className] = 1;
        }
    }

    if (occurences.size() > 0)
    {
        std::map<std::string, int>::iterator it;
        for (it = occurences.begin(); it != occurences.end(); ++it)
        {
            Trace::show("%6d instances of:  %s", it->second, it->first.data());
        }
    }

#else

    Trace::show("  Tracking of active instances not enabled!");

#endif

    Trace::show("Total number of active object instances: %d", numInstances);
}


} // namespace cvf

