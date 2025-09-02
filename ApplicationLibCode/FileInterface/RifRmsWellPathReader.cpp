/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RifRmsWellPathReader.h"

#include "RiaLogging.h"
#include "RiaStringEncodingTools.h"

#include "RifWellPathImporter.h"
#include "Well/RigWellPathGeometryTools.h"

#include "well.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifWellPathImporter::WellData RifRmsWellPathReader::readWellData( const QString& filePath )
{
    try
    {
        int wellFormat = NRLib::Well::RMS;

        std::unique_ptr<NRLib::Well> well( NRLib::Well::ReadWell( RiaStringEncodingTools::toNativeEncoded( filePath ).data(), wellFormat ) );

        if ( well )
        {
            const std::map<std::string, std::vector<double>>& contLogs = well->GetContLog();

            std::vector<double> x;
            std::vector<double> y;
            std::vector<double> z;
            std::vector<double> md;
            for ( const auto& [name, values] : contLogs )
            {
                QString logName = QString::fromStdString( name ).toUpper();
                if ( logName == "X" )
                    x = values;
                else if ( logName == "Y" )
                    y = values;
                else if ( logName == "Z" )
                    z = values;
                else if ( logName == "MDEPTH" )
                    md = values;
            }

            if ( !x.empty() && x.size() == y.size() && x.size() == z.size() && md.empty() )
            {
                std::vector<cvf::Vec3d> points;
                points.resize( x.size() );
                for ( size_t i = 0; i < x.size(); i++ )
                {
                    points[i] = cvf::Vec3d( x[i], y[i], z[i] );
                }
                md = RigWellPathGeometryTools::calculateMeasuredDepth( points );
            }

            if ( !x.empty() && x.size() == y.size() && x.size() == z.size() && x.size() == md.size() )
            {
                auto wellData = RifWellPathImporter::WellData();

                wellData.m_name             = QString::fromStdString( well->GetWellName() );
                wellData.m_wellPathGeometry = new RigWellPath;
                for ( size_t i = 0; i < x.size(); i++ )
                {
                    cvf::Vec3d position( x[i], y[i], -z[i] );
                    double     measuredDepth = md[i];
                    wellData.m_wellPathGeometry->addWellPathPoint( position, measuredDepth );
                }

                return wellData;
            }
        }
    }
    catch ( std::exception& e )
    {
        if ( e.what() )
        {
            RiaLogging::error( QString( "Failed to import RMS well path: %1. Error: %2" ).arg( filePath ).arg( e.what() ) );
        }
    }

    RiaLogging::error( QString( "Failed to import RMS well path: %1." ).arg( filePath ) );
    return RifWellPathImporter::WellData();
}
