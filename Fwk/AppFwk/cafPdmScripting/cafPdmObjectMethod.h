//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cafAssert.h"
#include "cafPdmObject.h"
#include "cafPdmObjectFactory.h"
#include "cafPdmPointer.h"
#include "cafPdmScriptResponse.h"

/// CAF_PDM_OBJECT_METHOD_SOURCE_INIT associates the self class keyword and the method keyword with the method factory
/// Place this in the cpp file, preferably above the constructor
#define CAF_PDM_OBJECT_METHOD_SOURCE_INIT( SelfClassName, MethodClassName, methodKeyword ) \
    CAF_PDM_XML_ABSTRACT_SOURCE_INIT( MethodClassName, methodKeyword )                     \
    static bool PDM_OBJECT_STRING_CONCATENATE( method##MethodClassName, __LINE__ ) =       \
        caf::PdmObjectMethodFactory::instance()->registerMethod<SelfClassName, MethodClassName>()

namespace caf
{
//==================================================================================================
/// PdmObject script method
/// Sub-class and register to the PdmObject to assign methods to a PdmObject that is accessible from
/// ... scripting engines such as Python.
/// Store arguments as member fields and assign return values in a PdmObject for execute.
/// Return value can be a storage class based on PdmObject returning resultIsPersistent() == false.
/// Or it can be a PdmObject in the project tree returning resultIsPersistent() == true.
///
//==================================================================================================
class PdmObjectMethod : public PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    PdmObjectMethod( PdmObjectHandle* self );

    // The returned object contains the results of the method and is the responsibility of the caller.
    virtual PdmObjectHandle* execute() = 0;

    // Some execute() methods can return a null pointer as a valid return value.
    // Return true here to allow this
    virtual bool isNullptrValidResult() const { return false; }

    virtual QString selfClassKeyword() const { return m_self->xmlCapability()->classKeyword(); }

    // True if object is a persistent project tree item. False if the object is to be deleted on completion.
    virtual bool resultIsPersistent() const = 0;

    // In order for the code generators to inspect the fields in the result object any PdmObjectMethod
    // ... need to provide an implementation that returns the same object type as the execute method.
    virtual std::unique_ptr<PdmObjectHandle> defaultResult() const = 0;

protected:
    // Basically the "this" pointer to the object the method belongs to
    template <typename PdmObjectType>
    PdmObjectType* self()
    {
        PdmObjectType* object = dynamic_cast<PdmObjectType*>( m_self.p() );
        CAF_ASSERT( object );
        return object;
    }

private:
    friend class PdmObjectScriptingCapability;
    PdmPointer<PdmObjectHandle> m_self;
};

//==================================================================================================
/// PdmObject script method factory
/// Register methods with this factory to be able to create and call methods.
//==================================================================================================
class PdmObjectMethodFactory
{
public:
    static PdmObjectMethodFactory* instance();

    std::shared_ptr<PdmObjectMethod> createMethod( PdmObjectHandle* self, const QString& methodName );

    template <typename PdmObjectDerivative, typename PdmObjectScriptMethodDerivative>
    bool registerMethod()
    {
        QString className  = PdmObjectDerivative::classKeywordStatic();
        QString methodName = PdmObjectScriptMethodDerivative::classKeywordStatic();

        auto classEntryIt = m_factoryMap.find( className );
        if ( classEntryIt != m_factoryMap.end() )
        {
            auto methodEntryIt = classEntryIt->second.find( methodName );
            if ( methodEntryIt != classEntryIt->second.end() )
            {
                CAF_ASSERT( methodName != methodEntryIt->first ); // classNameKeyword has already been used
                CAF_ASSERT( false ); // To be sure ..
                return false; // never hit;
            }
        }
        m_factoryMap[className][methodName] =
            std::shared_ptr<PdmObjectMethodCreatorBase>( new PdmObjectMethodCreator<PdmObjectScriptMethodDerivative>() );
        return true;
    }

    std::vector<QString> registeredMethodNames( const QString& className ) const;

private:
    PdmObjectMethodFactory()  = default;
    ~PdmObjectMethodFactory() = default;

    // Internal helper classes
    class PdmObjectMethodCreatorBase
    {
    public:
        PdmObjectMethodCreatorBase() {}
        virtual ~PdmObjectMethodCreatorBase() {}
        virtual std::shared_ptr<PdmObjectMethod> create( PdmObjectHandle* self ) = 0;
    };

    template <typename PdmObjectScriptMethodDerivative>
    class PdmObjectMethodCreator : public PdmObjectMethodCreatorBase
    {
    public:
        std::shared_ptr<PdmObjectMethod> create( PdmObjectHandle* self ) override
        {
            return std::shared_ptr<PdmObjectMethod>( new PdmObjectScriptMethodDerivative( self ) );
        }
    };

private:
    // Map to store factory
    std::map<QString, std::map<QString, std::shared_ptr<PdmObjectMethodCreatorBase>>> m_factoryMap;
    // Self pointer

}; // namespace caf

} // namespace caf
