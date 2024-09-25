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
#include <QVariant>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

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
bool deleteOldRecords( int maximumRecordCount )
{
    QSqlQuery countQuery( "SELECT COUNT(*) FROM file_versions" );
    if ( !countQuery.exec() || !countQuery.next() )
    {
        RiaLogging::error( "Error counting records: " + countQuery.lastError().text() );
        return false;
    }

    int count           = countQuery.value( 0 ).toInt();
    int recordsToDelete = count - maximumRecordCount;
    if ( recordsToDelete <= 0 ) return true;

    QSqlQuery query;
    query.prepare( "DELETE FROM file_versions WHERE id IN (SELECT id FROM file_versions ORDER BY timestamp ASC LIMIT :limit)" );
    query.bindValue( ":limit", recordsToDelete );

    if ( !query.exec() )
    {
        QString txt = "Error deleting old records:" + query.lastError().text();
        RiaLogging::error( txt );
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool appendTextToDatabase( const QString& databaseFilePath, int maximumRecordCount, const QString& content )
{
    const QString databaseType = "QSQLITE";

    if ( !QSqlDatabase::isDriverAvailable( databaseType ) )
    {
        RiaLogging::error( "sqlite database is not available." );
        return false;
    }

    // Try to open the SQLITE database
    QSqlDatabase db = QSqlDatabase::database();
    if ( !db.isValid() || !db.open() )
    {
        RiaLogging::info( "Adding database" );

        // Add the SQLITE database, and it it required to do this once per session. The database will be available during the lifetime of
        // the application, and can be accessed using QSqlDatabase::database()
        db = QSqlDatabase::addDatabase( databaseType );
    }
    if ( !db.open() )
    {
        QString txt = "Error opening database:" + db.lastError().text();
        RiaLogging::error( txt );
        return false;
    }

    // Set the file name for the database. The database will be created if it does not exist.
    db.setDatabaseName( databaseFilePath );

    if ( !createTableIfNeeded() ) return false;
    if ( !deleteOldRecords( maximumRecordCount ) ) return false;

    if ( !insertContent( content ) ) return false;

    return true;
}

} // namespace RiaProjectBackupTools
