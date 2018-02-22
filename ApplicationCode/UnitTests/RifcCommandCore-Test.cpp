#include "gtest/gtest.h"

#include "RicfCommandFileExecutor.h"
#include "RicfCommandObject.h"
#include "RicfMessages.h"
#include "RifcCommandFileReader.h"

#include "cafPdmField.h"

class TestCommand1: public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;
public: 
    TestCommand1() 
    {
        RICF_InitField(&m_textArgument,   "TextArgument",   QString(), "TextArgument",   "", "", "");
        RICF_InitField(&m_doubleArgument, "DoubleArgument",       0.0, "DoubleArgument", "", "", "");
        RICF_InitField(&m_intArgument,    "IntArgument",            0, "IntArgument",    "", "", "");
    }

    virtual void execute() override { std::cout << "TestCommand1::execute(" << "\"" << m_textArgument().toStdString() << "\", " 
                                                                                    << m_doubleArgument() << ", " 
                                                                                    << m_intArgument << ");" << std::endl; }

    caf::PdmField<QString> m_textArgument;
    caf::PdmField<double>  m_doubleArgument;
    caf::PdmField<int>     m_intArgument;
};

CAF_PDM_SOURCE_INIT(TestCommand1, "TestCommand1");


class TC2: public RicfCommandObject
{
    CAF_PDM_HEADER_INIT;
public: 
    TC2() 
    {
        RICF_InitField(&m_textArgument,   "ta",   QString(), "TextArgument",   "", "", "");
        RICF_InitField(&m_doubleArgument, "da",         0.0, "DoubleArgument", "", "", "");
        RICF_InitField(&m_intArgument,    "ia",            0,"IntArgument",    "", "", "");
    }

    virtual void execute() override { std::cout << "TC2::execute(" << "\"" << m_textArgument().toStdString() << "\", " 
        << m_doubleArgument() << ", " 
        << m_intArgument << ");" << std::endl; }

    caf::PdmField<QString> m_textArgument;
    caf::PdmField<double>  m_doubleArgument;
    caf::PdmField<int>     m_intArgument;
};

CAF_PDM_SOURCE_INIT(TC2, "TC2");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RicfCommands, Test1)
{
    QString commandString("TestCommand1(IntArgument=3, TextArgument=\"Dette er en tekst, \\\"og\\\" jeg er: (happy)\", DoubleArgument=5.0e3) \n"
                          "TestCommand1 (  IntArgument = 4 , \n  TextArgument =  \"Dette er en tekst, \\\"og\\\" jeg er: (happy)\", \n  DoubleArgument =  5.0e-3  ) \n"
                          "  TestCommand1(TextArgument=\"Litt kortere tekst.\") \n"
                          "TC2 ( ta = \"Hepp\", ia = 3, da= 0.123)");

    //std::cout << commandString.toStdString() << std::endl;

    QTextStream inputStream(&commandString);
    RicfMessages errors;

    auto objects = RicfCommandFileReader::readCommands(inputStream, caf::PdmDefaultObjectFactory::instance(), &errors);
    EXPECT_EQ((size_t)4, objects.size());

    auto tc2 = dynamic_cast<TestCommand1*>(objects[0]);
    EXPECT_EQ(39, tc2->m_textArgument().size());
    EXPECT_EQ(5.0e3, tc2->m_doubleArgument());
    
    tc2 = dynamic_cast<TestCommand1*>(objects[1]);
    EXPECT_EQ(39, tc2->m_textArgument().size());
    EXPECT_EQ(5e-3, tc2->m_doubleArgument());
    
    tc2 = dynamic_cast<TestCommand1*>(objects[2]);
    EXPECT_EQ(19, tc2->m_textArgument().size());
    EXPECT_EQ(0.0, tc2->m_doubleArgument());

    auto tc3 = dynamic_cast<TC2*>(objects[3]);
    EXPECT_EQ(4, tc3->m_textArgument().size());
    EXPECT_EQ(0.123, tc3->m_doubleArgument());
    EXPECT_EQ(3, tc3->m_intArgument());

    for (auto obj: objects)
    {
        obj->execute();
    }

    for (auto obj: objects)
    {
        delete(obj);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RicfCommands, ErrorMessages)
{
    QString commandString("TesCommand1(IntArgument=3, TextArgument=\"Dette er en tekst, \\\"og\\\" jeg er: (happy)\", DoubleArgument=5.0e3) \n"
                          "TestCommand1 (  IntArgument = , \n  TextA rgument =  \"Dette er en tekst, \\\"og\\\" jeg er: (happy)\", \n  DoubleArgument  ) \n"
                          "  TestCommand1(TextArgument=Litt kortere tekst.\") \n"
                          "TC3 ( ta = \"Hepp\", ia = 3, da= 0.123)");

    std::cout << commandString.toStdString() << std::endl;

    QTextStream inputStream(&commandString);
    RicfMessages errors;

    auto objects = RicfCommandFileReader::readCommands(inputStream, caf::PdmDefaultObjectFactory::instance(), &errors);

    EXPECT_EQ((size_t)2, objects.size());
    EXPECT_EQ((size_t)5, errors.m_messages.size());
    // Errors should be:
    // Line 1 : TesCommand1 does not exist
    // Line 2 : Unreadable value for argument IntArgument
    // Line 3 : Can't find = after argument named TextA (space withing argument name)
    // Line 4 : Can't find = after argument named DoubleArgument
    // Line 5 : Missing quotes around TextArgument value

    for (const auto& msg: errors.m_messages)
    {
        QString label;
        if (msg.first == RicfMessages::MESSAGE_ERROR)
        {
            label = "Error  : ";
        }
        else
        {
            label = "Warning: ";
        }
        std::cout << label.toStdString() << msg.second.toStdString() << std::endl;
    }

    for (auto obj: objects)
    {
        delete(obj);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RicfCommands, EmptyArgumentList)
{
    // Ensure no error messages when command with no arguments is read
    QString commandString("TestCommand1()");

    QTextStream inputStream(&commandString);
    RicfMessages errors;

    auto objects = RicfCommandFileReader::readCommands(inputStream, caf::PdmDefaultObjectFactory::instance(), &errors);

    EXPECT_EQ((size_t)1, objects.size());
    EXPECT_EQ((size_t)0, errors.m_messages.size());

    for (auto obj : objects)
    {
        delete(obj);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RicfCommands, TransformFileCommandObjectsToExecutableCommandObjects)
{
    QString commandString = R"(
    replaceCase(newGridFile="/1.EGRID", caseId=1)
    replaceCase(newGridFile="/2.EGRID", caseId=2)

    openProject(path="/home/user/ResInsightProject.rsp")
    replaceCase(newGridFile="/3.EGRID", caseId=3)
    replaceCase(newGridFile="/4.EGRID", caseId=4)
  
    exportSnapshots()
    replaceCase(newGridFile="/6.EGRID", caseId=6)
    replaceCase(newGridFile="/7.EGRID", caseId=7)

    closeProject()

    )";

    QTextStream inputStream(&commandString);
    RicfMessages errors;

    auto objects = RicfCommandFileReader::readCommands(inputStream, caf::PdmDefaultObjectFactory::instance(), &errors);
    EXPECT_TRUE(errors.m_messages.empty());
    EXPECT_EQ((size_t)9, objects.size());

    auto exeObjects = RicfCommandFileExecutor::prepareFileCommandsForExecution(objects);
    EXPECT_EQ((size_t)6, exeObjects.size());

    for (auto obj : exeObjects)
    {
        delete(obj);
    }
}
