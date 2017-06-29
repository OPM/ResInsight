#include "gtest/gtest.h"

#include "RifcCommandFileReader.h"
#include "RicfCommandObject.h"
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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RicfCommands, Test1)
{
    TestCommand1* tc = new TestCommand1;
    tc->m_textArgument = "textValue";
    tc->execute();
    delete tc;

    QString commandString("TestCommand1(IntArgument=3, TextArgument=\"Dette er en tekst, \\\"og\\\" jeg er: (happy)\", DoubleArgument=5.0e3) \n"
                          "TestCommand1 (  IntArgument = 4 , \n  TextArgument =  \"Dette er en tekst, \\\"og\\\" jeg er: (happy)\", \n  DoubleArgument =  5.0e-3  ) \n"
                          "  TestCommand1(TextArgument=\"Litt kortere tekst.\") \n");
    std::cout << commandString.toStdString() << std::endl;
    QTextStream inputStream(&commandString);

    auto objects = RicfCommandFileReader::readCommands(inputStream, caf::PdmDefaultObjectFactory::instance());
    EXPECT_EQ(3, objects.size());

    auto tc2 = dynamic_cast<TestCommand1*>(objects[0]);
    EXPECT_EQ(39, tc2->m_textArgument().size());
    EXPECT_EQ(5.0e3, tc2->m_doubleArgument());

    tc2 = dynamic_cast<TestCommand1*>(objects[1]);
    EXPECT_EQ(37, tc2->m_textArgument().size());
    EXPECT_EQ(5e-3, tc2->m_doubleArgument());

    tc2 = dynamic_cast<TestCommand1*>(objects[2]);
    EXPECT_EQ(19, tc2->m_textArgument().size());
    EXPECT_EQ(0.0, tc2->m_doubleArgument());

    for (auto obj: objects)
    {
        obj->execute();
    }

    for (auto obj: objects)
    {
        delete(obj);
    }
}