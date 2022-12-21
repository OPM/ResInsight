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

#include <QString>

#include <vector>

#include "Token.hpp"

class RigEclipseCaseData;
class RigMainGrid;
class Reader;

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

    static bool createInputProperties( const QString& fileName, RigEclipseCaseData* eclipseCase );

private:
    static void interpretSplitenzData( int                       nz,
                                       float                     zoffset,
                                       float                     zscale,
                                       const std::vector<char>&  splitenz,
                                       const std::vector<float>& zdata,
                                       std::vector<float>&       zcornsv );

    static void
        convertToReservoirIndexOrder( int nx, int ny, int nz, const std::vector<char>& activeIn, std::vector<int>& activeOut );

    static void
        convertToReservoirIndexOrder( int nx, int ny, int nz, const std::vector<float>& in, std::vector<double>& out );

    static void convertToReservoirIndexOrder( int nx, int ny, int nz, const std::vector<int>& in, std::vector<double>& out );

    static void
        convertToReservoirIndexOrder( int nx, int ny, int nz, const std::vector<char>& in, std::vector<double>& out );

    static size_t computeActiveCellMatrixIndex( std::vector<int>& activeCells );

    static cvf::Vec3d getCorner( const RigMainGrid&        grid,
                                 const std::vector<float>& cornerLines,
                                 const std::vector<float>& zcorn,
                                 size_t                    cellIdx,
                                 int                       cornerIdx,
                                 const cvf::Vec3d&         offset,
                                 const cvf::Vec3d&         scale );

    static double interpolate( const cvf::Vec3d& top, const cvf::Vec3d& bottom, double z, int idx );

    static std::vector<double>
        readAndConvertToDouble( int nx, int ny, int nz, const std::string& keyword, Token::Kind kind, Reader& reader );

    static bool appendNewInputPropertyResult( RigEclipseCaseData* caseData,
                                              const QString&      resultName,
                                              const std::string&  keyword,
                                              Token::Kind         token,
                                              Reader&             reader );
};
