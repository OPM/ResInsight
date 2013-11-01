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
#include "cvfLogger.h"
#include "cvfLogManager.h"
#include "cvfLogEvent.h"
#include "cvfLogDestinationConsole.h"

#include "gtest/gtest.h"

using namespace cvf;


class MyLogDestination : public LogDestination
{
public:
    MyLogDestination(LogDestination* aRealDestination)
    :   m_aRealDestination(aRealDestination)
    {
    }

    virtual void log(const LogEvent& logEvent) 
    {
        m_lastLogEvent = logEvent;
        if (m_aRealDestination.notNull())
        {
            m_aRealDestination->log(logEvent);
        }
    }

    LogEvent getLastAndClear()
    {
        LogEvent le = m_lastLogEvent;
        m_lastLogEvent = LogEvent();
        return le;
    }

private:
    LogEvent            m_lastLogEvent;
    ref<LogDestination> m_aRealDestination;
};



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(LoggerTest, BasicConstruction)
{
    Logger l("myLogger", Logger::LL_WARNING, NULL);

    EXPECT_STREQ(L"myLogger", l.name().c_str());
    EXPECT_EQ(Logger::LL_WARNING, l.level());
    EXPECT_TRUE(l.destination() == NULL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(LoggerTest, SetAndQueryLogLevel)
{
    ref<Logger> l = new Logger("myLogger", 0, NULL);

    EXPECT_EQ(0, l->level());
    EXPECT_FALSE(l->isErrorEnabled());
    EXPECT_FALSE(l->isWarningEnabled());
    EXPECT_FALSE(l->isInfoEnabled());
    EXPECT_FALSE(l->isDebugEnabled());

    l->setLevel(99);
    EXPECT_EQ(99, l->level());
    EXPECT_TRUE(l->isErrorEnabled());
    EXPECT_TRUE(l->isWarningEnabled());
    EXPECT_TRUE(l->isInfoEnabled());
    EXPECT_TRUE(l->isDebugEnabled());

    l->setLevel(Logger::LL_ERROR);
    EXPECT_EQ(Logger::LL_ERROR, l->level());
    EXPECT_TRUE(l->isErrorEnabled());
    EXPECT_FALSE(l->isWarningEnabled());
    EXPECT_FALSE(l->isInfoEnabled());
    EXPECT_FALSE(l->isDebugEnabled());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(LoggerTest, FullLoggingAtDifferentLevels)
{
    ref<MyLogDestination> dest = new MyLogDestination(NULL);
    ref<Logger> l = new Logger("myLogger", Logger::LL_DEBUG, dest.p());

    {
        l->error("err1");
        LogEvent le = dest->getLastAndClear();
        EXPECT_EQ(Logger::LL_ERROR, le.level());
        EXPECT_STREQ(L"err1", le.message().c_str());
    }
    {
        l->warning("warn1");
        LogEvent le = dest->getLastAndClear();
        EXPECT_EQ(Logger::LL_WARNING, le.level());
        EXPECT_STREQ(L"warn1", le.message().c_str());
    }
    {
        l->info("info1");
        LogEvent le = dest->getLastAndClear();
        EXPECT_EQ(Logger::LL_INFO, le.level());
        EXPECT_STREQ(L"info1", le.message().c_str());
    }
    {
        l->debug("dbg1", CodeLocation());
        LogEvent le = dest->getLastAndClear();
        EXPECT_EQ(Logger::LL_DEBUG, le.level());
        EXPECT_STREQ(L"dbg1", le.message().c_str());
    }

    // Log the same messages after setting log level to errors only
    l->setLevel(Logger::LL_ERROR);

    {
        l->error("err1");
        LogEvent le = dest->getLastAndClear();
        EXPECT_EQ(Logger::LL_ERROR, le.level());
        EXPECT_STREQ(L"err1", le.message().c_str());
    }
    {
        l->warning("warn1");
        LogEvent le = dest->getLastAndClear();
        EXPECT_EQ(Logger::LL_ERROR, le.level());
        EXPECT_STREQ(L"", le.message().c_str());
    }
    {
        l->info("info1");
        LogEvent le = dest->getLastAndClear();
        EXPECT_EQ(Logger::LL_ERROR, le.level());
        EXPECT_STREQ(L"", le.message().c_str());
    }
    {
        l->debug("dbg1", CodeLocation());
        LogEvent le = dest->getLastAndClear();
        EXPECT_EQ(Logger::LL_ERROR, le.level());
        EXPECT_STREQ(L"", le.message().c_str());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(LoggerTest, TestLoggingMacros)
{
    //ref<MyLogDestination> dest = new MyLogDestination(new LogDestinationConsole);
    ref<MyLogDestination> dest = new MyLogDestination(NULL);
    ref<Logger> l = new Logger("myLogger", Logger::LL_DEBUG, dest.p());

    {
        CVF_LOG_ERROR(l, "err1");
        LogEvent le = dest->getLastAndClear();
        EXPECT_EQ(Logger::LL_ERROR, le.level());
        EXPECT_STREQ(L"err1", le.message().c_str());
    }
    {
        CVF_LOG_WARNING(l, "warn1");
        LogEvent le = dest->getLastAndClear();
        EXPECT_EQ(Logger::LL_WARNING, le.level());
        EXPECT_STREQ(L"warn1", le.message().c_str());
    }
    {
        CVF_LOG_INFO(l, "info1");
        LogEvent le = dest->getLastAndClear();
        EXPECT_EQ(Logger::LL_INFO, le.level());
        EXPECT_STREQ(L"info1", le.message().c_str());
    }
    {
        CVF_LOG_DEBUG(l, "dbg1");
        LogEvent le = dest->getLastAndClear();
        EXPECT_EQ(Logger::LL_DEBUG, le.level());
        EXPECT_STREQ(L"dbg1", le.message().c_str());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(LoggerTest, CreatingNamedLoggers)
{
    ref<LogManager> myLogManager = new LogManager;
    ref<Logger> root = myLogManager->rootLogger();
    ASSERT_STREQ(L"", root->name().c_str());

    const int rootLevel = root->level();
    ref<LogDestination> rootDestination = root->destination();

    ref<Logger> l_moduleA_classA = myLogManager->logger("moduleA.classA");
    EXPECT_STREQ(L"moduleA.classA", l_moduleA_classA->name().c_str());
    EXPECT_EQ(rootLevel, l_moduleA_classA->level());
    EXPECT_EQ(rootDestination, l_moduleA_classA->destination());
    {
        // Get it again and make sure we get same pointer
        ref<Logger> l_moduleA_classA_2 = myLogManager->logger("moduleA.classA");
        EXPECT_EQ(l_moduleA_classA, l_moduleA_classA_2);
    }

    ref<Logger> l_moduleA_classB = myLogManager->logger("moduleA.classB");
    EXPECT_STREQ(L"moduleA.classB", l_moduleA_classB->name().c_str());
    EXPECT_EQ(rootLevel, l_moduleA_classB->level());
    EXPECT_EQ(rootDestination, l_moduleA_classB->destination());

    ref<Logger> l_moduleA = myLogManager->logger("moduleA");
    EXPECT_STREQ(L"moduleA", l_moduleA->name().c_str());
    EXPECT_EQ(rootLevel, l_moduleA->level());
    EXPECT_EQ(rootDestination, l_moduleA->destination());

    // Now set another destination and level for ModuleA and create a new child logger
    const int moduleA_level = Logger::LL_DEBUG;
    ref<LogDestinationConsole> moduleA_destination = new LogDestinationConsole;
    l_moduleA->setLevel(moduleA_level);
    l_moduleA->setDestination(moduleA_destination.p());

    ref<Logger> l_moduleA_classC = myLogManager->logger("moduleA.classC");
    EXPECT_STREQ(L"moduleA.classC", l_moduleA_classC->name().c_str());
    EXPECT_EQ(moduleA_level, l_moduleA_classC->level());
    EXPECT_EQ(moduleA_destination, l_moduleA_classC->destination());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(LoggerTest, SettingLevelRecursive)
{
    ref<LogManager> myLogManager = new LogManager;

    ref<Logger> mA       = myLogManager->logger("moduleA");
    ref<Logger> mA_cX    = myLogManager->logger("moduleA.classX");
    ref<Logger> mA_cX_f1 = myLogManager->logger("moduleA.classX.func1");
    ref<Logger> mA_cY    = myLogManager->logger("moduleA.classY");
    ref<Logger> mB_cX    = myLogManager->logger("moduleB.classX");
    ref<Logger> mB_cY_f2 = myLogManager->logger("moduleB.classY.func2");
    EXPECT_EQ(Logger::LL_WARNING, mA       ->level());
    EXPECT_EQ(Logger::LL_WARNING, mA_cX    ->level());
    EXPECT_EQ(Logger::LL_WARNING, mA_cX_f1 ->level());
    EXPECT_EQ(Logger::LL_WARNING, mA_cY    ->level());
    EXPECT_EQ(Logger::LL_WARNING, mB_cX    ->level());
    EXPECT_EQ(Logger::LL_WARNING, mB_cY_f2 ->level());

    myLogManager->setLevelRecursive("", Logger::LL_DEBUG);
    EXPECT_EQ(Logger::LL_DEBUG, mA       ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cX    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cX_f1 ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cY    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cX    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cY_f2 ->level());

    myLogManager->setLevelRecursive("", Logger::LL_DEBUG);
    myLogManager->setLevelRecursive("moduleA", Logger::LL_ERROR);
    EXPECT_EQ(Logger::LL_ERROR, mA       ->level());
    EXPECT_EQ(Logger::LL_ERROR, mA_cX    ->level());
    EXPECT_EQ(Logger::LL_ERROR, mA_cX_f1 ->level());
    EXPECT_EQ(Logger::LL_ERROR, mA_cY    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cX    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cY_f2 ->level());

    myLogManager->setLevelRecursive("", Logger::LL_DEBUG);
    myLogManager->setLevelRecursive("moduleA.classX", Logger::LL_ERROR);
    EXPECT_EQ(Logger::LL_DEBUG, mA       ->level());
    EXPECT_EQ(Logger::LL_ERROR, mA_cX    ->level());
    EXPECT_EQ(Logger::LL_ERROR, mA_cX_f1 ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cY    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cX    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cY_f2 ->level());

    myLogManager->setLevelRecursive("", Logger::LL_DEBUG);
    myLogManager->setLevelRecursive("moduleB.classY.func2", Logger::LL_ERROR);
    EXPECT_EQ(Logger::LL_DEBUG, mA       ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cX    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cX_f1 ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cY    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cX    ->level());
    EXPECT_EQ(Logger::LL_ERROR, mB_cY_f2 ->level());


    // The following should give no change
    myLogManager->setLevelRecursive("", Logger::LL_DEBUG);
    myLogManager->setLevelRecursive("module", Logger::LL_ERROR);
    EXPECT_EQ(Logger::LL_DEBUG, mA       ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cX    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cX_f1 ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cY    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cX    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cY_f2 ->level());

    myLogManager->setLevelRecursive("classX", Logger::LL_ERROR);
    EXPECT_EQ(Logger::LL_DEBUG, mA       ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cX    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cX_f1 ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cY    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cX    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cY_f2 ->level());

    myLogManager->setLevelRecursive("func1", Logger::LL_ERROR);
    EXPECT_EQ(Logger::LL_DEBUG, mA       ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cX    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cX_f1 ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mA_cY    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cX    ->level());
    EXPECT_EQ(Logger::LL_DEBUG, mB_cY_f2 ->level());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(LoggerTest, SettingDestinationRecursive)
{
    ref<LogManager> myLogManager = new LogManager;

    ref<LogDestination> root = myLogManager->rootLogger()->destination();
    ref<LogDestinationConsole> dest0 = new LogDestinationConsole;
    ref<LogDestinationConsole> dest1 = new LogDestinationConsole;

    ref<Logger> mA       = myLogManager->logger("moduleA");
    ref<Logger> mA_cX    = myLogManager->logger("moduleA.classX");
    ref<Logger> mA_cX_f1 = myLogManager->logger("moduleA.classX.func1");
    ref<Logger> mA_cY    = myLogManager->logger("moduleA.classY");
    ref<Logger> mB_cX    = myLogManager->logger("moduleB.classX");
    ref<Logger> mB_cY_f2 = myLogManager->logger("moduleB.classY.func2");
    EXPECT_EQ(root, mA       ->destination());
    EXPECT_EQ(root, mA_cX    ->destination());
    EXPECT_EQ(root, mA_cX_f1 ->destination());
    EXPECT_EQ(root, mA_cY    ->destination());
    EXPECT_EQ(root, mB_cX    ->destination());
    EXPECT_EQ(root, mB_cY_f2 ->destination());

    myLogManager->setDestinationRecursive("", dest0.p());
    EXPECT_EQ(dest0, mA       ->destination());
    EXPECT_EQ(dest0, mA_cX    ->destination());
    EXPECT_EQ(dest0, mA_cX_f1 ->destination());
    EXPECT_EQ(dest0, mA_cY    ->destination());
    EXPECT_EQ(dest0, mB_cX    ->destination());
    EXPECT_EQ(dest0, mB_cY_f2 ->destination());

    myLogManager->setDestinationRecursive("", dest0.p());
    myLogManager->setDestinationRecursive("moduleA", dest1.p());
    EXPECT_EQ(dest1, mA       ->destination());
    EXPECT_EQ(dest1, mA_cX    ->destination());
    EXPECT_EQ(dest1, mA_cX_f1 ->destination());
    EXPECT_EQ(dest1, mA_cY    ->destination());
    EXPECT_EQ(dest0, mB_cX    ->destination());
    EXPECT_EQ(dest0, mB_cY_f2 ->destination());

    myLogManager->setDestinationRecursive("", dest0.p());
    myLogManager->setDestinationRecursive("moduleA.classX", dest1.p());
    EXPECT_EQ(dest0, mA       ->destination());
    EXPECT_EQ(dest1, mA_cX    ->destination());
    EXPECT_EQ(dest1, mA_cX_f1 ->destination());
    EXPECT_EQ(dest0, mA_cY    ->destination());
    EXPECT_EQ(dest0, mB_cX    ->destination());
    EXPECT_EQ(dest0, mB_cY_f2 ->destination());

    myLogManager->setDestinationRecursive("", dest0.p());
    myLogManager->setDestinationRecursive("moduleB.classY.func2", dest1.p());
    EXPECT_EQ(dest0, mA       ->destination());
    EXPECT_EQ(dest0, mA_cX    ->destination());
    EXPECT_EQ(dest0, mA_cX_f1 ->destination());
    EXPECT_EQ(dest0, mA_cY    ->destination());
    EXPECT_EQ(dest0, mB_cX    ->destination());
    EXPECT_EQ(dest1, mB_cY_f2 ->destination());


    // The following should give no change
    myLogManager->setDestinationRecursive("", dest0.p());
    myLogManager->setDestinationRecursive("module", dest1.p());
    EXPECT_EQ(dest0, mA       ->destination());
    EXPECT_EQ(dest0, mA_cX    ->destination());
    EXPECT_EQ(dest0, mA_cX_f1 ->destination());
    EXPECT_EQ(dest0, mA_cY    ->destination());
    EXPECT_EQ(dest0, mB_cX    ->destination());
    EXPECT_EQ(dest0, mB_cY_f2 ->destination());

    myLogManager->setDestinationRecursive("classX", dest1.p());
    EXPECT_EQ(dest0, mA       ->destination());
    EXPECT_EQ(dest0, mA_cX    ->destination());
    EXPECT_EQ(dest0, mA_cX_f1 ->destination());
    EXPECT_EQ(dest0, mA_cY    ->destination());
    EXPECT_EQ(dest0, mB_cX    ->destination());
    EXPECT_EQ(dest0, mB_cY_f2 ->destination());

    myLogManager->setDestinationRecursive("func1", dest1.p());
    EXPECT_EQ(dest0, mA       ->destination());
    EXPECT_EQ(dest0, mA_cX    ->destination());
    EXPECT_EQ(dest0, mA_cX_f1 ->destination());
    EXPECT_EQ(dest0, mA_cY    ->destination());
    EXPECT_EQ(dest0, mB_cX    ->destination());
    EXPECT_EQ(dest0, mB_cY_f2 ->destination());
}

