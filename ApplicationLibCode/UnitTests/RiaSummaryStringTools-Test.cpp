#include "gtest/gtest.h"

#include "RiaSummaryStringTools.h"

#include <QString>
#include <QStringList>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RiaSummaryStringToolsTest, ParseNumbers )
{
    QString     wellFilter      = "wopt:op_*";
    QString     fieldFilter     = "fopt";
    QStringList dataSourceNames = { "iter-1", "iter-22", "real-10", "real-11" };

    {
        QString     dataSourceFilter = "iter-1";
        QStringList arguments        = { wellFilter, fieldFilter, dataSourceFilter };

        QStringList addressFilters;
        QStringList dataSourceFilters;

        RiaSummaryStringTools::splitIntoAddressAndDataSourceFilters( arguments,
                                                                     dataSourceNames,
                                                                     addressFilters,
                                                                     dataSourceFilters );

        EXPECT_TRUE( addressFilters[0] == wellFilter );
        EXPECT_TRUE( addressFilters[1] == fieldFilter );

        EXPECT_TRUE( dataSourceFilters[0] == dataSourceFilter );
    }

    {
        QString     dataSourceFilter = "iter-22:real-2*";
        QStringList arguments        = { wellFilter, fieldFilter, dataSourceFilter };

        QStringList addressFilters;
        QStringList dataSourceFilters;

        RiaSummaryStringTools::splitIntoAddressAndDataSourceFilters( arguments,
                                                                     dataSourceNames,
                                                                     addressFilters,
                                                                     dataSourceFilters );

        EXPECT_TRUE( addressFilters[0] == wellFilter );
        EXPECT_TRUE( addressFilters[1] == fieldFilter );

        EXPECT_TRUE( dataSourceFilters[0] == dataSourceFilter );
    }
}
