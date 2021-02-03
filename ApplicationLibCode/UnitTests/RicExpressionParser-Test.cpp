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

    EXPECT_TRUE( parser.expandStatementsAndEvaluate( expr ) );

    EXPECT_DOUBLE_EQ( c[0], 100.0 );
    EXPECT_DOUBLE_EQ( c[9], 19.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( RicExpressionParserTest, ExpandDeltaStatementsIncludingAssignment )
{
    std::vector<double> a( 10 );
    std::iota( a.begin(), a.end(), 10 );

    std::vector<double> b( 10 );
    std::iota( b.begin(), b.end(), 100 );

    std::vector<double> a_delta( 10 );
    std::vector<double> b_delta( 10 );

    ExpressionParser parser;
    parser.assignVector( "a", a );
    parser.assignVector( "b", b );
    parser.assignVector( "b_delta", b_delta );
    parser.assignVector( "a_delta", a_delta );

    QString expandedText;

    QString expressionText = "a_delta := delta(a)";

    {
        QStringList lines = expressionText.split( "\n" );

        for ( const auto& line : lines )
        {
            QString lineNoWhitespace = line;
            lineNoWhitespace         = lineNoWhitespace.replace( " ", "" );
            QStringList words        = lineNoWhitespace.split( ":=" );
            if ( words.size() == 2 )
            {
                auto deltaVariable = words[0].trimmed();

                auto sourceVariable = words[1].trimmed();
                sourceVariable.replace( "delta(", "" );
                sourceVariable.replace( ")", "" );

                QString deltaLoopText = QString( "%1[0] := 0\n" ).arg( deltaVariable );
                deltaLoopText += QString( "for (var i := 1; i < %1[]; i += 1)\n" ).arg( sourceVariable );
                deltaLoopText += "{\n";
                deltaLoopText += QString( "  %1[i] := %2[i] - %2[i-1];\n" ).arg( deltaVariable ).arg( sourceVariable );
                deltaLoopText += "}\n";

                expandedText += deltaLoopText;
            }
            else
            {
                expandedText += line;
                expandedText += "\n";
            }
        }
    }

    QString errorText;
    EXPECT_TRUE( parser.evaluate( expandedText, &errorText ) );

    EXPECT_DOUBLE_EQ( a_delta[0], 00.0 );
    EXPECT_DOUBLE_EQ( a_delta[1], 1.0 );
}
