/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

class RimPolygon;
class RimPolygonInView;

class QString;

namespace caf
{
class PdmObject;
}

class RimPolygonTools
{
public:
    static void activate3dEditOfPolygonInView( RimPolygon* polygon, caf::PdmObject* sourceObject );
    static void selectPolygonInView( RimPolygon* polygon, caf::PdmObject* sourceObject );
    static bool exportPolygonCsv( const RimPolygon* polygon, const QString& filePath );
    static bool exportPolygonPol( const RimPolygon* polygon, const QString& filePath );

    static QString polygonCacheName();

private:
    static RimPolygonInView* findPolygonInView( RimPolygon* polygon, caf::PdmObject* sourceObject );
};
