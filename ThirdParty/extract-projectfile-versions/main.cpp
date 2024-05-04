#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QDateTime, QString>> getAllContent()
{
    std::vector<std::pair<QDateTime, QString>> content;

    QSqlQuery query;

    query.prepare( "SELECT timestamp, content FROM file_versions" );
    if ( !query.exec() )
    {
        qDebug() << "Error retrieving content:" << query.lastError().text();
        return content;
    }

    while ( query.next() )
    {
        QDateTime timestamp      = query.value( 0 ).toDateTime();
        QString   projectContent = query.value( 1 ).toString();
        content.push_back( std::make_pair( timestamp, projectContent ) );
    }

    return content;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void extractVersionsToFolder( const QString& destinationFolder )
{
    auto allContent = getAllContent();

    for ( const auto& [timestamp, content] : allContent )
    {
        const QString dateFormat = "yyyy-MM-dd_hh-mm-ss";
        QString       filename   = destinationFolder + "/" + timestamp.toString( dateFormat ) + ".rsp";
        QFile         file( filename );
        if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
        {
            qCritical() << "Error opening file for writing:" << filename;
            return;
        }

        QTextStream out( &file );
        out << content;
        file.close();
    }
}

int main( int argc, char* argv[] )
{
    QCoreApplication app( argc, argv );
    QCoreApplication::setApplicationName( "extract-projectfile-versions" );
    QCoreApplication::setApplicationVersion( "1.0" );

    QCommandLineParser parser;
    parser.setApplicationDescription(
        "extract-projectfile-versions is used to restore previous revisions of a ResInsight project from a project file database." );
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument( "file", "ResInsight project file database (*.rspdb)" );
    parser.addPositionalArgument( "outputfolder", "Output folder all project files" );

    parser.process( app );

    const QStringList args = parser.positionalArguments();
    if ( args.size() != 2 )
    {
        qCritical() << "Failed to detect two input arguments.";
        parser.showHelp( 1 );
        return 1;
    }

    QString databaseFilePath = args.front();
    if ( !QFile::exists( databaseFilePath ) )
    {
        qCritical() << "Database file does not exist:" << databaseFilePath;
        return 1;
    }

    QString destinationDir = args[1];
    QDir    dir;
    if ( !dir.mkpath( destinationDir ) )
    {
        qCritical() << "Not able to create destination folder : " << destinationDir;
        return 1;
    }

    const QString databaseType = "QSQLITE";

    if ( !QSqlDatabase::isDriverAvailable( databaseType ) )
    {
        qInfo() << "sqlite database is not available.";
        return 1;
    }

    // Try to open the SQLITE database
    QSqlDatabase db = QSqlDatabase::database();
    if ( !db.isValid() || !db.open() )
    {
        qInfo() << "Adding database";

        // Add the SQLITE database, and it it required to do this once per session. The database will be available during the lifetime
        // of the application, and can be accessed using QSqlDatabase::database()
        db = QSqlDatabase::addDatabase( databaseType );
    }

    QFileInfo fileInfo( databaseFilePath );
    auto      dbPath = fileInfo.absoluteFilePath();
    db.setDatabaseName( dbPath );

    if ( !db.open() )
    {
        QString txt = "Error opening database:" + db.lastError().text();
        qCritical() << txt;
        return 1;
    }

    extractVersionsToFolder( destinationDir );

    return 1;
}
