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
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
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
        RiaLogging::error( txt.toStdString() );
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
        RiaLogging::error( txt.toStdString() );
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
        RiaLogging::error( "Error counting records: " + countQuery.lastError().text().toStdString() );
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
        RiaLogging::error( txt.toStdString() );
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
        RiaLogging::error( txt.toStdString() );
        return false;
    }

    // Set the file name for the database. The database will be created if it does not exist.
    db.setDatabaseName( databaseFilePath );

    if ( !createTableIfNeeded() ) return false;
    if ( !deleteOldRecords( maximumRecordCount ) ) return false;

    if ( !insertContent( content ) ) return false;

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Restore all previous revisions of a project file stored in the backup database to the destination folder.
/// Each revision is written to a separate file named by its timestamp.
//--------------------------------------------------------------------------------------------------
std::expected<QStringList, QString> extractVersionsToFolder( const QString& databaseFilePath, const QString& destinationFolder )
{
    auto fail = []( const QString& message ) -> std::expected<QStringList, QString>
    {
        RiaLogging::error( message.toStdString() );
        return std::unexpected( message );
    };

    if ( !QFile::exists( databaseFilePath ) )
    {
        return fail( "Backup database file does not exist: " + databaseFilePath );
    }

    QDir dir;
    if ( !dir.mkpath( destinationFolder ) )
    {
        return fail( "Not able to create destination folder: " + destinationFolder );
    }

    const QString databaseType = "QSQLITE";
    if ( !QSqlDatabase::isDriverAvailable( databaseType ) )
    {
        return fail( "sqlite database driver is not available." );
    }

    // Use a dedicated named connection to avoid interfering with the default connection used when writing backups.
    const QString connectionName = "RiaProjectBackupToolsRestoreConnection";

    std::expected<QStringList, QString> result;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase( databaseType, connectionName );
        db.setDatabaseName( databaseFilePath );

        if ( !db.open() )
        {
            auto error = fail( "Error opening backup database: " + db.lastError().text() );
            db         = QSqlDatabase();
            QSqlDatabase::removeDatabase( connectionName );
            return error;
        }

        QSqlQuery query( db );
        query.prepare( "SELECT timestamp, content FROM file_versions ORDER BY timestamp ASC" );
        if ( !query.exec() )
        {
            result = fail( "Error retrieving content from backup database: " + query.lastError().text() );
        }
        else
        {
            const QString dateFormat = "yyyy-MM-dd_hh-mm-ss";
            QStringList   restoredFiles;

            while ( query.next() )
            {
                QDateTime timestamp      = query.value( 0 ).toDateTime();
                QString   projectContent = query.value( 1 ).toString();

                QString filename = destinationFolder + "/" + timestamp.toString( dateFormat ) + ".rsp";
                // Versions saved within the same second must not overwrite each other
                for ( int suffix = 2; restoredFiles.contains( filename ); suffix++ )
                {
                    filename = destinationFolder + "/" + timestamp.toString( dateFormat ) + QString( "_%1.rsp" ).arg( suffix );
                }
                QFile file( filename );
                if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
                {
                    result = fail( "Error opening file for writing: " + filename );
                    break;
                }

                QTextStream out( &file );
                out << projectContent;
                file.close();

                restoredFiles.append( filename );
            }

            if ( result.has_value() ) result = restoredFiles;
        }

        db.close();
    }
    QSqlDatabase::removeDatabase( connectionName );

    return result;
}
} // namespace RiaProjectBackupTools
