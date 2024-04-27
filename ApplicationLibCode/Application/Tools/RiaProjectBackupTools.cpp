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

#include "RiaProjectBackupTools.h"
#include "RiaLogging.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace RiaProjectBackupTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool createTableIfNeeded()
{
    QSqlQuery query;
    if ( !query.exec( "CREATE TABLE IF NOT EXISTS file_versions ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "timestamp DATETIME,"
                      "content TEXT)" ) )
    {
        QString txt = "Error creating table:" + query.lastError().text();
        RiaLogging::error( txt );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool insertContent( const QString& content )
{
    QSqlQuery query;
    query.prepare( "INSERT INTO file_versions (timestamp, content) "
                   "VALUES (:timestamp, :content)" );
    query.bindValue( ":timestamp", QDateTime::currentDateTime() );
    query.bindValue( ":content", content );
    if ( !query.exec() )
    {
        QString txt = "Error saving file content to database:" + query.lastError().text();
        RiaLogging::error( txt );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool appendTextToDatabase( const QString& databaseFilePath, const QString& content )
{
    QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE" );
    db.setDatabaseName( databaseFilePath );
    if ( !db.open() )
    {
        QString txt = "Error opening database:" + db.lastError().text();
        RiaLogging::error( txt );
        return false;
    }

    if ( !createTableIfNeeded() ) return false;
    if ( !insertContent( content ) ) return false;

    return true;
}

} // namespace RiaProjectBackupTools
