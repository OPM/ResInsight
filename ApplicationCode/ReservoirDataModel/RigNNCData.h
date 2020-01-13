/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RiaNncDefines.h"

#include "cvfObject.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <cmath> // Needed for HUGE_VAL on Linux
#include <map>
#include <vector>

class RigMainGrid;
class RigCell;
class RigEclipseResultAddress;

class RigConnection
{
public:
    RigConnection()
        : m_c1GlobIdx( cvf::UNDEFINED_SIZE_T )
        , m_c1Face( cvf::StructGridInterface::NO_FACE )
        , m_c2GlobIdx( cvf::UNDEFINED_SIZE_T )
    {
    }

    bool hasCommonArea() const
    {
        return m_polygon.size() > 0;
    }

    size_t                             m_c1GlobIdx;
    cvf::StructGridInterface::FaceType m_c1Face;
    size_t                             m_c2GlobIdx;

    std::vector<cvf::Vec3d> m_polygon;
};

class RigNNCData : public cvf::Object
{
public:
    enum NNCResultType
    {
        NNC_DYNAMIC,
        NNC_STATIC,
        NNC_GENERATED
    };

    RigNNCData();

    void processConnections( const RigMainGrid& mainGrid );

    static cvf::StructGridInterface::FaceType calculateCellFaceOverlap( const RigCell&           c1,
                                                                        const RigCell&           c2,
                                                                        const RigMainGrid&       mainGrid,
                                                                        std::vector<size_t>*     connectionPolygon,
                                                                        std::vector<cvf::Vec3d>* connectionIntersections );

    std::vector<RigConnection>& connections()
    {
        return m_connections;
    }
    const std::vector<RigConnection>& connections() const
    {
        return m_connections;
    }

    std::vector<double>&       makeStaticConnectionScalarResult( QString nncDataType );
    const std::vector<double>* staticConnectionScalarResult( const RigEclipseResultAddress& resVarAddr ) const;
    const std::vector<double>* staticConnectionScalarResultByName( const QString& nncDataType ) const;

    std::vector<std::vector<double>>& makeDynamicConnectionScalarResult( QString nncDataType, size_t timeStepCount );
    const std::vector<std::vector<double>>* dynamicConnectionScalarResult( const RigEclipseResultAddress& resVarAddr ) const;
    const std::vector<double>*              dynamicConnectionScalarResult( const RigEclipseResultAddress& resVarAddr,
                                                                           size_t                         timeStep ) const;
    const std::vector<std::vector<double>>* dynamicConnectionScalarResultByName( const QString& nncDataType ) const;
    const std::vector<double>* dynamicConnectionScalarResultByName( const QString& nncDataType, size_t timeStep ) const;

    std::vector<std::vector<double>>& makeGeneratedConnectionScalarResult( QString nncDataType, size_t timeStepCount );
    const std::vector<std::vector<double>>*
                                      generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr ) const;
    const std::vector<double>*        generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr,
                                                                       size_t                         timeStep ) const;
    std::vector<std::vector<double>>* generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr );
    std::vector<double>* generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr, size_t timeStep );
    const std::vector<std::vector<double>>* generatedConnectionScalarResultByName( const QString& nncDataType ) const;
    const std::vector<double>* generatedConnectionScalarResultByName( const QString& nncDataType, size_t timeStep ) const;
    std::vector<std::vector<double>>* generatedConnectionScalarResultByName( const QString& nncDataType );
    std::vector<double>* generatedConnectionScalarResultByName( const QString& nncDataType, size_t timeStep );

    std::vector<QString> availableProperties( NNCResultType resultType ) const;

    void setEclResultAddress( const QString& nncDataType, const RigEclipseResultAddress& resVarAddr );

    bool hasScalarValues( const RigEclipseResultAddress& resVarAddr );

private:
    const QString getNNCDataTypeFromScalarResultIndex( const RigEclipseResultAddress& resVarAddr ) const;
    bool          isNative( QString nncDataType ) const;

private:
    std::vector<RigConnection>                          m_connections;
    std::map<QString, std::vector<std::vector<double>>> m_connectionResults;
    std::map<RigEclipseResultAddress, QString>          m_resultAddrToNNCDataType;
};
