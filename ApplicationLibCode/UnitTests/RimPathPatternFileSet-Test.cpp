#include "gtest/gtest.h"

#include "Tools/RimPathPatternFileSet.h"

namespace internal
{
QString placeholderText()
{
    return "$(INDEX)";
}
} // namespace internal

TEST( RimPathPatternFileSetTest, OneVaryingNumberMultipleLocations )
{
    QStringList filePaths = { "file_1/path-02/real-1.txt", "file_2/path-02/real-2.txt", "file_3/path-02/real-3.txt", "file_13/path-02/real-13.txt" };
    auto result = RimPathPatternFileSet::findPathPattern( filePaths, internal::placeholderText() );
    EXPECT_STREQ( result.first.toStdString().data(), "file_$(INDEX)/path-02/real-$(INDEX).txt" );
    EXPECT_STREQ( result.second.toStdString().data(), "1-3, 13" );

    auto paths = RimPathPatternFileSet::createPathsFromPattern( result, internal::placeholderText() );
    EXPECT_EQ( paths.size(), 4 );
    EXPECT_STREQ( paths[0].toStdString().data(), "file_1/path-02/real-1.txt" );
    EXPECT_STREQ( paths[1].toStdString().data(), "file_2/path-02/real-2.txt" );
    EXPECT_STREQ( paths[2].toStdString().data(), "file_3/path-02/real-3.txt" );
    EXPECT_STREQ( paths[3].toStdString().data(), "file_13/path-02/real-13.txt" );
}

TEST( RimPathPatternFileSetTest, OneVaryingNumber )
{
    QStringList filePaths = { "file_001.txt", "file_002.txt", "file_003.txt" };
    auto        result    = RimPathPatternFileSet::findPathPattern( filePaths, internal::placeholderText() );
    EXPECT_STREQ( result.first.toStdString().data(), "file_$(INDEX).txt" );
    EXPECT_STREQ( result.second.toStdString().data(), "1-3" );
}

TEST( RimPathPatternFileSetTest, EmptyInput )
{
    QStringList filePaths;
    auto        result = RimPathPatternFileSet::findPathPattern( filePaths, internal::placeholderText() );
    EXPECT_TRUE( result.first.isEmpty() );
    EXPECT_TRUE( result.second.isEmpty() );
}

TEST( RimPathPatternFileSetTest, NoVaryingNumbers )
{
    QStringList filePaths = { "file_123.txt", "file_123.txt" };
    auto        result    = RimPathPatternFileSet::findPathPattern( filePaths, internal::placeholderText() );
    EXPECT_TRUE( result.first.isEmpty() );
    EXPECT_TRUE( result.second.isEmpty() );
}
