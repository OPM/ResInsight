/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RiaGrpcHelper.h"

#include "RiaApplication.h"
#include "RimCase.h"
#include "RimCommandRouter.h"
#include "RimProject.h"

#include "PdmObject.pb.h"

#include "cafPdmObjectScriptingCapabilityRegister.h"

//--------------------------------------------------------------------------------------------------
/// Convert internal ResInsight representation of cells with negative depth to positive depth.
//--------------------------------------------------------------------------------------------------
void RiaGrpcHelper::convertVec3dToPositiveDepth( cvf::Vec3d* vec )
{
    double& z = vec->z();
    z *= -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcHelper::setCornerValues( rips::Vec3d* out, const cvf::Vec3d& in )
{
    out->set_x( in.x() );
    out->set_y( in.y() );
    out->set_z( in.z() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RiaGrpcHelper::findCafObjectFromRipsObject( const rips::PdmObject& ripsObject )
{
    QString  scriptClassName = QString::fromStdString( ripsObject.class_keyword() );
    uint64_t address         = ripsObject.address();
    return findCafObjectFromScriptNameAndAddress( scriptClassName, address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RiaGrpcHelper::findCafObjectFromScriptNameAndAddress( const QString& scriptClassName, uint64_t address )
{
    QString classKeyword = caf::PdmObjectScriptingCapabilityRegister::classKeywordFromScriptClassName( scriptClassName );

    if ( classKeyword == RimCommandRouter::classKeywordStatic() ) return RiaApplication::instance()->commandRouter();

    RimProject*                  project = RimProject::current();
    std::vector<caf::PdmObject*> objectsOfCurrentClass;

    project->descendantsIncludingThisFromClassKeyword( classKeyword, objectsOfCurrentClass );

    caf::PdmObject* matchingObject = nullptr;
    for ( caf::PdmObject* testObject : objectsOfCurrentClass )
    {
        if ( reinterpret_cast<uint64_t>( testObject ) == address )
        {
            matchingObject = testObject;
        }
    }
    return matchingObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaGrpcHelper::numberOfDataUnitsInPackage( size_t dataUnitSize, size_t packageByteCount /*= 64 * 1024u */ )
{
    size_t dataUnitCount = packageByteCount / dataUnitSize;
    return dataUnitCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RiaGrpcHelper::findCase( int caseId )
{
    std::vector<RimCase*> cases = RimProject::current()->allGridCases();
    for ( RimCase* rimCase : cases )
    {
        if ( caseId == rimCase->caseId() )
        {
            return rimCase;
        }
    }
    return nullptr;
}

std::string RiaGrpcHelper::faceTypeToString( cvf::StructGridInterface::FaceType faceType )
{
    switch ( faceType )
    {
        case cvf::StructGridInterface::POS_I:
            return "I+";
        case cvf::StructGridInterface::NEG_I:
            return "I-";
        case cvf::StructGridInterface::POS_J:
            return "J+";
        case cvf::StructGridInterface::NEG_J:
            return "J-";
        case cvf::StructGridInterface::POS_K:
            return "K+";
        case cvf::StructGridInterface::NEG_K:
            return "K-";
        default:
            break;
    }
    return "";
}
