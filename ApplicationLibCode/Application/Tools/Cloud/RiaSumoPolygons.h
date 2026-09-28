/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RiaSumoDefines.h"

#include <QByteArray>
#include <QString>

#include <vector>

class RiaSumoConnector;

// Category of a polygon result, matching ri-cloud-api's PolygonResult enum.
enum class SumoPolygonResult
{
    FieldOutline,
    StructureDepthFaultLines,
    FluidContactOutline
};

// One named polygon result. contactType is set only for fluid contact outline entries.
struct SumoPolygonMeta
{
    QString name;
    QString contactType;
};

// Polygon results available for an ensemble, categorized as ri-cloud-api reports them. An absent
// category is left empty.
struct SumoPolygonDirectory
{
    std::vector<SumoPolygonMeta> fieldOutline;
    std::vector<SumoPolygonMeta> structureDepthFaultLines;
    std::vector<SumoPolygonMeta> fluidContactOutline;
};

// Decoded geometry of one polygon (one POLY_ID group), already in [x,y,z] form -- ri-cloud-api
// owns the Sumo schema and returns plain coordinate arrays.
struct SumoPolygonData
{
    QString             name;
    QString             polyId;
    std::vector<double> xArr;
    std::vector<double> yArr;
    std::vector<double> zArr;
};

//==================================================================================================
/// Polygon results of a Sumo ensemble: what is available (polygon_result_directory) and decoded
/// coordinates of a chosen one (polygons_data). Both endpoints return plain JSON, so there is no
/// blob id/download step, unlike grid and summary data.
//==================================================================================================
class RiaSumoPolygons
{
public:
    explicit RiaSumoPolygons( RiaSumoConnector& connector );

    SumoPolygonDirectory polygonResultDirectory( const SumoCaseId& caseId, const QString& ensembleName );

    // name is ignored for FieldOutline. contactType is required only for FluidContactOutline.
    std::vector<SumoPolygonData> polygonsData( const SumoCaseId& caseId,
                                               const QString&    ensembleName,
                                               int               realization,
                                               SumoPolygonResult polygonResult,
                                               const QString&    name        = QString(),
                                               const QString&    contactType = QString() );

    // Query-parameter/JSON-key spelling of a polygon result category.
    static QString polygonResultKey( SumoPolygonResult polygonResult );

private:
    static SumoPolygonDirectory         parseDirectory( const QByteArray& body );
    static std::vector<SumoPolygonData> parsePolygonsData( const QByteArray& body );

private:
    RiaSumoConnector& m_connector;
};
