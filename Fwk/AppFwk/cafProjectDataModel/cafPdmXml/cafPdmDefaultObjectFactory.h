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

#include "cafPdmObjectFactory.h"

#include "cafAssert.h"

#include <QString>

#include <map>
#include <vector>

namespace caf
{
//==================================================================================================
/// "Private" class for implementation of a factory for PdmObjectBase derived objects
/// Every PdmObject must register with this factory to be readable
/// This class can be considered private in the Pdm system
//==================================================================================================

class PdmDefaultObjectFactory : public PdmObjectFactory
{
public:
    static PdmDefaultObjectFactory * instance();

    PdmObjectHandle* create(const QString& classNameKeyword) override;

    template< typename PdmObjectBaseDerivative >
    bool                registerCreator()
    {
        std::vector<QString> classNameKeywords = PdmObjectBaseDerivative::classKeywordAliases();

        for (QString classNameKeyword : classNameKeywords)
        {
            auto entryIt = m_factoryMap.find(classNameKeyword);
            if (entryIt != m_factoryMap.end())
            {
                CAF_ASSERT(classNameKeyword != entryIt->first); // classNameKeyword has already been used
                CAF_ASSERT(false); // To be sure ..
                return false;  // never hit;
            }
        }
        auto object = new PdmObjectCreator<PdmObjectBaseDerivative>();
        for (QString classNameKeyword : classNameKeywords)
        {
            m_factoryMap[classNameKeyword] = object;
        }
        return true;
    }

    std::vector<QString> classKeywords() const override;

private:
    PdmDefaultObjectFactory()  {}
    ~PdmDefaultObjectFactory() override { /* Could clean up, but ... */ }

    // Internal helper classes

    class PdmObjectCreatorBase
    {
    public:
        PdmObjectCreatorBase() {}
        virtual ~PdmObjectCreatorBase() {}
        virtual PdmObjectHandle * create() = 0;
    };

    template< typename PdmObjectBaseDerivative >
    class PdmObjectCreator : public PdmObjectCreatorBase
    {
    public:
        PdmObjectHandle * create() override { return new PdmObjectBaseDerivative(); }
    };

    // Map to store factory
    std::map<QString, PdmObjectCreatorBase*> m_factoryMap;
};


} //End of namespace caf
