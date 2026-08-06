#include "gtest/gtest.h"

#include "Tools/Ensemble/RiaEnsembleImportTools.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

namespace internal
{
QString placeholderText()
{
    return "$(INDEX)";
}

void createEmptyFile( const QString& filePath )
{
    QDir().mkpath( QFileInfo( filePath ).absolutePath() );

    QFile file( filePath );
    if ( file.open( QIODevice::WriteOnly ) ) file.close();
}
} // namespace internal

TEST( RimPathPatternFileSetTest, OneVaryingNumberRealizations )
{
    QStringList filePaths = { "drogon-varying-grid-geometry/realization-0/iter-0/eclipse/model/DROGON-0.EGRID",
                              "drogon-varying-grid-geometry/realization-1/iter-0/eclipse/model/DROGON-1.EGRID",
                              "drogon-varying-grid-geometry/realization-2/iter-0/eclipse/model/DROGON-2.EGRID",
                              "drogon-varying-grid-geometry/realization-3/iter-0/eclipse/model/DROGON-3.EGRID",
                              "drogon-varying-grid-geometry/realization-13/iter-0/eclipse/model/DROGON-13.EGRID" };

    const auto& [basePath, numberRange] = RiaEnsembleImportTools::findPathPattern( filePaths, internal::placeholderText() );
    EXPECT_STREQ( basePath.toStdString().data(),
                  "drogon-varying-grid-geometry/realization-$(INDEX)/iter-0/eclipse/model/DROGON-$(INDEX).EGRID" );
    EXPECT_STREQ( numberRange.toStdString().data(), "0-3, 13" );

    const auto paths = RiaEnsembleImportTools::createPathsFromPattern( basePath, numberRange, internal::placeholderText() );
    EXPECT_EQ( paths.size(), 5 );
    if ( paths.size() != filePaths.size() ) return;
    for ( auto i = 0; i < paths.size(); i++ )
    {
        EXPECT_STREQ( paths[i].toStdString().data(), filePaths[i].toStdString().data() );
    }
}

TEST( RimPathPatternFileSetTest, FolderNameContainingDot )
{
    QStringList filePaths = { "/scratch/fmu/user/drogon.2024/realization-0/iter-0/eclipse/model/DROGON-0.ESMRY",
                              "/scratch/fmu/user/drogon.2024/realization-1/iter-0/eclipse/model/DROGON-1.ESMRY",
                              "/scratch/fmu/user/drogon.2024/realization-2/iter-0/eclipse/model/DROGON-2.ESMRY" };

    const auto& [pattern, numberRange] = RiaEnsembleImportTools::findPathPattern( filePaths, internal::placeholderText() );
    EXPECT_STREQ( pattern.toStdString().data(),
                  "/scratch/fmu/user/drogon.2024/realization-$(INDEX)/iter-0/eclipse/model/DROGON-$(INDEX).ESMRY" );
    EXPECT_STREQ( numberRange.toStdString().data(), "0-2" );

    const auto paths = RiaEnsembleImportTools::createPathsFromPattern( pattern, numberRange, internal::placeholderText() );
    EXPECT_EQ( paths.size(), filePaths.size() );
    if ( paths.size() != filePaths.size() ) return;
    for ( auto i = 0; i < paths.size(); i++ )
    {
        EXPECT_STREQ( paths[i].toStdString().data(), filePaths[i].toStdString().data() );
    }
}

TEST( RimPathPatternFileSetTest, SearchFileSystemFolderNameContainingDot )
{
    QTemporaryDir tempDir;
    ASSERT_TRUE( tempDir.isValid() );

    const QString rootFolder = QDir::fromNativeSeparators( tempDir.path() ) + "/drogon.2024";

    for ( int realizationNumber = 0; realizationNumber < 3; realizationNumber++ )
    {
        const QString filePath =
            QString( "%1/realization-%2/iter-0/eclipse/model/DROGON-%2.SMSPEC" ).arg( rootFolder ).arg( realizationNumber );
        internal::createEmptyFile( filePath );
    }

    const QString pathPattern = rootFolder + "/realization-*/iter-0/eclipse/model/DROGON-*";

    const auto paths = RiaEnsembleImportTools::createPathsBySearchingFileSystem( pathPattern, ".SMSPEC", "*" );
    EXPECT_EQ( paths.size(), 3 );
}

TEST( RimPathPatternFileSetTest, OneVaryingNumberMultipleLocations )
{
    QStringList filePaths = { "file_1/path-02/real-1.txt", "file_2/path-02/real-2.txt", "file_3/path-02/real-3.txt", "file_13/path-02/real-13.txt" };
    const auto& [basePath, numberRange] = RiaEnsembleImportTools::findPathPattern( filePaths, internal::placeholderText() );
    EXPECT_STREQ( basePath.toStdString().data(), "file_$(INDEX)/path-02/real-$(INDEX).txt" );
    EXPECT_STREQ( numberRange.toStdString().data(), "1-3, 13" );

    const auto paths = RiaEnsembleImportTools::createPathsFromPattern( basePath, numberRange, internal::placeholderText() );
    EXPECT_EQ( paths.size(), 4 );
    EXPECT_STREQ( paths[0].toStdString().data(), "file_1/path-02/real-1.txt" );
    EXPECT_STREQ( paths[1].toStdString().data(), "file_2/path-02/real-2.txt" );
    EXPECT_STREQ( paths[2].toStdString().data(), "file_3/path-02/real-3.txt" );
    EXPECT_STREQ( paths[3].toStdString().data(), "file_13/path-02/real-13.txt" );
}

TEST( RimPathPatternFileSetTest, OneVaryingNumber )
{
    QStringList filePaths = { "file_001.txt", "file_002.txt", "file_003.txt" };
    auto        result    = RiaEnsembleImportTools::findPathPattern( filePaths, internal::placeholderText() );
    EXPECT_STREQ( result.first.toStdString().data(), "file_$(INDEX).txt" );
    EXPECT_STREQ( result.second.toStdString().data(), "1-3" );
}

TEST( RimPathPatternFileSetTest, EmptyInput )
{
    QStringList filePaths;
    auto        result = RiaEnsembleImportTools::findPathPattern( filePaths, internal::placeholderText() );
    EXPECT_TRUE( result.first.isEmpty() );
    EXPECT_TRUE( result.second.isEmpty() );
}

TEST( RimPathPatternFileSetTest, NoVaryingNumbers )
{
    QStringList filePaths = { "file_123.txt", "file_123.txt" };
    auto        result    = RiaEnsembleImportTools::findPathPattern( filePaths, internal::placeholderText() );
    EXPECT_TRUE( result.first.isEmpty() );
    EXPECT_TRUE( result.second.isEmpty() );
}
