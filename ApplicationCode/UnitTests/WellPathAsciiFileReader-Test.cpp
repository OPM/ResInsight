#include "gtest/gtest.h"

#include "RifWellPathImporter.h"

#include <QTemporaryFile>
#include <QTextStream>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RimWellPathAsciiFileReaderTest, TestWellNameNoColon)
{
    QTemporaryFile file;
    if (file.open())
    {
        QString wellName = "My test Wellname";
        {
            QTextStream out(&file);
            out << "name " << wellName << "\n";
            out << "1 2 3";
        }

        RifWellPathImporter reader;
        RifWellPathImporter::WellData wpData = reader.readWellData(file.fileName(), 0);
        EXPECT_TRUE(wpData.m_name == wellName);
     }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RimWellPathAsciiFileReaderTest, TestWellNameWithColon)
{
    QTemporaryFile file;
    if (file.open())
    {
        QString wellName = "My test Wellname";
        {
            QTextStream out(&file);
            out << "WELLNAME:" << wellName << "\n";
            out << "1 2 3";
        }

        RifWellPathImporter reader;
        RifWellPathImporter::WellData wpData = reader.readWellData(file.fileName(), 0);
        EXPECT_TRUE(wpData.m_name == wellName);
     }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RimWellPathAsciiFileReaderTest, TestWellNameWithColonAndSpace)
{
    QTemporaryFile file;
    if (file.open())
    {
        QString wellName = "My test Wellname";
        {
            QTextStream out(&file);
            out << "WELLNAME  :  " << wellName << "\n";
            out << "1 2 3";
        }

        RifWellPathImporter reader;
        RifWellPathImporter::WellData wpData = reader.readWellData(file.fileName(), 0);
        EXPECT_TRUE(wpData.m_name == wellName);
     }
}
