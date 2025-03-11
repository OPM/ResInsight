/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RiaWslTools.h"

#include "RimProcess.h"

#include <QFile>

//--------------------------------------------------------------------------------------------------
/// Returns path to the wsl command on Windows if found, nothing if not on Windows or not found
//--------------------------------------------------------------------------------------------------
QString RiaWslTools::wslCommand()
{
#ifdef WIN32
    QString wslCmd = qEnvironmentVariable( "SystemRoot", "C:\\WINDOWS" ) + "\\System32\\wsl.exe";
    if ( QFile::exists( wslCmd ) ) return wslCmd;
#endif
    return "";
}

//--------------------------------------------------------------------------------------------------
/// Returns the list of installed wsl distributions on this computer, or empty list if no wsl
//--------------------------------------------------------------------------------------------------
QStringList RiaWslTools::wslDistributionList()
{
    QString wslCmd = wslCommand();
    if ( !wslCmd.isEmpty() )
    {
        RimProcess wslProc( false /*no logging*/ );
        wslProc.addEnvironmentVariable( "WSL_UTF8", "1" );
        wslProc.setCommand( wslCmd );
        wslProc.addParameter( "--list" ); // list distribution names
        wslProc.addParameter( "--quiet" ); // quiet, only show name

        if ( wslProc.execute() )
        {
            return wslProc.stdOut();
        }
    }
    return QStringList();
}

//--------------------------------------------------------------------------------------------------
/// Returns the input windows path as seen in the wsl world. I.e. c:\tmp -> /mnt/c/tmp
//--------------------------------------------------------------------------------------------------
QString RiaWslTools::convertToWslPath( QString windowsPath )
{
    if ( windowsPath.size() < 2 ) return windowsPath;
    if ( windowsPath.startsWith( '/' ) ) return windowsPath;
    if ( windowsPath.at( 1 ) != ':' ) return windowsPath;

    QString drive = windowsPath.at( 0 );
    drive         = drive.toLower();

    auto path = windowsPath.replace( '\\', '/' );
    path      = path.section( ':', -1 );

    return QString( "/mnt/%1%2" ).arg( drive ).arg( path );
}
