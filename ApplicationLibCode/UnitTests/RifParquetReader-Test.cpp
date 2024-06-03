#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include <arrow/csv/api.h>
#include <arrow/io/api.h>
#include <arrow/scalar.h>
#include <parquet/arrow/reader.h>

#include <QDir>
#include <QString>

TEST( RifParquetReaderTest, ReadValidFile )
{
    QDir baseFolder( TEST_DATA_DIR );

    QString filename( "RifParquetReader/example.parquet" );
    QString filePath = baseFolder.absoluteFilePath( filename );
    EXPECT_TRUE( QFile::exists( filePath ) );

    arrow::MemoryPool* pool = arrow::default_memory_pool();

    auto openResult = arrow::io::ReadableFile::Open( filePath.toStdString().c_str() );
    EXPECT_TRUE( openResult.ok() );

    std::shared_ptr<arrow::io::RandomAccessFile> input = std::move( openResult ).ValueOrDie();

    // Open Parquet file reader
    std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
    EXPECT_TRUE( parquet::arrow::OpenFile( input, pool, &arrow_reader ).ok() );

    // Read entire file as a single Arrow table
    std::shared_ptr<arrow::Table> table;
    EXPECT_TRUE( arrow_reader->ReadTable( &table ).ok() );

    // Expect one column named "col1"
    EXPECT_EQ( table->columns().size(), 1 );
    std::vector<std::string> expectedNames = { "col1" };
    EXPECT_EQ( table->ColumnNames(), expectedNames );

    // Expected the column to contain 100 int64 [0, 99]
    auto columnData   = table->column( 0 );
    int  expectedSize = 100;
    EXPECT_EQ( columnData->length(), expectedSize );
    for ( int i = 0; i < expectedSize; i++ )
    {
        std::shared_ptr<arrow::Scalar>      scalar    = columnData->GetScalar( i ).ValueOrDie();
        std::shared_ptr<arrow::Int64Scalar> intScalar = std::dynamic_pointer_cast<arrow::Int64Scalar>( scalar );
        EXPECT_TRUE( scalar->Equals( arrow::Int64Scalar( i ) ) );
    }
}
