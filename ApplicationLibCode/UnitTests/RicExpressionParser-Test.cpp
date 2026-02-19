#include "gtest/gtest.h"

#include "RimSummaryCalculation.h"

#include "ExpressionParserImpl.h"
#include "expressionparser/ExpressionParser.h"

#include <numeric>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, BasicUsage )
{
    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> b( 10 );
    std::iota( b.begin(), b.end(), 100 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "b", b );
    parser.assignVector( "c", c );

    QString expr = "c := a + b";
    EXPECT_TRUE( parser.evaluate( expr ) );

    EXPECT_DOUBLE_EQ( c[0], 110.0 );
    EXPECT_DOUBLE_EQ( c[9], 128.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, DetectVariables )
{
    QString expr = "c := a + (x / y)";

    std::vector<QString> variables = ExpressionParser::detectReferencedVariables( expr );

    EXPECT_STREQ( variables[0].toStdString().data(), "c" );
    EXPECT_STREQ( variables[1].toStdString().data(), "a" );
    EXPECT_STREQ( variables[2].toStdString().data(), "x" );
    EXPECT_STREQ( variables[3].toStdString().data(), "y" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, FindLeftHandSide )
{
    {
        QString expr = "c := a";

        QString s = RimSummaryCalculation::findLeftHandSide( expr );

        EXPECT_STREQ( s.toStdString().data(), "c" );
    }

    {
        QString expr = "c:=a";

        QString s = RimSummaryCalculation::findLeftHandSide( expr );

        EXPECT_STREQ( s.toStdString().data(), "c" );
    }

    {
        QString expr = "\na:=b\n\nc:=a";

        QString s = RimSummaryCalculation::findLeftHandSide( expr );

        EXPECT_STREQ( s.toStdString().data(), "c" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, ForLoopWithIfStatement )
{
    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> b( 10 );
    std::iota( b.begin(), b.end(), 100 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "b", b );
    parser.assignVector( "c", c );

    QString expr = "for (var i := 0; i < min(a[],b[],c[]); i += 1)\n"
                   "{                                             \n"
                   "    c[i] := if((a[i] > 13), a[i], b[i]);      \n"
                   "}                                             \n";

    EXPECT_TRUE( parser.evaluate( expr ) );

    EXPECT_DOUBLE_EQ( c[0], 100.0 );
    EXPECT_DOUBLE_EQ( c[9], 19.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, ExpandedIfStatement )
{
    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> b( 10 );
    std::iota( b.begin(), b.end(), 100 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "b", b );
    parser.assignVector( "c", c );

    QString expr         = "c := if((a > 13), a, b)";
    auto    expandedText = ExpressionParserImpl::expandIfStatements( expr );

    // std::cout << expandedText.toStdString();

    EXPECT_TRUE( parser.evaluate( expandedText ) );

    EXPECT_DOUBLE_EQ( c[0], 100.0 );
    EXPECT_DOUBLE_EQ( c[9], 19.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, ExpandIfStatementsAndEvaluate )
{
    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> b( 10 );
    std::iota( b.begin(), b.end(), 100 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "b", b );
    parser.assignVector( "c", c );

    QString expr = "c := if((a > 13), a, b)";

    EXPECT_TRUE( parser.expandIfStatementsAndEvaluate( expr ) );

    EXPECT_DOUBLE_EQ( c[0], 100.0 );
    EXPECT_DOUBLE_EQ( c[9], 19.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, ExpandIfWithMinAggregation )
{
    // Test that min(b) in an if-condition is treated as the global minimum of vector b,
    // not as a per-element operation. This was a bug where min(b) was incorrectly expanded
    // to min(b[i]) which just returns b[i] (the identity for a single scalar argument).

    // a = [5, 6, 25], b = [4, 7, 6]
    // min(b) = 4
    // Expected: c[i] = (a[i] > 4) ? a[i] : b[i]
    //   c[0] = (5 > 4) ? 5 : 4  = 5
    //   c[1] = (6 > 4) ? 6 : 7  = 6   <- key: should compare 6 with min(b)=4, not b[1]=7
    //   c[2] = (25 > 4) ? 25 : 6 = 25

    std::vector<double> a = { 5.0, 6.0, 25.0 };
    std::vector<double> b = { 4.0, 7.0, 6.0 };
    std::vector<double> c( 3 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "b", b );
    parser.assignVector( "c", c );

    QString expr = "c := if(a > min(b), a, b)";

    EXPECT_TRUE( parser.expandIfStatementsAndEvaluate( expr ) );

    EXPECT_DOUBLE_EQ( c[0], 5.0 );
    EXPECT_DOUBLE_EQ( c[1], 6.0 ); // Would be 7.0 with the bug (comparing a[1]=6 with b[1]=7)
    EXPECT_DOUBLE_EQ( c[2], 25.0 );
}

//--------------------------------------------------------------------------------------------------
///
// The expression "c := min(if(a > 13, a, b))" computes if(a > 13, a, b) element-wise and assigns
// the minimum of that result as a constant to all elements of c.
//
// With a = [10..19] and b = [100..109]:
//   if(a > 13, a, b) => [100, 101, 102, 103, 14, 15, 16, 17, 18, 19]
//   min(...)         => 14
//
// expandIfStatements detects the agg_func(if(...)) pattern and generates a two-pass expansion:
// a reducing loop that finds the aggregate, followed by an assignment loop over all c elements.
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, MinOfIfStatementProducesConstant )
{
    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> b( 10 );
    std::iota( b.begin(), b.end(), 100 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "b", b );
    parser.assignVector( "c", c );

    QString expr = "c := min(if(a > 13, a, b))";

    EXPECT_TRUE( parser.expandIfStatementsAndEvaluate( expr ) );

    // All elements of c should be the constant minimum value 14
    for ( size_t i = 0; i < c.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( c[i], 14.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, MaxOfIfStatementProducesConstant )
{
    // a = [10..19], b = [100..109]
    // if(a > 13, a, b) => [100, 101, 102, 103, 14, 15, 16, 17, 18, 19]
    // max(...)         => 103

    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> b( 10 );
    std::iota( b.begin(), b.end(), 100 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "b", b );
    parser.assignVector( "c", c );

    EXPECT_TRUE( parser.expandIfStatementsAndEvaluate( "c := max(if(a > 13, a, b))" ) );

    for ( size_t i = 0; i < c.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( c[i], 103.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, SumOfIfStatementProducesConstant )
{
    // a = [10..19], b = [100..109]
    // if(a > 13, a, b) => [100, 101, 102, 103, 14, 15, 16, 17, 18, 19]
    // sum(...)         => 505

    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> b( 10 );
    std::iota( b.begin(), b.end(), 100 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "b", b );
    parser.assignVector( "c", c );

    EXPECT_TRUE( parser.expandIfStatementsAndEvaluate( "c := sum(if(a > 13, a, b))" ) );

    for ( size_t i = 0; i < c.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( c[i], 505.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, AvgOfIfStatementProducesConstant )
{
    // a = [10..19], b = [100..109]
    // if(a > 13, a, b) => [100, 101, 102, 103, 14, 15, 16, 17, 18, 19]
    // avg(...)         => 505 / 10 = 50.5

    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> b( 10 );
    std::iota( b.begin(), b.end(), 100 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "b", b );
    parser.assignVector( "c", c );

    EXPECT_TRUE( parser.expandIfStatementsAndEvaluate( "c := avg(if(a > 13, a, b))" ) );

    for ( size_t i = 0; i < c.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( c[i], 50.5 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, MinOfIfStatementAllConditionsTrue )
{
    // When all conditions are true the if always picks a.
    // a = [10..19], b = [100..109], condition a > 5 is always true
    // if(a > 5, a, b) => [10, 11, 12, 13, 14, 15, 16, 17, 18, 19]
    // min(...)        => 10

    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> b( 10 );
    std::iota( b.begin(), b.end(), 100 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "b", b );
    parser.assignVector( "c", c );

    EXPECT_TRUE( parser.expandIfStatementsAndEvaluate( "c := min(if(a > 5, a, b))" ) );

    for ( size_t i = 0; i < c.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( c[i], 10.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, MinOfIfStatementAllConditionsFalse )
{
    // When all conditions are false the if always picks b.
    // a = [10..19], b = [100..109], condition a > 100 is always false
    // if(a > 100, a, b) => [100, 101, 102, 103, 104, 105, 106, 107, 108, 109]
    // min(...)          => 100

    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> b( 10 );
    std::iota( b.begin(), b.end(), 100 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "b", b );
    parser.assignVector( "c", c );

    EXPECT_TRUE( parser.expandIfStatementsAndEvaluate( "c := min(if(a > 100, a, b))" ) );

    for ( size_t i = 0; i < c.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( c[i], 100.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
// Simple vector aggregation: c := min(a), max(a), sum(a), avg(a) without any if-statement.
// These expressions contain no "if" so they are passed directly to the exprtk evaluator.
//
// a = [10..19]
//   min(a) => 10
//   max(a) => 19
//   sum(a) => 145
//   avg(a) => 14.5
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, SimpleVectorMin )
{
    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "c", c );

    EXPECT_TRUE( parser.evaluate( "c := min(a)" ) );

    for ( size_t i = 0; i < c.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( c[i], 10.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, SimpleVectorMax )
{
    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "c", c );

    EXPECT_TRUE( parser.evaluate( "c := max(a)" ) );

    for ( size_t i = 0; i < c.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( c[i], 19.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, SimpleVectorSum )
{
    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "c", c );

    EXPECT_TRUE( parser.evaluate( "c := sum(a)" ) );

    for ( size_t i = 0; i < c.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( c[i], 145.0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, SimpleVectorAvg )
{
    // a = [10..19], avg = (10+11+...+19) / 10 = 145 / 10 = 14.5

    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> c( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "c", c );

    EXPECT_TRUE( parser.evaluate( "c := avg(a)" ) );

    for ( size_t i = 0; i < c.size(); i++ )
    {
        EXPECT_DOUBLE_EQ( c[i], 14.5 );
    }
}
