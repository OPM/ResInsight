#include "gtest/gtest.h"

#include "RifRoffReader.h"

#include "RiaTestDataDirectory.h"

#include <QDir>
#include <QString>

TEST( RifRoffReader, ReadValidFile )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifRoffReader/facies_info.roff" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    std::map<int, QString> codeNames;
    RifRoffReader::readCodeNames( filePath, codeNames );

    ASSERT_EQ( 6u, codeNames.size() );
    for ( int i = 0; i < 6; i++ )
    {
        ASSERT_TRUE( codeNames.find( i ) != codeNames.end() );
        ASSERT_EQ( codeNames.find( i )->second.toStdString(), QString( "code name %1" ).arg( i + 1 ).toStdString() );
    }
}

std::string readIncorrectFile( const QString filename )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filePath = baseFolder.absoluteFilePath( filename );

    std::map<int, QString> codeNames;
    try
    {
        RifRoffReader::readCodeNames( filePath, codeNames );
        return "";
    }
    catch ( RifRoffReaderException& ex )
    {
        return ex.message;
    }
}

TEST( RifRoffReader, ReadWrongFileType )
{
    // Read a surface file: no expected to work
    QString filename( "RifSurfaceImporter/test.ptl" );
    ASSERT_EQ( readIncorrectFile( filename ), std::string( "Unexpected file type: roff-asc header missing." ) );
}

TEST( RifRoffReader, ReadNonExistingFileType )
{
    // Read a non-existing file
    QString filename( "RifRoffReader/this_file_does_not_exist.roff" );
    ASSERT_EQ( readIncorrectFile( filename ), std::string( "Unable to open roff file." ) );
}

TEST( RifRoffReader, ReadFileWithIncorrectInteger )
{
    // Read a file with incorrect integer for code values
    QString filename( "RifRoffReader/code_values_integer_wrong.roff" );
    ASSERT_EQ( readIncorrectFile( filename ), std::string( "Unexpected value: not an integer." ) );
}

TEST( RifRoffReader, ReadFileCodeNamesMissing )
{
    // Read a file without code names
    QString filename( "RifRoffReader/code_names_missing.roff" );
    ASSERT_EQ( readIncorrectFile( filename ), std::string( "Code names not found." ) );
}

TEST( RifRoffReader, ReadFileCodeValuesMissing )
{
    // Read a file without code values
    QString filename( "RifRoffReader/code_values_missing.roff" );
    ASSERT_EQ( readIncorrectFile( filename ), std::string( "Code values not found." ) );
}

TEST( RifRoffReader, ReadFileCodeNamesAndValuesMismatch )
{
    // Read a file without code values
    QString filename( "RifRoffReader/code_names_and_values_mismatch.roff" );
    ASSERT_EQ( readIncorrectFile( filename ), std::string( "Inconsistent code names and values: must be equal length." ) );
}
