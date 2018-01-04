#include "gtest/gtest.h"

#include "RiaProjectFileVersionTools.h"
#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST(RiaProjectFileVersionTools, BasicUsage)
{
    /*
        {
            QString projectFileVersionString = "2017.05.1";

            int majorVersion  = 2017;
            int minorVersion  = 5;
            int patchNumber   = 0;
            int developmentId = 0;

            bool isNewer = RiaProjectFileVersionTools::isProjectFileVersionNewerThan(projectFileVersionString, majorVersion,
                                                                                     minorVersion, patchNumber, developmentId);
            EXPECT_TRUE(isNewer);
        }

        {
            QString projectFileVersionString = "2017.05.1.text.13";

            {
                int majorVersion  = 2017;
                int minorVersion  = 5;
                int patchNumber   = 1;
                int developmentId = 14;

                bool isNewer = RiaProjectFileVersionTools::isProjectFileVersionNewerThan(projectFileVersionString, majorVersion,
                                                                                         minorVersion, patchNumber,
       developmentId); EXPECT_FALSE(isNewer);
            }

            {
                int majorVersion  = 2017;
                int minorVersion  = 5;
                int patchNumber   = 1;
                int developmentId = 13;

                bool isNewer = RiaProjectFileVersionTools::isProjectFileVersionNewerThan(projectFileVersionString, majorVersion,
                                                                                         minorVersion, patchNumber,
       developmentId); EXPECT_TRUE(isNewer);
            }

            {
                int majorVersion  = 2017;
                int minorVersion  = 5;
                int patchNumber   = 1;
                int developmentId = 12;

                bool isNewer = RiaProjectFileVersionTools::isProjectFileVersionNewerThan(projectFileVersionString, majorVersion,
                                                                                         minorVersion, patchNumber,
       developmentId); EXPECT_TRUE(isNewer);
            }
        }
    */
}

TEST(RiaProjectFileVersionTools, DecodeProjectVersionString)
{
    {
        int majorVersion  = -1;
        int minorVersion  = -1;
        int patchNumber   = -1;
        int developmentId = -1;

        QString projectFileVersionString = "2017.05.1";
        RiaProjectFileVersionTools::decodeVersionString(projectFileVersionString, &majorVersion, &minorVersion, &patchNumber,
                                                        &developmentId);

        EXPECT_EQ(2017, majorVersion);
        EXPECT_EQ(5, minorVersion);
        EXPECT_EQ(1, patchNumber);
        EXPECT_EQ(-1, developmentId);
    }

    {
        int majorVersion  = -1;
        int minorVersion  = -1;
        int patchNumber   = -1;
        int developmentId = -1;

        QString projectFileVersionString = "";
        RiaProjectFileVersionTools::decodeVersionString(projectFileVersionString, &majorVersion, &minorVersion, &patchNumber,
                                                        &developmentId);

        EXPECT_EQ(-1, majorVersion);
        EXPECT_EQ(-1, minorVersion);
        EXPECT_EQ(-1, patchNumber);
        EXPECT_EQ(-1, developmentId);
    }

    {
        int majorVersion  = -1;
        int minorVersion  = -1;
        int patchNumber   = -1;
        int developmentId = -1;

        QString projectFileVersionString = "2017.05.2-dev.23";
        RiaProjectFileVersionTools::decodeVersionString(projectFileVersionString, &majorVersion, &minorVersion, &patchNumber,
                                                        &developmentId);

        EXPECT_EQ(2017, majorVersion);
        EXPECT_EQ(5, minorVersion);
        EXPECT_EQ(2, patchNumber);
        EXPECT_EQ(23, developmentId);
    }

    {
        int majorVersion  = -1;
        int minorVersion  = -1;
        int patchNumber   = -1;
        int developmentId = -1;

        QString projectFileVersionString = "2017.05.2-dev.long.text..23";
        RiaProjectFileVersionTools::decodeVersionString(projectFileVersionString, &majorVersion, &minorVersion, &patchNumber,
                                                        &developmentId);

        EXPECT_EQ(2017, majorVersion);
        EXPECT_EQ(5, minorVersion);
        EXPECT_EQ(2, patchNumber);
        EXPECT_EQ(23, developmentId);
    }
}

TEST(RiaProjectFileVersionTools, OrderKnownVersionStrings)
{
    QStringList versionStrings;
    {
        // The following list is taken from traversing history of ResInsightVersion.cmake

        versionStrings << "2017.05.2-dev.15";
        versionStrings << "2017.05.2-dev.14";
        versionStrings << "2017.05.2-dev.13";
        versionStrings << "2017.05.2-dev.12";
        versionStrings << "2017.05.2-dev.11";
        versionStrings << "2017.05.2-dev.10";
        versionStrings << "2017.05.2-dev.09";
        versionStrings << "2017.05.2-dev.08";
        versionStrings << "2017.05.2-dev.07";
        versionStrings << "2017.05.2-dev.06";
        versionStrings << "2017.05.2-dev.05";
        versionStrings << "2017.05.2-dev.04";
        versionStrings << "2017.05.2-dev.03";
        versionStrings << "2017.05.2-dev.02";
        versionStrings << "2017.05.2-fdev.02";
        versionStrings << "2017.05.2-dev.1";
        versionStrings << "2017.05.2-fdev.01";
        versionStrings << "2017.05.2";
        versionStrings << "2017.05.pre-proto.15";
        versionStrings << "2017.05.1-dev";
        versionStrings << "2017.05.1";
        versionStrings << "2017.05.0";

        versionStrings << "2016.11.flow.14";
        versionStrings << "2016.11.flow.12";
        versionStrings << "2016.11.flow.11";
        versionStrings << "2016.11.flow.9";
        versionStrings << "2016.11.flow.8";
        versionStrings << "2016.11.flow.7";
        versionStrings << "2016.11.flow.1";
        versionStrings << "2016.11.m.1";
        versionStrings << "2016.11.0";

        versionStrings << "1.6.10-dev";
        versionStrings << "1.6.9-dev";
        versionStrings << "1.6.8-dev";
        versionStrings << "1.6.7-gm-beta";
        versionStrings << "1.6.6-dev";
        versionStrings << "1.6.5-dev";
        versionStrings << "1.6.4-dev";
        versionStrings << "1.6.3-dev";
        versionStrings << "1.6.1-dev";
        versionStrings << "1.6.2-dev";
        versionStrings << "1.6.0-RC";

        versionStrings << "1.5.111-RC";
        versionStrings << "1.5.110-RC";
        versionStrings << "1.5.109-RC";
        versionStrings << "1.5.108-RC";
        versionStrings << "1.5.107-RC";
        versionStrings << "1.5.106-RC";
        versionStrings << "1.5.105-RC";
        versionStrings << "1.5.104-RC";
        versionStrings << "1.5.103-dev";
        versionStrings << "1.5.102-dev";
        versionStrings << "1.5.101-dev";
        versionStrings << "1.5.100-dev";
        versionStrings << "1.5.0";
    }

    // Additional dummy test versions
    versionStrings << "2016.12";
    versionStrings << "2015";
    versionStrings << "2016.10.1.sd flkj....03";
    versionStrings << "2016.10.1.sdf lkj. ...04";

    QStringList sortedVersionList = versionStrings;
    {
        qSort(sortedVersionList.begin(), sortedVersionList.end(),
              RiaProjectFileVersionTools::isCandidateVersionNewerThanOther);
    }

    for (const auto& s : sortedVersionList)
    {
        // std::cout << s.toStdString() << "\n";
    }
}
