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

// The category of a polygon result, matching ri-cloud-api's PolygonResult enum values (the query
// parameter it expects, and the keys of the polygon_result_directory response).
enum class SumoPolygonResult
{
    FieldOutline,
    StructureDepthFaultLines,
    FluidContactOutline
};

// One named polygon result available for an ensemble. contactType is only set (non-empty) for a
// fluid contact outline entry.
struct SumoPolygonMeta
{
    QString name;
    QString contactType;
};

// The polygon results available for an ensemble, categorized as ri-cloud-api reports them. A
// category not present in the response (nothing of that kind exists) is left empty here.
struct SumoPolygonDirectory
{
    std::vector<SumoPolygonMeta> fieldOutline;
    std::vector<SumoPolygonMeta> structureDepthFaultLines;
    std::vector<SumoPolygonMeta> fluidContactOutline;
};

// The decoded geometry of one polygon (one POLY_ID group within a named result), already in
// [x,y,z] form -- ri-cloud-api owns the Sumo parquet/csv schema and hands back plain coordinate
// arrays, so no parsing of Sumo's on-disk polygon format happens on the ResInsight side.
struct SumoPolygonData
{
    QString             name;
    QString             polyId;
    std::vector<double> xArr;
    std::vector<double> yArr;
    std::vector<double> zArr;
};

//==================================================================================================
/// The polygon results of a Sumo ensemble: what is available (polygon_result_directory) and the
/// decoded coordinates of a chosen one (polygons_data). Requests are made through RiaSumoConnector,
/// which owns the connection and does the transfers; ri-cloud-api returns plain JSON for both
/// endpoints, so there is no blob id/download step here, unlike grid and summary data.
//==================================================================================================
class RiaSumoPolygons
{
public:
    explicit RiaSumoPolygons( RiaSumoConnector& connector );

    SumoPolygonDirectory polygonResultDirectory( const SumoCaseId& caseId, const QString& ensembleName );

    // name is ignored for SumoPolygonResult::FieldOutline (there is exactly one, unnamed). contactType is
    // required only when polygonResult is FluidContactOutline.
    std::vector<SumoPolygonData> polygonsData( const SumoCaseId&  caseId,
                                               const QString&     ensembleName,
                                               int                realization,
                                               SumoPolygonResult  polygonResult,
                                               const QString&     name        = QString(),
                                               const QString&     contactType = QString() );

    // The query-parameter/JSON-key spelling of a polygon result category, matching ri-cloud-api's
    // PolygonResult enum values (e.g. "fluid_contact_outline").
    static QString polygonResultKey( SumoPolygonResult polygonResult );

private:
    static SumoPolygonDirectory        parseDirectory( const QByteArray& body );
    static std::vector<SumoPolygonData> parsePolygonsData( const QByteArray& body );

private:
    RiaSumoConnector& m_connector;
};
