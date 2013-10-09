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

#include <assert.h>
#include <map>

namespace caf
{

    //==================================================================================================
    /// A generic Factory class template
    /// Usage:
    ///     Simply add the classes that is supposed to be created by the factory by doing the folowing:
    ///
    ///     caf::Factory<BaseType, KeyType>::instance()->registerCreator<TypeToCreate>(key);
    ///
    ///     This must only be done once for each TypeToCreate. It will assert if you try to do it several times.
    ///     This method returns a bool to make it possible to make this initialization as a static variable initialization.
    ///     That is useful if you do not want a centralized registering (but rather making each class register itself):
    ///
    ///     static bool uniqueVarname = caf::Factory<BaseType, KeyType>::instance()->registerCreator<TypeToCreate>(key);
    ///
    ///     See also cafPdmUiFieldEditorHandle.h for an advanced example.
    ///
    ///     When you need an object:
    ///
    ///     BaseType* newObject = caf::Factory<BaseType, KeyType>::instance()->create(key);
    //==================================================================================================


    template<typename BaseType, typename KeyType>
    class Factory
    {
        class ObjectCreatorBase;
    public:
        typedef typename std::map<KeyType, ObjectCreatorBase*>::iterator iterator_type;

        static Factory<BaseType, KeyType> * instance()
        {
            static Factory<BaseType, KeyType>* fact = new Factory<BaseType, KeyType>;
            return fact;
        }

        template< typename TypeToCreate >
        bool registerCreator(const KeyType& key)
        {
            iterator_type entryIt;

            entryIt = m_factoryMap.find(key);
            if (entryIt == m_factoryMap.end())
            {
                m_factoryMap[key] = new ObjectCreator<TypeToCreate>();
                return true;
            }
            else
            {
                assert(key != entryIt->first); // classNameKeyword has already been used
                assert(false); // To be sure ..
                return false;
            }
        }

        BaseType* create(const KeyType& key)
        {
            iterator_type entryIt;

            entryIt = m_factoryMap.find(key);
            if (entryIt != m_factoryMap.end())
            {
                return entryIt->second->create();
            }
            else
            {
                return NULL;
            }
        }


    private:
        Factory ()  {}
        ~Factory() 
        {
            iterator_type entryIt;

            for (entryIt = m_factoryMap.begin(); entryIt != m_factoryMap.end(); ++entryIt)
            {
                delete(entryIt->second);
            }
        }

        // Internal helper classes

        class ObjectCreatorBase
        {
        public:
            ObjectCreatorBase()          {}
            virtual ~ObjectCreatorBase() {}
            virtual BaseType * create() = 0;
        };

        template< typename TypeToCreate >
        class ObjectCreator : public ObjectCreatorBase
        {
        public:
            virtual BaseType * create() { return new TypeToCreate(); }
        };

        // Map to store factory
        std::map<KeyType, ObjectCreatorBase*> m_factoryMap;
    };


}//End of namespace caf
