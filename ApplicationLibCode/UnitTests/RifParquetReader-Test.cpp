#include "gtest/gtest.h"

#include "RiaTestDataDirectory.h"

#include "RifArrowTools.h"

#undef signals
#include <arrow/array/builder_primitive.h>
#include <arrow/csv/api.h>
#include <arrow/io/api.h>
#include <arrow/scalar.h>
#include <parquet/arrow/reader.h>
#define signals Q_SIGNALS

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

TEST( RifParquetReaderTest, ConvertIntChunkedArrays )
{
    arrow::Status status;

    arrow::Int32Builder int_builder;
    status = int_builder.Append( 1 );
    status = int_builder.Append( 2 );
    status = int_builder.Append( 3 );

    std::shared_ptr<arrow::Array> int_array;
    status = int_builder.Finish( &int_array );

    auto int_chunked_array = std::make_shared<arrow::ChunkedArray>( int_array );

    {
        auto columnVector = RifArrowTools::chunkedArrayToVector<arrow::FloatArray, double>( int_chunked_array );
        EXPECT_EQ( columnVector.size(), 3 );
    }
    {
        auto columnVector = RifArrowTools::chunkedArrayToVector<arrow::Int32Array, int>( int_chunked_array );
        EXPECT_EQ( columnVector.size(), 3 );
    }
}

TEST( RifParquetReaderTest, ConvertFloatChunkedArrays )
{
    arrow::Status status;

    // Create an Arrow double array
    std::vector<double>           values = { 1.0, 2.0, 3.0, 4.0 };
    std::shared_ptr<arrow::Array> array;
    arrow::DoubleBuilder          builder;
    status = builder.AppendValues( values );
    status = builder.Finish( &array );

    // Create a chunked array from the Arrow array
    std::shared_ptr<arrow::ChunkedArray> chunkedArray = std::make_shared<arrow::ChunkedArray>( array );

    // Call the function under test
    auto resultVector = RifArrowTools::chunkedArrayToVector<arrow::DoubleArray, double>( chunkedArray );

    // Assert that the returned vector contains the expected values
    ASSERT_EQ( resultVector.size(), values.size() );
    for ( size_t i = 0; i < values.size(); ++i )
    {
        EXPECT_DOUBLE_EQ( resultVector[i], values[i] );
    }

    auto floatVector = RifArrowTools::chunkedArrayToVector<arrow::DoubleArray, float>( chunkedArray );
    ASSERT_EQ( floatVector.size(), values.size() );
    for ( size_t i = 0; i < values.size(); ++i )
    {
        EXPECT_DOUBLE_EQ( floatVector[i], values[i] );
    }
}
