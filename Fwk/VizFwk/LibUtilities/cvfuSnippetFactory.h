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

#include "cvfuTestSnippet.h"

namespace cvfu {

// Categorization of snippets
enum SnippetCategory
{
    SC_TEST         = 0x00000001,               // Ordinary test snippets
    SC_TEST_HEAVY   = 0x00000002,               // Test snippets, heavy (long load time, high memory usage, etc)
    SC_EXPERIMENT   = 0x00000004,               // 
    SC_ALL          = 0xffffffff,               // All categories
    SC_ALL_TESTS    = SC_TEST | SC_TEST_HEAVY   // All test categories
};

typedef cvf::Flags<SnippetCategory> SnippetCategoryFlags;


//==================================================================================================
//
// 
//
//==================================================================================================
class SnippetInfo
{
public:
    SnippetInfo(const cvf::String& id, const cvf::String& name);

public:
    cvf::String id;     // Textual ID of the snippet
    cvf::String name;   // Human readable name of the snippet
};


//==================================================================================================
//
// Base class for factory for creating instances of TestSnippets
//
//==================================================================================================
class SnippetFactory
{
public:
    void                        setTestDataDir(const cvf::String& testDataDir);

    std::vector<SnippetInfo>    availableSnippets(SnippetCategoryFlags categoryFlags) const;
    cvf::ref<TestSnippet>       createSnippet(const cvf::String& id) const;

protected:
    template<typename T> 
    void registerSnippet(const cvf::String& snippetId, SnippetCategory snippetCategory)
    {
        SnippetMeta snipMeta;
        snipMeta.id = snippetId;
        snipMeta.category = snippetCategory;
        snipMeta.createInstFuncPtr = &SnippetFactory::createSnippetInstance<T>;
        m_registeredSnippets.push_back(snipMeta);
    }

private:
    template<typename T> 
    static TestSnippet* createSnippetInstance() 
    { 
        return new T; 
    }

private:
    typedef TestSnippet*(*create_inst_func_ptr_type)();

    struct SnippetMeta
    {
        cvf::String                 id;
        SnippetCategory             category;
        create_inst_func_ptr_type   createInstFuncPtr;
    };

    cvf::String                 m_testDataDir;          // Directory for test data available to snippets
    std::vector<SnippetMeta>    m_registeredSnippets;  // Array of snippets that this factory is capable of creating
};



//==================================================================================================
//
// Registry for registering factories to create instances of all TestSnippets
//
//==================================================================================================
class SnippetRegistry
{
public:
    static SnippetRegistry*     instance();

    void                        addFactory(SnippetFactory* factory);
    std::vector<SnippetInfo>    availableSnippets(SnippetCategoryFlags categoryFlags) const;
    cvf::ref<TestSnippet>       createSnippet(const cvf::String& id) const;

private:
    SnippetRegistry();

private:
    static SnippetRegistry*         m_instance;
    std::vector<SnippetFactory*>    m_factories;
};

}


// Helper macro to use when registering snippets in a snippet factory constructor
#define CVFU_REGISTER_SNIPPET(SNIPPET_CLASS, SNIPPET_CAT)  registerSnippet<SNIPPET_CLASS>(#SNIPPET_CLASS, SNIPPET_CAT);


