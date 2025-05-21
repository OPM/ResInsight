/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RifEdfmTools.h"

#include <QFile>
#include <QFileInfo>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RifEdfmTools::checkForEdfmLimitI( QString gridFileName )
{
    QFileInfo fi( gridFileName );

    QString edfmFileName = fi.absolutePath() + "/_EDFM.txt";
    if ( !QFile::exists( edfmFileName ) ) return 0;

    QFile edfmFile( edfmFileName );
    if ( edfmFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        QTextStream ts( &edfmFile );
        while ( !ts.atEnd() )
        {
            auto line = ts.readLine().trimmed();
            if ( line.startsWith( "NXORIGINAL" ) )
            {
                auto list = line.split( " ", Qt::SkipEmptyParts );
                if ( list.size() == 2 )
                {
                    return list[1].toUInt();
                }

                break;
            }
        }
        edfmFile.close();
    }

    return 0;
}
