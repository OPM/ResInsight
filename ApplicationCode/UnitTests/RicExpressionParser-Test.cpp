#include "gtest/gtest.h"


#include "expressionparser/ExpressionParser.h"

#include <numeric>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RicExpressionParserTest, BasicUsage)
{
    std::vector<double> a(10);
    std::iota(a.begin(), a.end(), 10);

    std::vector<double> b(10);
    std::iota(b.begin(), b.end(), 100);

    std::vector<double> c(10);

    ExpressionParser parser;
    parser.assignVector("a", a);
    parser.assignVector("b", b);
    parser.assignVector("c", c);

    QString expr = "c := a + b";
    EXPECT_TRUE(parser.evaluate(expr));

    EXPECT_DOUBLE_EQ(c[0], 110.0);
    EXPECT_DOUBLE_EQ(c[9], 128.0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RicExpressionParserTest, DetectVariables)
{
    QString expr = "c := a + (x / y)";

    std::vector<QString> variables = ExpressionParser::detectReferencedVariables(expr);

    EXPECT_STREQ(variables[0].toStdString().data(), "a");
    EXPECT_STREQ(variables[1].toStdString().data(), "c");
    EXPECT_STREQ(variables[2].toStdString().data(), "x");
    EXPECT_STREQ(variables[3].toStdString().data(), "y");
}

