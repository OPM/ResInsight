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

    static std::pair<bool, std::map<QString, QString>> createInputProperties( const QString& fileName, RigEclipseCaseData* eclipseCase );

    static bool hasGridData( const QString& filename );

    static size_t computeActiveCellMatrixIndex( std::vector<int>& activeCells );

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

    static bool appendNewInputPropertyResult( RigEclipseCaseData* caseData,
                                              const QString&      resultName,
                                              const std::string&  keyword,
                                              roff::Token::Kind   token,
                                              roff::Reader&       reader );

    static RiaDefines::ResultDataType mapFromType( roff::Token::Kind kind );
};
