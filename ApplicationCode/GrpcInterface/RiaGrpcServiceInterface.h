/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
//////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <grpcpp/grpcpp.h>

#include <vector>

class RiaGrpcCallbackInterface;
class RimCase;

namespace caf
{
class PdmChildArrayFieldHandle;
class PdmChildFieldHandle;
class PdmFieldHandle;
class PdmObject;
class PdmObjectHandle;
} // namespace caf

namespace rips
{
class PdmObject;
}

class QString;
class QVariant;

//==================================================================================================
//
// gRPC-service interface which all gRPC-services has to implement
//
//==================================================================================================
class RiaGrpcServiceInterface
{
public:
    virtual std::vector<RiaGrpcCallbackInterface*> createCallbacks() = 0;
    virtual ~RiaGrpcServiceInterface()                               = default;
    static RimCase* findCase( int caseId );
    static size_t   numberOfDataUnitsInPackage( size_t dataUnitSize, size_t packageByteCount = 64 * 1024u );

    static void copyPdmObjectFromCafToRips( const caf::PdmObjectHandle* source, rips::PdmObject* destination );
    static void copyPdmObjectFromRipsToCaf( const rips::PdmObject* source, caf::PdmObjectHandle* destination );

    static bool
        assignFieldValue( const QString& stringValue, caf::PdmFieldHandle* field, QVariant* oldValue, QVariant* newValue );

    static caf::PdmObjectHandle* emplaceChildField( caf::PdmObject* parent, const QString& fieldLabel );

    static caf::PdmObjectHandle* emplaceChildField( caf::PdmChildFieldHandle* childField );
    static caf::PdmObjectHandle* emplaceChildArrayField( caf::PdmChildArrayFieldHandle* childArrayField );
};

#include "cafFactory.h"
typedef caf::Factory<RiaGrpcServiceInterface, size_t> RiaGrpcServiceFactory;
