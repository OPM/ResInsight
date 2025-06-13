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
#include "cafPdmPointer.h"

#include <expected>

/// CAF_PDM_OBJECT_METHOD_SOURCE_INIT associates the self class keyword and the method keyword with the method factory
/// Place this in the cpp file, preferably above the constructor
#define CAF_PDM_OBJECT_METHOD_SOURCE_INIT( SelfClassName, MethodClassName, methodKeyword ) \
    CAF_PDM_XML_ABSTRACT_SOURCE_INIT( MethodClassName, methodKeyword )                     \
    static bool PDM_OBJECT_STRING_CONCATENATE( method##MethodClassName, __LINE__ ) =       \
        caf::PdmObjectMethodFactory::instance() -> registerMethod<SelfClassName, MethodClassName>()

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
    enum class NullPointerType
    {
        NULL_IS_VALID,
        NULL_IS_INVALID
    };
    enum class ResultType
    {
        PERSISTENT_TRUE,
        PERSISTENT_FALSE
    };

public:
    PdmObjectMethod( PdmObjectHandle* self, NullPointerType nullPointerType, ResultType resultType );

    // The returned object contains the results of the method and is the responsibility of the caller.
    virtual std::expected<caf::PdmObjectHandle*, QString> execute() = 0;

    virtual QString selfClassKeyword() const;
    virtual QString classKeywordReturnedType() const = 0;

    bool isNullptrValidResult() const;
    bool resultIsPersistent() const;

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

    NullPointerType m_nullPointerType = NullPointerType::NULL_IS_VALID;
    ResultType      m_resultType      = ResultType::PERSISTENT_TRUE;
};

// This is a method that does not return anything, i.e. it returns a nullptr as the result object
class PdmVoidObjectMethod : public PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    PdmVoidObjectMethod( PdmObjectHandle* self );

    QString classKeywordReturnedType() const override final;
};

// This is a method that creates a new Pdm object and returns it as the result object
// The object is persistent, i.e. it is stored in the project tree, and must not be deleted by the caller
class PdmObjectCreationMethod : public PdmObjectMethod
{
    CAF_PDM_HEADER_INIT;

public:
    PdmObjectCreationMethod( PdmObjectHandle* self );
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
