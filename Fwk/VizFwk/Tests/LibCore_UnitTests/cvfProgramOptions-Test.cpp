//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfBase.h"
#include "cvfProgramOptions.h"
#include "cvfTrace.h"

#include "gtest/gtest.h"

using namespace cvf;



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OptionTest, DefaultConstructionInvalidObj)
{
    Option o;
    EXPECT_FALSE(o.isValid());
    EXPECT_FALSE(o);
    EXPECT_TRUE(o.name() == "");
    EXPECT_EQ(0, o.valueCount());
    EXPECT_EQ(0, o.values().size());
    EXPECT_TRUE(o.combinedValues() == "");
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(OptionTest, Construction)
{
    std::vector<String> vals;
    vals.push_back("v0");
    vals.push_back("v1");
    Option o("dummy", vals);
    EXPECT_TRUE(o.isValid());
    EXPECT_TRUE(o);
    EXPECT_TRUE(o.name() == "dummy");
    EXPECT_EQ(2, o.valueCount());
    ASSERT_EQ(2, o.values().size());
    EXPECT_TRUE(o.values()[0] == "v0");
    EXPECT_TRUE(o.values()[1] == "v1");
    EXPECT_TRUE(o.combinedValues() == "v0 v1");
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, Construction)
{
    ProgramOptions po;
    ASSERT_FALSE(po.hasOption("dummy"));
    ASSERT_TRUE(po.firstValue("dummy") == "");
    ASSERT_EQ(0, po.values("dummy").size());

    Option o = po.option("dummy");
    ASSERT_FALSE(o.isValid());
    ASSERT_EQ(0, o.valueCount());
    ASSERT_TRUE(o.combinedValues() == "");
    ASSERT_TRUE(po.option("dummy").combinedValues() == "");

    ASSERT_EQ(0, po.positionalParameters().size());
    ASSERT_EQ(0, po.unknownOptions().size());
    ASSERT_EQ(0, po.optionsWithMissingValues().size());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, EmptyOptions)
{
    ProgramOptions po;
    po.registerOption("flag1");
    po.registerOption("");

    String cmdLine("MyExe  --flag1 -- file1.txt");
    std::vector<String> cmdLineArgs = cmdLine.split();
    ASSERT_TRUE(po.parse(cmdLineArgs));

    EXPECT_TRUE(po.hasOption("flag1"));

    ASSERT_EQ(1, po.positionalParameters().size());
    EXPECT_TRUE(po.positionalParameters()[0] == "file1.txt");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, SimpleFlagOptions)
{
    ProgramOptions po;
    po.registerOption("flag1");
    po.registerOption("flag2");
    po.registerOption("flag3");

    String cmdLine("MyExe  --flag3 --flag1");
    std::vector<String> cmdLineArgs = cmdLine.split();
    ASSERT_TRUE(po.parse(cmdLineArgs));

    EXPECT_TRUE(po.hasOption("flag1"));
    EXPECT_TRUE(po.hasOption("flag3"));

    EXPECT_FALSE(po.hasOption("flag2"));
    EXPECT_FALSE(po.hasOption("flag0"));
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, SingleValueOptions)
{
    ProgramOptions po;
    po.registerOption("opt1", ProgramOptions::SINGLE_VALUE);
    po.registerOption("opt2", ProgramOptions::SINGLE_VALUE);
    po.registerOption("opt3", ProgramOptions::SINGLE_VALUE);

    String cmdLine("MyExe --opt1 AA --opt2 BB");
    std::vector<String> cmdLineArgs = cmdLine.split();
    ASSERT_TRUE(po.parse(cmdLineArgs));

    ASSERT_TRUE(po.hasOption("opt1"));
    ASSERT_EQ(1, po.values("opt1").size());
    EXPECT_TRUE(po.values("opt1")[0] == "AA");

    EXPECT_TRUE(po.hasOption("opt2"));
    ASSERT_EQ(1, po.values("opt2").size());
    EXPECT_TRUE(po.values("opt2")[0] == "BB");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, MultiValueOptions)
{
    ProgramOptions po;
    po.registerOption("opt1", ProgramOptions::MULTI_VALUE);
    po.registerOption("opt2", ProgramOptions::MULTI_VALUE);
    po.registerOption("opt3", ProgramOptions::MULTI_VALUE);

    String cmdLine("MyExe --opt1 A0 --opt2 B0 B1 --opt3 C0 C1 C2");
    std::vector<String> cmdLineArgs = cmdLine.split();
    ASSERT_TRUE(po.parse(cmdLineArgs));

    ASSERT_TRUE(po.hasOption("opt1"));
    ASSERT_EQ(1, po.values("opt1").size());
    EXPECT_TRUE(po.values("opt1")[0] == "A0");

    EXPECT_TRUE(po.hasOption("opt2"));
    ASSERT_EQ(2, po.values("opt2").size());
    EXPECT_TRUE(po.values("opt2")[0] == "B0");
    EXPECT_TRUE(po.values("opt2")[1] == "B1");

    EXPECT_TRUE(po.hasOption("opt3"));
    ASSERT_EQ(3, po.values("opt3").size());
    EXPECT_TRUE(po.values("opt3")[0] == "C0");
    EXPECT_TRUE(po.values("opt3")[1] == "C1");
    EXPECT_TRUE(po.values("opt3")[2] == "C2");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, SingleValueOptionWithMissingValue)
{
    ProgramOptions po;
    po.registerOption("opt1", ProgramOptions::SINGLE_VALUE);
    po.registerOption("opt2", ProgramOptions::SINGLE_VALUE);
    po.registerOption("opt3", ProgramOptions::SINGLE_VALUE);

    String cmdLine("MyExe --opt1 1 --opt2  --opt3 3");
    std::vector<String> cmdLineArgs = cmdLine.split();
    ASSERT_FALSE(po.parse(cmdLineArgs));

    ASSERT_TRUE(po.hasOption("opt1"));
    ASSERT_EQ(1, po.values("opt1").size());
    EXPECT_TRUE(po.values("opt1")[0] == "1");

    EXPECT_FALSE(po.hasOption("opt2"));
    ASSERT_EQ(0, po.values("opt2").size());
    ASSERT_EQ(1, po.optionsWithMissingValues().size());
    EXPECT_TRUE(po.optionsWithMissingValues()[0] == "opt2");

    EXPECT_TRUE(po.hasOption("opt3"));
    ASSERT_EQ(1, po.values("opt3").size());
    EXPECT_TRUE(po.values("opt3")[0] == "3");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, MultiValueOptionWithMissingValues)
{
    ProgramOptions po;
    po.registerOption("opt1", ProgramOptions::MULTI_VALUE);
    po.registerOption("opt2", ProgramOptions::MULTI_VALUE);
    po.registerOption("opt3", ProgramOptions::MULTI_VALUE);

    String cmdLine("MyExe --opt3 1 2 3 --opt2  --opt1 1");
    std::vector<String> cmdLineArgs = cmdLine.split();
    ASSERT_FALSE(po.parse(cmdLineArgs));

    ASSERT_TRUE(po.hasOption("opt1"));
    ASSERT_EQ(1, po.values("opt1").size());
    EXPECT_TRUE(po.values("opt1")[0] == "1");

    EXPECT_FALSE(po.hasOption("opt2"));
    ASSERT_EQ(0, po.values("opt2").size());
    ASSERT_EQ(1, po.optionsWithMissingValues().size());
    EXPECT_TRUE(po.optionsWithMissingValues()[0] == "opt2");

    EXPECT_TRUE(po.hasOption("opt3"));
    ASSERT_EQ(3, po.values("opt3").size());
    EXPECT_TRUE(po.values("opt3")[0] == "1");
    EXPECT_TRUE(po.values("opt3")[1] == "2");
    EXPECT_TRUE(po.values("opt3")[2] == "3");

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, UnknownOptions)
{
    ProgramOptions po;
    po.registerOption("noval",  ProgramOptions::NO_VALUE);
    po.registerOption("single", ProgramOptions::SINGLE_VALUE);
    po.registerOption("multi",  ProgramOptions::MULTI_VALUE);

    String cmdLine("MyExe --u1 v1 --multi 1 2 --u2 v2 --noval f1 --u3 --single 1 --u4 f2");
    std::vector<String> cmdLineArgs = cmdLine.split();
    ASSERT_FALSE(po.parse(cmdLineArgs));

    ASSERT_TRUE(po.hasOption("noval"));
    ASSERT_EQ(0, po.values("noval").size());
    ASSERT_TRUE(po.hasOption("single"));
    ASSERT_EQ(1, po.values("single").size());
    ASSERT_TRUE(po.hasOption("multi"));
    ASSERT_EQ(2, po.values("multi").size());

    EXPECT_FALSE(po.hasOption("u1"));
    EXPECT_FALSE(po.hasOption("u2"));
    EXPECT_FALSE(po.hasOption("u3"));
    EXPECT_FALSE(po.hasOption("u4"));
    EXPECT_FALSE(po.option("u1"));
    EXPECT_FALSE(po.option("u2"));
    EXPECT_FALSE(po.option("u3"));
    EXPECT_FALSE(po.option("u4"));

    ASSERT_EQ(4, po.unknownOptions().size());
    EXPECT_TRUE(po.unknownOptions()[0] == "u1");
    EXPECT_TRUE(po.unknownOptions()[1] == "u2");
    EXPECT_TRUE(po.unknownOptions()[2] == "u3");
    EXPECT_TRUE(po.unknownOptions()[3] == "u4");

    ASSERT_EQ(4, po.positionalParameters().size());
    EXPECT_TRUE(po.positionalParameters()[0] == "v1");
    EXPECT_TRUE(po.positionalParameters()[1] == "v2");
    EXPECT_TRUE(po.positionalParameters()[2] == "f1");
    EXPECT_TRUE(po.positionalParameters()[3] == "f2");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, RepeatOptions)
{
    ProgramOptions po;
    po.setOptionPrefix(ProgramOptions::SINGLE_DASH);
    po.registerOption("single", ProgramOptions::SINGLE_VALUE);
    po.registerOption("multi", ProgramOptions::MULTI_VALUE);

    String cmdLine("MyExe  -single 1  -multi A  -multi A B  -single 2");
    std::vector<String> cmdLineArgs = cmdLine.split();
    ASSERT_TRUE(po.parse(cmdLineArgs));

    ASSERT_TRUE(po.hasOption("single"));
    ASSERT_EQ(1, po.values("single").size());
    ASSERT_TRUE(po.option("single").combinedValues() == "2");

    ASSERT_TRUE(po.hasOption("multi"));
    ASSERT_EQ(2, po.values("multi").size());
    ASSERT_TRUE(po.option("multi").combinedValues() == "A B");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, RepeatOptionsCombining)
{
    ProgramOptions po;
    po.setOptionPrefix(ProgramOptions::SINGLE_DASH);
    po.registerOption("single", ProgramOptions::SINGLE_VALUE, ProgramOptions::COMBINE_REPEATED);
    po.registerOption("multi",  ProgramOptions::MULTI_VALUE, ProgramOptions::COMBINE_REPEATED);

    String cmdLine("MyExe  -single 1  -multi A  -multi A B  -single 2");
    std::vector<String> cmdLineArgs = cmdLine.split();
    ASSERT_TRUE(po.parse(cmdLineArgs));

    ASSERT_TRUE(po.hasOption("single"));
    ASSERT_EQ(2, po.values("single").size());
    ASSERT_TRUE(po.option("single").combinedValues() == "1 2");

    ASSERT_TRUE(po.hasOption("multi"));
    ASSERT_EQ(3, po.values("multi").size());
    ASSERT_TRUE(po.option("multi").combinedValues() == "A A B");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, RealWorld_1)
{
    ProgramOptions po;
    po.registerOption("calculate", "",                "", ProgramOptions::NO_VALUE);
    po.registerOption("size",      "<x> <y>",         "", ProgramOptions::MULTI_VALUE);
    po.registerOption("replace",   "<id> <fileName>", "", ProgramOptions::MULTI_VALUE);
    po.registerOption("search",    "<filters>",       "", ProgramOptions::MULTI_VALUE);
    po.registerOption("outFile",   "<fileName>",      "", ProgramOptions::SINGLE_VALUE);

    String cmdLine("MyExe fileName.txt --size 10 20 --calculate --outfile outfile.txt  --replace 99 replace.txt --search *.txt case???.bin");
    std::vector<String> cmdLineArgs = cmdLine.split();
    ASSERT_TRUE(po.parse(cmdLineArgs));

    ASSERT_EQ(1, po.positionalParameters().size());
    EXPECT_TRUE(po.positionalParameters()[0] == "fileName.txt");

    ASSERT_TRUE(po.hasOption("calculate"));
    
    {
        ASSERT_TRUE(po.hasOption("size"));
        std::vector<String> vals = po.values("size");
        ASSERT_EQ(2, vals.size());
        int x = vals[0].toInt();
        int y = vals[1].toInt();
        EXPECT_EQ(10, x);
        EXPECT_EQ(20, y);
    }

    {
        ASSERT_TRUE(po.hasOption("replace"));
        std::vector<String> vals = po.values("replace");
        ASSERT_EQ(2, vals.size());
        int id = vals[0].toInt();
        String fileName = vals[1];
        EXPECT_EQ(99, id);
        EXPECT_TRUE(fileName == "replace.txt");
    }

    {
        ASSERT_TRUE(po.hasOption("search"));
        std::vector<String> vals = po.values("search");
        ASSERT_EQ(2, vals.size());
        EXPECT_TRUE(vals[0] == "*.txt");
        EXPECT_TRUE(vals[1] == "case???.bin");
    }

    {
        ASSERT_TRUE(po.hasOption("outFile"));
        String fileName = po.firstValue("outFile");
        EXPECT_TRUE(fileName == "outfile.txt");
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, RealWorld_2)
{
    ProgramOptions po;
    po.setOptionPrefix(ProgramOptions::SLASH);
    po.registerOption("version", "", "",             ProgramOptions::NO_VALUE);
    po.registerOption("width", "<width>", "",        ProgramOptions::SINGLE_VALUE);
    po.registerOption("outFiles", "<fileNames>", "", ProgramOptions::MULTI_VALUE, ProgramOptions::COMBINE_REPEATED);

    String cmdLine("MyExe  file1.txt file2.txt  /width 25  /outFiles res\\out1.txt res\\out2.txt  /version  /outFiles c:\\out3.txt");
    std::vector<String> cmdLineArgs = cmdLine.split();
    ASSERT_TRUE(po.parse(cmdLineArgs));

    ASSERT_EQ(2, po.positionalParameters().size());
    EXPECT_TRUE(po.positionalParameters()[0] == "file1.txt");
    EXPECT_TRUE(po.positionalParameters()[1] == "file2.txt");

    ASSERT_TRUE(po.hasOption("version"));

    {
        ASSERT_TRUE(po.hasOption("width"));
        int width = po.firstValue("width").toInt();
        EXPECT_EQ(25, width);
    }

    {
        ASSERT_TRUE(po.hasOption("outFiles"));
        std::vector<String> vals = po.values("outFiles");
        ASSERT_EQ(3, vals.size());
        EXPECT_TRUE(vals[0] == "res\\out1.txt");
        EXPECT_TRUE(vals[1] == "res\\out2.txt");
        EXPECT_TRUE(vals[2] == "c:\\out3.txt");
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, UsageText)
{
    ProgramOptions po;
    po.registerOption("calculate", "",                "Do the calculation.", ProgramOptions::NO_VALUE);
    po.registerOption("size",      "<x> <y>",         "Set widow size.", ProgramOptions::MULTI_VALUE);
    po.registerOption("replace",   "<id> <filName>",  "Replace file for case with specified id.", ProgramOptions::MULTI_VALUE);
    po.registerOption("search",    "<filters>",       "Search filters", ProgramOptions::MULTI_VALUE);
    po.registerOption("outFile",   "<fileName>",      "Output file name", ProgramOptions::SINGLE_VALUE);

    String usageText = po.usageText(60);
    cvf::Trace::show(usageText);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(ProgramOptionsTest, RealWorld_3)
{
    ProgramOptions po;
    po.registerOption("myFlag1",   "",                  "", ProgramOptions::NO_VALUE);
    po.registerOption("myFlag2",   "",                  "", ProgramOptions::NO_VALUE);
    po.registerOption("size",      "<x> <y>",           "", ProgramOptions::MULTI_VALUE);
    po.registerOption("outFile",   "<fileName>",        "", ProgramOptions::SINGLE_VALUE);

    String cmdLine("MyExe --size 10 20 --myFlag2  --outfile outfile.txt");
    std::vector<String> cmdLineArgs = cmdLine.split();
    ASSERT_TRUE(po.parse(cmdLineArgs));

    if (Option o = po.option("myFlag1"))
    {
        FAIL() << "Option should not be present.";
    }

    if (Option o = po.option("myFlag2"))
    {
        EXPECT_TRUE(o.isValid());
        EXPECT_EQ(0, o.valueCount());
    }
    else
    {
        FAIL() << "Option missing.";
    }

    if (Option o = po.option("size"))
    {
        EXPECT_TRUE(o.isValid());
        EXPECT_EQ(2, o.valueCount());
        int x = o.safeValue(0).toInt(-1);
        int y = o.safeValue(1).toInt(-1);
        EXPECT_EQ(10, x);
        EXPECT_EQ(20, y);
    }
    else
    {
        FAIL() << "Option missing.";
    }

    if (Option o = po.option("outFile"))
    {
        EXPECT_TRUE(o.isValid());
        EXPECT_EQ(1, o.valueCount());
        String fileName = o.combinedValues();
        EXPECT_TRUE(fileName == "outfile.txt");
    }
    else
    {
        FAIL() << "Option missing.";
    }
}


