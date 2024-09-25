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

#include "RigNncConnection.h"

#include "cvfObject.h"
#include "cvfStructGrid.h"
#include "cvfVector3.h"

#include <QStringList>

#include <cmath> // Needed for HUGE_VAL on Linux
#include <deque>
#include <map>
#include <vector>

class RigActiveCellInfo;
class RigMainGrid;
class RigCell;
class RigEclipseResultAddress;

class RigNNCData : public cvf::Object
{
public:
    enum class NNCResultType
    {
        NNC_DYNAMIC,
        NNC_STATIC,
        NNC_GENERATED
    };

    RigNNCData();

    void setSourceDataForProcessing( RigMainGrid* mainGrid, const RigActiveCellInfo* activeCellInfo, bool includeInactiveCells );

    void                          setEclipseConnections( RigConnectionContainer& eclipseConnections );
    void                          buildPolygonsForEclipseConnections();
    size_t                        eclipseConnectionCount() const;
    const RigConnectionContainer& availableConnections() const;

    bool                    ensureAllConnectionDataIsProcessed();
    RigConnectionContainer& allConnections();

    std::vector<double>&       makeStaticConnectionScalarResult( QString nncDataType );
    const std::vector<double>* staticConnectionScalarResult( const RigEclipseResultAddress& resVarAddr ) const;
    const std::vector<double>* staticConnectionScalarResultByName( const QString& nncDataType ) const;
    void                       makeScalarResultAndSetValues( const QString& nncDataType, const std::vector<double>& values );

    std::vector<std::vector<double>>&       makeDynamicConnectionScalarResult( QString nncDataType, size_t timeStepCount );
    const std::vector<std::vector<double>>* dynamicConnectionScalarResult( const RigEclipseResultAddress& resVarAddr ) const;
    const std::vector<double>* dynamicConnectionScalarResult( const RigEclipseResultAddress& resVarAddr, size_t timeStep ) const;
    const std::vector<std::vector<double>>* dynamicConnectionScalarResultByName( const QString& nncDataType ) const;
    const std::vector<double>*              dynamicConnectionScalarResultByName( const QString& nncDataType, size_t timeStep ) const;

    std::vector<std::vector<double>>&       makeGeneratedConnectionScalarResult( QString nncDataType, size_t timeStepCount );
    const std::vector<std::vector<double>>* generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr ) const;
    const std::vector<double>*        generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr, size_t timeStep ) const;
    std::vector<std::vector<double>>* generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr );
    std::vector<double>*              generatedConnectionScalarResult( const RigEclipseResultAddress& resVarAddr, size_t timeStep );
    const std::vector<std::vector<double>>* generatedConnectionScalarResultByName( const QString& nncDataType ) const;
    const std::vector<double>*              generatedConnectionScalarResultByName( const QString& nncDataType, size_t timeStep ) const;
    std::vector<std::vector<double>>*       generatedConnectionScalarResultByName( const QString& nncDataType );
    std::vector<double>*                    generatedConnectionScalarResultByName( const QString& nncDataType, size_t timeStep );

    std::vector<QString> availableProperties( NNCResultType resultType ) const;

    void setEclResultAddress( const QString& nncDataType, const RigEclipseResultAddress& resVarAddr );

    bool hasScalarValues( const RigEclipseResultAddress& resVarAddr );

    bool generateScalarValues( const RigEclipseResultAddress& resVarAddr );

private:
    const QString getNNCDataTypeFromScalarResultIndex( const RigEclipseResultAddress& resVarAddr ) const;
    bool          isNative( QString nncDataType ) const;

    void computeAdditionalNncs( const RigMainGrid* mainGrid, const RigActiveCellInfo* activeCellInfo, bool includeInactiveCells );

    size_t connectionsWithNoCommonArea( QStringList& connectionTextFirstItems, size_t maxItemCount );

private:
    RigConnectionContainer                              m_connections;
    size_t                                              m_eclipseConnectionCount;
    std::map<QString, std::vector<std::vector<double>>> m_connectionResults;
    std::map<RigEclipseResultAddress, QString>          m_resultAddrToNNCDataType;

    bool                     m_havePolygonsForEclipseConnections;
    bool                     m_haveGeneratedConnections;
    RigMainGrid*             m_mainGrid;
    const RigActiveCellInfo* m_activeCellInfo;
    bool                     m_computeNncForInactiveCells;
};
