#include "gtest/gtest.h"

#include "RifEclipseDataTableFormatter.h"

TEST(RifEclipseDataTableFormatter, BasicUsage)
{
    QString tableText;
    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);


    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("Well"),
        RifEclipseOutputTableColumn("Integer Number"),
        RifEclipseOutputTableColumn("IntNumer 2"),
        RifEclipseOutputTableColumn("IntNumer 3"),
    };

    formatter.header(header);

    formatter.add("well a");
    formatter.add(1);
    formatter.add(2);
    formatter.add(3);
    formatter.rowCompleted();

    formatter.add("well B");
    formatter.add(12);
    formatter.add(23);
    formatter.add(233);
    formatter.rowCompleted();

    formatter.tableCompleted();

    std::cout << tableText.toStdString();

}


TEST(RifEclipseDataTableFormatter, NoPrefix)
{
    QString tableText;
    QTextStream                  stream(&tableText);
    RifEclipseDataTableFormatter formatter(stream);

    formatter.setTableRowPrependText("   ");
    formatter.setTableRowLineAppendText("");


    std::vector<RifEclipseOutputTableColumn> header = {
        RifEclipseOutputTableColumn("Well"),
        RifEclipseOutputTableColumn("Integer Number", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("IntNumer 2", RifEclipseOutputTableDoubleFormatting(), RIGHT),
        RifEclipseOutputTableColumn("IntNumer 3", RifEclipseOutputTableDoubleFormatting(), RIGHT),
    };

    formatter.header(header);

    formatter.add("well a");
    formatter.add(1);
    formatter.add(2);
    formatter.add(3);
    formatter.rowCompleted();

    formatter.add("well B");
    formatter.add(12);
    formatter.add(231);
    formatter.add(23123);
    formatter.rowCompleted();

    formatter.tableCompleted();

    std::cout << tableText.toStdString();

}
