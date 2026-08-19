/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022-     Equinor ASA
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

#include "cvfObject.h"
#include "cvfVector3.h"

#include "cafAssert.h"

#include <QString>

#include <iosfwd>
#include <map>
#include <vector>

#include "RiaDefines.h"
#include "Token.hpp"

class RigEclipseCaseData;
class RigMainGrid;

namespace roff
{
class Reader;
}

//==================================================================================================
//
// Class for access to Roff grids.
//
//==================================================================================================
class RifRoffFileTools : public cvf::Object
{
public:
    RifRoffFileTools();
    ~RifRoffFileTools() override;

    static bool openGridFile( const QString& fileName, RigEclipseCaseData* eclipseCase, QString* errorMessages );

    // Parse a roff grid directly from an already opened binary stream. Used when the roff data does not
    // originate from a file on disk (e.g. a blob downloaded from Sumo).
    static bool openGridFile( std::istream& stream, RigEclipseCaseData* eclipseCase, QString* errorMessages );

    static std::pair<bool, std::map<QString, QString>> createInputProperties( const QString& fileName, RigEclipseCaseData* eclipseCase );

    // Read roff property data from an already opened binary stream (e.g. a blob downloaded from Sumo). The
    // sourceName is only used for log messages. resultCategory selects the result category the properties are
    // imported into (e.g. STATIC_NATIVE for static and DYNAMIC_NATIVE for time dependent properties).
    static std::pair<bool, std::map<QString, QString>>
        createInputProperties( std::istream&             stream,
                               RigEclipseCaseData*       eclipseCase,
                               const QString&            sourceName,
                               RiaDefines::ResultCatType resultCategory = RiaDefines::ResultCatType::INPUT_PROPERTY );

    static bool hasGridData( const QString& filename );

    // Read a single grid property from an in-memory roff blob and return its values (with inactive cells
    // masked), without registering it. Among the arrays matching the grid cell count, the one whose keyword
    // matches propertyName (case-insensitive) is preferred; otherwise the first matching array is used.
    static bool propertyValuesFromStream( std::istream&        stream,
                                          RigEclipseCaseData*  eclipseCase,
                                          const QString&       propertyName,
                                          std::vector<double>* values );

    static size_t computeActiveCellMatrixIndex( std::vector<int>& activeCells );

    static std::vector<double> computeZoneValuesFromSubgrids( const std::vector<int>& nLayers, size_t nx, size_t ny, size_t nz );

    static cvf::Vec3d getCorner( const RigMainGrid&        grid,
                                 const std::vector<float>& cornerLines,
                                 const std::vector<float>& zcorn,
                                 size_t                    cellIdx,
                                 int                       cornerIdx,
                                 const cvf::Vec3d&         offset,
                                 const cvf::Vec3d&         scale );

private:
    static void interpretSplitenzData( int                       nz,
                                       float                     zoffset,
                                       float                     zscale,
                                       const std::vector<char>&  splitenz,
                                       const std::vector<float>& zdata,
                                       std::vector<float>&       zcornsv );

    static double interpolate( const cvf::Vec3d& top, const cvf::Vec3d& bottom, double z, int idx );

    static std::vector<double>
        readAndConvertToDouble( int nx, int ny, int nz, const std::string& keyword, roff::Token::Kind kind, roff::Reader& reader );

    static bool appendNewInputPropertyResult( RigEclipseCaseData*       caseData,
                                              const QString&            resultName,
                                              const std::string&        keyword,
                                              roff::Token::Kind         token,
                                              roff::Reader&             reader,
                                              RiaDefines::ResultCatType resultCategory );

    static bool
        appendZoneIndexPropertyFromSubgrids( RigEclipseCaseData* caseData, roff::Reader& reader, std::map<QString, QString>& keywordMapping );

    static RiaDefines::ResultDataType mapFromType( roff::Token::Kind kind );
};
