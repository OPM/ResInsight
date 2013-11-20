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
#include "cvfuSnippetFactory.h"

namespace cvfu {

using cvf::ref;



//==================================================================================================
///
/// \class cvfu::SnippetInfo
/// \ingroup Utilities
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SnippetInfo::SnippetInfo(const cvf::String& id_, const cvf::String& name_)
:   id(id_), 
    name(name_)
{
}


//==================================================================================================
///
/// \class cvfu::SnippetFactory
/// \ingroup Utilities
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SnippetFactory::setTestDataDir(const cvf::String& testDataDir)
{
    m_testDataDir = testDataDir;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<SnippetInfo> SnippetFactory::availableSnippets(SnippetCategoryFlags categoryFlags) const
{
    std::vector<SnippetInfo> si;

    std::vector<SnippetMeta>::const_iterator it;
    for (it = m_registeredSnippets.begin(); it != m_registeredSnippets.end(); it++)
    {
        if (categoryFlags.testFlag(it->category))
        {
            cvf::String id = it->id;
            create_inst_func_ptr_type createSnipInstFuncPtr = it->createInstFuncPtr;

            // Create a live object and query for name
            TestSnippet* snippet = createSnipInstFuncPtr();
            si.push_back(SnippetInfo(id, snippet->name()));
        }
    }

    return si;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<TestSnippet> SnippetFactory::createSnippet(const cvf::String& id) const
{
    std::vector<SnippetMeta>::const_iterator it;
    for (it = m_registeredSnippets.begin(); it != m_registeredSnippets.end(); it++)
    {
        if (it->id == id)
        {
            // Looks funky, but is actually a function pointer.
            // The effect is to call the function that performs new on the snippet
            ref<TestSnippet> snippet = it->createInstFuncPtr();

            snippet->setTestDataDir(m_testDataDir);

            return snippet;
        }
    }

    return NULL;
}



//==================================================================================================
///
/// \class cvfu::SnippetRegistry
/// \ingroup Utilities
///
/// 
///
//==================================================================================================

SnippetRegistry* SnippetRegistry::m_instance = NULL;

//--------------------------------------------------------------------------------------------------
/// Private constructor
//--------------------------------------------------------------------------------------------------
SnippetRegistry::SnippetRegistry()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SnippetRegistry* SnippetRegistry::instance()
{
    if (!m_instance)
    {
        m_instance = new SnippetRegistry;
    }

    return m_instance;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void SnippetRegistry::addFactory(SnippetFactory* factory)
{
    m_factories.push_back(factory);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<SnippetInfo> SnippetRegistry::availableSnippets(SnippetCategoryFlags categoryFlags) const
{
    std::vector<SnippetInfo> availableSI;

    size_t numFactories = m_factories.size();
    size_t i;
    for (i = 0; i < numFactories; i++)
    {
        const SnippetFactory* factory = m_factories[i];
        std::vector<SnippetInfo> si = factory->availableSnippets(categoryFlags);

        availableSI.insert(availableSI.end(), si.begin(), si.end());
    }

    return availableSI;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<TestSnippet> SnippetRegistry::createSnippet(const cvf::String& id) const
{
    ref<TestSnippet> newSnippet;

    size_t numFactories = m_factories.size();
    size_t i;
    for (i = 0; i < numFactories; i++)
    {
        const SnippetFactory* factory = m_factories[i];
        ref<TestSnippet> newSnippet = factory->createSnippet(id);
        if (newSnippet.notNull())
        {
            return newSnippet;
        }
    }

    return NULL;
}


} // namespace cvfu

