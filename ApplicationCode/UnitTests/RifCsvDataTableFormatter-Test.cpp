#include "gtest/gtest.h"

#include "RifCsvDataTableFormatter.h"

#include <QString>
#include <QStringList>

TEST( RifCsvDataTableFormatter, BasicUsage )
{
    QString                  tableText;
    QTextStream              stream( &tableText );
    RifCsvDataTableFormatter formatter( stream, ";" );

    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn( "Well" ),
        RifEclipseOutputTableColumn( "Integer Number" ),
        RifEclipseOutputTableColumn( "sci", RifEclipseOutputTableDoubleFormat::RIF_SCIENTIFIC ),
        RifEclipseOutputTableColumn( "float", RifEclipseOutputTableDoubleFormat::RIF_FLOAT ),
        RifEclipseOutputTableColumn( "consise", RifEclipseOutputTableDoubleFormat::RIF_CONSISE ),
    };

    formatter.header( header );

    formatter.add( "well a" );
    formatter.add( 1 );
    formatter.add( 2.123456789 );
    formatter.add( 2.123456789 );
    formatter.add( 2.123456789 );
    formatter.rowCompleted();

    formatter.add( "well B" );
    formatter.add( 12 );
    formatter.add( 0.3e-12 );
    formatter.add( 0.3e-12 );
    formatter.add( 0.3e-12 );
    formatter.rowCompleted();

    formatter.add( "well c" );
    formatter.add( 123 );
    formatter.add( 0.3e+12 );
    formatter.add( 0.3e+12 );
    formatter.add( 0.3e+12 );
    formatter.rowCompleted();

    formatter.tableCompleted();

    std::cout << tableText.toStdString();
}
