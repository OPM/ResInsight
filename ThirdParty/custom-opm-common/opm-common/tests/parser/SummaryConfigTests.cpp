/*
  Copyright 2013 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE SummaryConfigTests

#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>

#include <algorithm>

using namespace Opm;

static Deck createDeck_no_wells( const std::string& summary ) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "10 MAI 2007 / \n"
            "RUNSPEC\n"
            "\n"
            "DIMENS\n"
            " 10 10 10 /\n"
            "REGDIMS\n"
            "  3/\n"
            "GRID\n"
            "DXV \n 10*400 /\n"
            "DYV \n 10*400 /\n"
            "DZV \n 10*400 /\n"
            "TOPS \n 100*2202 / \n"
            "PERMX\n"
            "  1000*0.25 /\n"
            "COPY\n"
            "  PERMX PERMY /\n"
            "  PERMX PERMZ /\n"
            "/\n"
            "PORO \n"
            "   1000*0.15 /\n"
            "REGIONS\n"
            "FIPNUM\n"
            "200*1 300*2 500*3 /\n"
            "SCHEDULE\n"
            "SUMMARY\n"
            + summary;

    return parser.parseString(input);
}


static Deck createDeck( const std::string& summary ) {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "10 MAI 2007 / \n"
            "RUNSPEC\n"
            "\n"
            "DIMENS\n"
            " 10 10 10 /\n"
            "REGDIMS\n"
            "  3/\n"
            "GRID\n"
            "DXV \n 10*400 /\n"
            "DYV \n 10*400 /\n"
            "DZV \n 10*400 /\n"
            "TOPS \n 100*2202 / \n"
            "PERMX\n"
            "  1000*0.25 /\n"
            "COPY\n"
            "  PERMX PERMY /\n"
            "  PERMX PERMZ /\n"
            "/\n"
            "PORO \n"
            "   1000*0.15 /\n"
            "REGIONS\n"
            "FIPNUM\n"
            "200*1 300*2 500*3 /\n"
            "SCHEDULE\n"
            "WELSPECS\n"
            "     \'W_1\'        \'OP\'   1   1  3.33       \'OIL\'  7* /   \n"
            "     \'WX2\'        \'OP\'   2   2  3.33       \'OIL\'  7* /   \n"
            "     \'W_3\'        \'OP\'   2   5  3.92       \'OIL\'  7* /   \n"
            "     'PRODUCER' 'G'   5  5 2000 'GAS'     /\n"
            "/\n"
            "COMPDAT\n"
            "'PRODUCER'   5  5  1  1 'OPEN' 1* -1  0.5  / \n"
            "'W_1'   3    7    2    2      'OPEN'  1*          *      0.311   4332.346  2*         'X'     22.123 / \n"
            "'W_1'   2    2    1    1      /\n"
            "'WX2'   2    2    1    1      /\n"
            "/\n"
            "SUMMARY\n"
            + summary;

    return parser.parseString(input);
}

static std::vector< std::string > sorted_names( const SummaryConfig& summary ) {
    std::vector< std::string > ret;
    for( const auto& x : summary ) {
        auto wgname = x.namedEntity();
        if(wgname.size())
            ret.push_back( wgname );
    }

    std::sort( ret.begin(), ret.end() );
    return ret;
}

static std::vector< std::string > sorted_keywords( const SummaryConfig& summary ) {
    std::vector< std::string > ret;
    for( const auto& x : summary )
        ret.push_back( x.keyword() );

    std::sort( ret.begin(), ret.end() );
    return ret;
}

static std::vector< std::string > sorted_key_names( const SummaryConfig& summary ) {
    std::vector< std::string > ret;
    for( const auto& x : summary ) {
        ret.push_back( x.uniqueNodeKey() );
    }

    std::sort( ret.begin(), ret.end() );
    return ret;
}

static SummaryConfig createSummary( std::string input , const ParseContext& parseContext = ParseContext()) {
    ErrorGuard errors;
    auto deck = createDeck( input );
    auto python = std::make_shared<Python>();
    EclipseState state( deck );
    Schedule schedule(deck, state, parseContext, errors, python);
    return SummaryConfig( deck, schedule, state.getTableManager( ), parseContext, errors );
}

BOOST_AUTO_TEST_CASE(wells_all) {
    const auto input = "WWCT\n/\n";
    const auto summary = createSummary( input );

    const auto wells = { "PRODUCER", "WX2", "W_1", "W_3" };
    const auto names = sorted_names( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            wells.begin(), wells.end(),
            names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE(EMPTY) {
    auto deck = createDeck_no_wells( "" );
    auto python = std::make_shared<Python>();
    EclipseState state( deck );
    Schedule schedule(deck, state, python);
    SummaryConfig conf(deck, schedule, state.getTableManager());
    BOOST_CHECK_EQUAL( conf.size(), 0 );
}

BOOST_AUTO_TEST_CASE(wells_missingI) {
    auto python = std::make_shared<Python>();
    ParseContext parseContext;
    ErrorGuard errors;
    const auto input = "WWCT\n/\n";
    auto deck = createDeck_no_wells( input );
    parseContext.update(ParseContext::SUMMARY_UNKNOWN_WELL, InputError::THROW_EXCEPTION);
    EclipseState state( deck );
    Schedule schedule(deck, state, parseContext, errors, python );
    BOOST_CHECK_NO_THROW( SummaryConfig( deck, schedule, state.getTableManager( ), parseContext, errors ));
}


BOOST_AUTO_TEST_CASE(wells_select) {
    const auto input = "WWCT\n'W_1' 'WX2' /\n";
    const auto summary = createSummary( input );
    const auto wells = { "WX2", "W_1" };
    const auto names = sorted_names( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            wells.begin(), wells.end(),
            names.begin(), names.end() );

    BOOST_CHECK_EQUAL( summary.size(), 2 );
}

BOOST_AUTO_TEST_CASE(groups_all) {
    const auto summary = createSummary( "GWPR \n /\n" );
    const auto groups = { "G", "OP" };
    const auto names = sorted_names( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS( groups.begin(), groups.end(),
                                   names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE(wells_pattern) {
    const auto input = "WWCT\n'W*' /\n";
    const auto summary = createSummary( input );
    const auto wells = { "WX2", "W_1", "W_3" };
    const auto names = sorted_names( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            wells.begin(), wells.end(),
            names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE(fields) {
    const auto input = "FOPT\n";
    const auto summary = createSummary( input );
    const auto keywords = { "FOPT" };
    const auto names = sorted_keywords( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE(field_oil_efficiency) {
    const auto input = "FOE\n";
    const auto summary = createSummary( input );

    BOOST_CHECK_EQUAL( true , summary.hasKeyword( "FOE"));
}

BOOST_AUTO_TEST_CASE(blocks) {
    const auto input = "BPR\n"
                       "3 3 6 /\n"
                       "4 3 6 /\n"
                       "/";
    const auto summary = createSummary( input );
    const auto keywords = { "BPR", "BPR" };
    const auto names = sorted_keywords( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE(regions) {
    const auto input = "ROIP\n"
                       "1 2 3 /\n"
                       "RWIP\n"
                       "/\n"
                       "RGIP\n"
                       "1 2 /\n";

    const auto summary = createSummary( input );
    const auto keywords = { "RGIP", "RGIP",
                    "ROIP", "ROIP", "ROIP",
                    "RWIP", "RWIP", "RWIP" };
    const auto names = sorted_keywords( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            names.begin(), names.end() );
}

BOOST_AUTO_TEST_CASE(region2region) {
  const auto input = "ROFT\n"
    "1 2/\n"
    "3 4/\n"
    "/\n"
    "RWIP\n"
    "/\n"
    "RGFT\n"
    "5 6/\n"
    "7 8/\n"
    "/\n";


  ParseContext parseContext;
  parseContext.update(ParseContext::SUMMARY_UNHANDLED_KEYWORD, InputError::IGNORE);

  const auto summary = createSummary( input, parseContext );
  const auto keywords = { "RWIP", "RWIP", "RWIP" };
  const auto names = sorted_keywords( summary );

  BOOST_CHECK_EQUAL_COLLECTIONS(keywords.begin(), keywords.end(),
                                names.begin(), names.end() );

  parseContext.update(ParseContext::SUMMARY_UNHANDLED_KEYWORD, InputError::THROW_EXCEPTION);
  BOOST_CHECK_THROW( createSummary(input, parseContext), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(completions) {
    const auto input = "CWIR\n" // all specified
                       "'PRODUCER'  /\n"
                       "'WX2' 1 1 1 /\n"
                       "'WX2' 2 2 1 /\n"
                       "/\n"
                       "CWIT\n" // block defaulted
                       "'W_1' /\n"
                       "/\n"
                       "CGIT\n" // well defaulted
                       "* 2 2 1 /\n"
                       "/\n"
                       "CGIR\n" // all defaulted
                       " '*' /\n"
                       "/\n"
                       "CPRL\n" // all defaulted
                       " '*' /\n"
                       "/\n";

    const auto summary = createSummary( input );
    const auto keywords = { "CGIR", "CGIR", "CGIR", "CGIR",
                            "CGIT", "CGIT",
                            "CPRL", "CPRL", "CPRL", "CPRL",
                            "CWIR", "CWIR",
                            "CWIT", "CWIT" };
    const auto names = sorted_keywords( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            names.begin(), names.end() );

}

BOOST_AUTO_TEST_CASE( merge ) {
    const auto input1 = "WWCT\n/\n";
    auto summary1 = createSummary( input1 );

    const auto keywords = { "FOPT", "WWCT", "WWCT", "WWCT", "WWCT" };
    const auto wells = { "PRODUCER", "WX2", "W_1", "W_3" };

    const auto input2 = "FOPT\n";
    const auto summary2 = createSummary( input2 );

    summary1.merge( summary2 );
    const auto kw_names = sorted_keywords( summary1 );
    const auto well_names = sorted_names( summary1 );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            kw_names.begin(), kw_names.end() );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            wells.begin(), wells.end(),
            well_names.begin(), well_names.end() );
}

BOOST_AUTO_TEST_CASE( merge_move ) {
    const auto input = "WWCT\n/\n";
    auto summary = createSummary( input );

    const auto keywords = { "FOPT", "WWCT", "WWCT", "WWCT", "WWCT" };
    const auto wells = { "PRODUCER", "WX2", "W_1", "W_3" };

    summary.merge( createSummary( "FOPT\n" ) );

    const auto kw_names = sorted_keywords( summary );
    const auto well_names = sorted_names( summary );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keywords.begin(), keywords.end(),
            kw_names.begin(), kw_names.end() );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            wells.begin(), wells.end(),
            well_names.begin(), well_names.end() );
}

static const auto ALL_keywords = {
        "FAQR",  "FAQRG", "FAQT", "FAQTG", "FGIP", "FGIPG", "FGIPL",
        "FGIR",  "FGIT",  "FGOR", "FGPR",  "FGPT", "FOIP",  "FOIPG",
        "FOIPL", "FOIR",  "FOIT", "FOPR",  "FOPT", "FPR",   "FVIR",
        "FVIT",  "FVPR",  "FVPT", "FWCT",  "FWGR", "FWIP",  "FWIR",
        "FWIT",  "FWPR",  "FWPT",
        "GGIR",  "GGIT",  "GGOR", "GGPR",  "GGPT", "GOIR",  "GOIT",
        "GOPR",  "GOPT",  "GVIR", "GVIT",  "GVPR", "GVPT",  "GWCT",
        "GWGR",  "GWIR",  "GWIT", "GWPR",  "GWPT",
        "WBHP",  "WGIR",  "WGIT", "WGOR",  "WGPR", "WGPT",  "WOIR",
        "WOIT",  "WOPR",  "WOPT", "WPI",   "WTHP", "WVIR",  "WVIT",
        "WVPR",  "WVPT",  "WWCT", "WWGR",  "WWIR", "WWIT",  "WWPR",
        "WWPT",
        // ALL will not expand to these keywords yet
        "AAQR",  "AAQRG", "AAQT", "AAQTG"
};


BOOST_AUTO_TEST_CASE(summary_ALL) {

    const auto input = "ALL\n";

    const auto summary = createSummary( input );
    const auto key_names = sorted_key_names( summary );

    std::vector<std::string> all;

    for( std::string keyword: ALL_keywords ) {
        if(keyword[0]=='F') {
            all.push_back(keyword);
        }
        else if (keyword[0]=='G') {
            auto kn = keyword + ":";
            all.push_back(kn + "G");
            all.push_back(kn + "OP");
        }
        else if (keyword[0]=='W') {
            auto kn = keyword + ":";
            all.push_back(kn + "W_1");
            all.push_back(kn + "WX2");
            all.push_back(kn + "W_3");
            all.push_back(kn + "PRODUCER");
        }
    }

    std::sort(all.begin(), all.end());

    BOOST_CHECK_EQUAL_COLLECTIONS(
        all.begin(), all.end(),
        key_names.begin(), key_names.end());

    BOOST_CHECK_EQUAL( true , summary.hasKeyword( "FOPT"));
    BOOST_CHECK_EQUAL( true , summary.hasKeyword( "GGIT"));
    BOOST_CHECK_EQUAL( true , summary.hasKeyword( "WWCT"));

    BOOST_CHECK_EQUAL( false, summary.hasKeyword( "WOPP"));
    BOOST_CHECK_EQUAL( false, summary.hasKeyword( "FOPP"));

    BOOST_CHECK_EQUAL( false , summary.hasKeyword("NO-NOT-THIS"));
}



BOOST_AUTO_TEST_CASE(INVALID_WELL1) {
    ParseContext parseContext;
    const auto input = "CWIR\n"
                       "NEW-WELL /\n"
        "/\n";
    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_WELL , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( createSummary( input , parseContext ) , std::invalid_argument);

    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_WELL , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( createSummary( input , parseContext ));
}


BOOST_AUTO_TEST_CASE(INVALID_WELL2) {
    ParseContext parseContext;
    const auto input = "WWCT\n"
        " NEW-WELL /\n";
    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_WELL , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( createSummary( input , parseContext ) , std::invalid_argument);

    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_WELL , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( createSummary( input , parseContext ));
}

BOOST_AUTO_TEST_CASE(UNDEFINED_UDQ_WELL) {
    ParseContext parseContext;
    const auto input = "WUWCT\n"
        "/\n";
    parseContext.updateKey( ParseContext::SUMMARY_UNDEFINED_UDQ, InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( createSummary( input , parseContext ) , std::invalid_argument);

    parseContext.updateKey( ParseContext::SUMMARY_UNDEFINED_UDQ, InputError::IGNORE );
    BOOST_CHECK_NO_THROW( createSummary( input , parseContext ));
}




BOOST_AUTO_TEST_CASE(INVALID_GROUP) {
    ParseContext parseContext;
    const auto input = "GWCT\n"
        " NEW-GR /\n";
    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_GROUP , InputError::THROW_EXCEPTION );
    BOOST_CHECK_THROW( createSummary( input , parseContext ) , std::invalid_argument);

    parseContext.updateKey( ParseContext::SUMMARY_UNKNOWN_GROUP , InputError::IGNORE );
    BOOST_CHECK_NO_THROW( createSummary( input , parseContext ));
}

BOOST_AUTO_TEST_CASE( REMOVE_DUPLICATED_ENTRIES ) {
    ParseContext parseContext;
    const auto input = "WGPR \n/\n"
                       "WGPR \n/\n"
                       "ALL\n";

    const auto summary = createSummary( input );
    const auto keys = sorted_key_names( summary );
    auto uniq_keys = keys;
    uniq_keys.erase( std::unique( uniq_keys.begin(),
                                  uniq_keys.end(),
                                  std::equal_to< std::string >() ),
                     uniq_keys.end() );

    BOOST_CHECK_EQUAL_COLLECTIONS(
            keys.begin(), keys.end(),
            uniq_keys.begin(), uniq_keys.end() );
}

BOOST_AUTO_TEST_CASE( ANALYTICAL_AQUIFERS ) {
    const std::string input = R"(
            AAQR
                1 2 /
            AAQP
                2 1 /
            AAQT
                /
            AAQRG
                /
            AAQTG
                /
            AAQTD
                /
            AAQPD
                /
    )";
    const auto summary = createSummary( input );
}

BOOST_AUTO_TEST_CASE( NUMERICAL_AQUIFERS ) {
    const std::string input = R"(
            ANQR
                1 2 /
            ANQP
                2 1 /
            ANQT
                /
    )";
    const auto summary = createSummary( input );
}

static const auto GMWSET_keywords = {
    "GMWPT", "GMWPR", "GMWPA", "GMWPU", "GMWPG", "GMWPO", "GMWPS",
    "GMWPV", "GMWPP", "GMWPL", "GMWIT", "GMWIN", "GMWIA", "GMWIU", "GMWIG",
    "GMWIS", "GMWIV", "GMWIP", "GMWDR", "GMWDT", "GMWWO", "GMWWT"
};

BOOST_AUTO_TEST_CASE( summary_GMWSET ) {

    const auto input = "GMWSET\n";
    const auto summary = createSummary( input );
    const auto key_names = sorted_key_names( summary );

    std::vector< std::string > all;

    for( std::string kw : GMWSET_keywords ) {
        all.emplace_back(kw + ":G");
        all.emplace_back(kw + ":OP");
    }

    std::sort( all.begin(), all.end() );

    BOOST_CHECK_EQUAL_COLLECTIONS( all.begin(), all.end(),
                                   key_names.begin(), key_names.end() );

    BOOST_CHECK( summary.hasKeyword( "GMWPS" ) );
    BOOST_CHECK( summary.hasKeyword( "GMWPT" ) );
    BOOST_CHECK( summary.hasKeyword( "GMWPR" ) );

    BOOST_CHECK( !summary.hasKeyword("NO-NOT-THIS") );
}

static const auto FMWSET_keywords = {
    "FMCTF", "FMWPT", "FMWPR", "FMWPA", "FMWPU", "FMWPF", "FMWPO", "FMWPS",
    "FMWPV", "FMWPP", "FMWPL", "FMWIT", "FMWIN", "FMWIA", "FMWIU", "FMWIF",
    "FMWIS", "FMWIV", "FMWIP", "FMWDR", "FMWDT", "FMWWO", "FMWWT"
};

BOOST_AUTO_TEST_CASE( summary_FMWSET ) {

    const auto input = "FMWSET\n";
    const auto summary = createSummary( input );
    const auto key_names = sorted_key_names( summary );

    std::vector< std::string > all( FMWSET_keywords.begin(),
                                    FMWSET_keywords.end() );
    std::sort( all.begin(), all.end() );

    BOOST_CHECK_EQUAL_COLLECTIONS( all.begin(), all.end(),
                                   key_names.begin(), key_names.end() );

    BOOST_CHECK( summary.hasKeyword( "FMWPS" ) );
    BOOST_CHECK( summary.hasKeyword( "FMWPT" ) );
    BOOST_CHECK( summary.hasKeyword( "FMWPR" ) );

    BOOST_CHECK( !summary.hasKeyword("NO-NOT-THIS") );
}



BOOST_AUTO_TEST_CASE( WOPRL ) {
    const std::string input = R"(
WOPRL
   'W_1'  1 /
   'WX2'  2 /
   'W_3'  3 /
/
)";

    ParseContext parseContext;
    parseContext.update(ParseContext::SUMMARY_UNHANDLED_KEYWORD, InputError::THROW_EXCEPTION);
    BOOST_CHECK_THROW(createSummary( input, parseContext ), std::invalid_argument);
    parseContext.update(ParseContext::SUMMARY_UNHANDLED_KEYWORD, InputError::IGNORE);
    BOOST_CHECK_NO_THROW( createSummary(input, parseContext ));
}


BOOST_AUTO_TEST_CASE( summary_require3DField ) {
    {
        const auto input = "WWCT\n/\n";
        const auto summary = createSummary( input );

        BOOST_CHECK( !summary.require3DField( "NO-NOT-THIS"));

        BOOST_CHECK( !summary.require3DField( "PRESSURE"));
        BOOST_CHECK( !summary.require3DField( "OIP"));
        BOOST_CHECK( !summary.require3DField( "GIP"));
        BOOST_CHECK( !summary.require3DField( "WIP"));
        BOOST_CHECK( !summary.require3DField( "OIPL"));
        BOOST_CHECK( !summary.require3DField( "OIPG"));
        BOOST_CHECK( !summary.require3DField( "GIPL"));
        BOOST_CHECK( !summary.require3DField( "GIPG"));
        BOOST_CHECK( !summary.require3DField( "SWAT"));
        BOOST_CHECK( !summary.require3DField( "SGAS"));
    }

    {
        const auto input = "BPR\n"
            "3 3 6 /\n"
            "4 3 6 /\n"
            "/";

        const auto summary = createSummary( input );
        BOOST_CHECK( summary.require3DField( "PRESSURE"));
        BOOST_CHECK( !summary.requireFIPNUM( ));
    }


    {
        const auto input = "FPR\n";
        const auto summary = createSummary( input );
        BOOST_CHECK( summary.require3DField( "PRESSURE"));
    }


    {
        const auto input = "BSWAT\n"
            "3 3 6 /\n"
            "4 3 6 /\n"
            "/";

        const auto summary = createSummary( input );
        BOOST_CHECK( summary.require3DField( "SWAT"));
    }

    {
        const auto input = "BSGAS\n"
            "3 3 6 /\n"  // 523
            "4 3 6 /\n"  // 524
            "/";

        const auto summary = createSummary( input );
        BOOST_CHECK( summary.require3DField( "SGAS"));
        BOOST_CHECK( summary.hasSummaryKey( "BSGAS:523" ) );
    }


    {
        const auto input = "RPR\n/\n";
        const auto summary = createSummary( input );
        BOOST_CHECK( summary.require3DField( "PRESSURE"));
        BOOST_CHECK( summary.requireFIPNUM( ));
        BOOST_CHECK( summary.hasKeyword( "RPR" ) );
        BOOST_CHECK( summary.hasSummaryKey( "RPR:1" ) );
        BOOST_CHECK( summary.hasSummaryKey( "RPR:3" ) );
        BOOST_CHECK( !summary.hasSummaryKey( "RPR:4" ) );
    }


    {
        const auto input = "RPR\n 10 /\n";
        BOOST_CHECK_THROW( createSummary( input ) , std::invalid_argument );
    }



    {
        const auto input = "RGIPL\n/\n";
        const auto summary = createSummary( input );
        BOOST_CHECK( summary.require3DField( "GIPL"));
        BOOST_CHECK( summary.requireFIPNUM( ));
    }
}


BOOST_AUTO_TEST_CASE( SUMMARY_MISC) {
    {
        const auto summary = createSummary( "TCPU\n" );
        BOOST_CHECK( summary.hasKeyword( "TCPU" ) );
    }

    {
        const auto summary = createSummary( "PERFORMA\n" );
        BOOST_CHECK( summary.hasKeyword( "ELAPSED" ) );
        BOOST_CHECK( !summary.hasKeyword("PERFORMA"));
    }
}

BOOST_AUTO_TEST_CASE(Summary_Segment)
{
    auto python = std::make_shared<Python>();
    const auto input = std::string { "SOFR_TEST.DATA" };
    const auto deck  = Parser{}.parseFile(input);
    const auto state = EclipseState { deck };

    const auto schedule = Schedule { deck, state, python};
    const auto summary  = SummaryConfig {
        deck, schedule, state.getTableManager()
    };

    // SOFR PROD01 segments 1, 10, 21.
    BOOST_CHECK(deck.hasKeyword("SOFR"));
    BOOST_CHECK(summary.hasKeyword("SOFR"));
    BOOST_CHECK(summary.hasSummaryKey("SOFR:PROD01:1"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:2"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:3"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:4"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:5"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:6"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:7"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:8"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SOFR:PROD01:10"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:11"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:12"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:13"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:14"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:15"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:16"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:17"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:18"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:19"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:20"));
    BOOST_CHECK(summary.hasSummaryKey("SOFR:PROD01:21"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:22"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:23"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:24"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:25"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:26"));
    BOOST_CHECK(!summary.hasSummaryKey("SOFR:PROD01:27"));

    BOOST_CHECK(!summary.hasSummaryKey("SOFR:INJE01:1"));

    {
        auto sofr = std::find_if(summary.begin(), summary.end(),
            [](const SummaryConfigNode& node)
        {
            return node.keyword() == "SOFR";
        });

        BOOST_REQUIRE(sofr != summary.end());

        BOOST_CHECK_MESSAGE(sofr->category() == SummaryConfigNode::Category::Segment,
            R"("SOFR" keyword category must be "Segment")"
        );
        BOOST_CHECK_EQUAL(sofr->namedEntity(), "PROD01");
    }

    BOOST_CHECK(deck.hasKeyword("SGFR"));
    BOOST_CHECK(summary.hasKeyword("SGFR"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:1"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:2"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:3"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:4"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:5"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:6"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:7"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:8"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:10"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:11"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:12"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:13"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:14"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:15"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:16"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:17"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:18"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:19"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:20"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:21"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:22"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:23"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:24"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:25"));
    BOOST_CHECK(summary.hasSummaryKey("SGFR:PROD01:26"));
    BOOST_CHECK(!summary.hasSummaryKey("SGFR:PROD01:27"));  // No such segment.

    // SPR PROD01 segment 10 only.
    BOOST_CHECK(deck.hasKeyword("SPR"));
    BOOST_CHECK(summary.hasKeyword("SPR"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:1"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:2"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:3"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:4"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:5"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:6"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:7"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:8"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SPR:PROD01:10"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:11"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:12"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:13"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:14"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:15"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:16"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:17"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:18"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:19"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:20"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:21"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:22"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:23"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:24"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:25"));
    BOOST_CHECK(!summary.hasSummaryKey("SPR:PROD01:26"));

    BOOST_CHECK(!summary.hasSummaryKey("SPR:INJE01:10"));

    // SWFR for all segments in all MS wells.
    BOOST_CHECK(deck.hasKeyword("SWFR"));
    BOOST_CHECK(summary.hasKeyword("SWFR"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:1"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:2"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:3"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:4"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:5"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:6"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:7"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:8"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:10"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:11"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:12"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:13"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:14"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:15"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:16"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:17"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:18"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:19"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:20"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:21"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:22"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:23"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:24"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:25"));
    BOOST_CHECK(summary.hasSummaryKey("SWFR:PROD01:26"));

    BOOST_CHECK(!summary.hasSummaryKey("SWFR:INJE01:1"));

    // SPRD for all segments in all MS wells.
    BOOST_CHECK(deck.hasKeyword("SPRD"));
    BOOST_CHECK(summary.hasKeyword("SPRD"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:1"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:2"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:3"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:4"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:5"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:6"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:7"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:8"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:10"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:11"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:12"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:13"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:14"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:15"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:16"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:17"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:18"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:19"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:20"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:21"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:22"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:23"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:24"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:25"));
    BOOST_CHECK(summary.hasSummaryKey("SPRD:PROD01:26"));

    BOOST_CHECK(!summary.hasSummaryKey("SPRD:INJE01:1"));

    // SPRDH for all segments of MS well PROD01.
    BOOST_CHECK(deck.hasKeyword("SPRDH"));
    BOOST_CHECK(summary.hasKeyword("SPRDH"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:1"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:2"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:3"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:4"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:5"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:6"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:7"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:8"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:10"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:11"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:12"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:13"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:14"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:15"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:16"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:17"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:18"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:19"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:20"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:21"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:22"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:23"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:24"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:25"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDH:PROD01:26"));

    BOOST_CHECK(!summary.hasSummaryKey("SPRDH:INJE01:1"));

    // SPRDF for segments 10 and 16 of MS well PROD01.
    BOOST_CHECK(deck.hasKeyword("SPRDF"));
    BOOST_CHECK(summary.hasKeyword("SPRDF"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:1"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:2"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:3"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:4"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:5"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:6"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:7"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:8"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDF:PROD01:10"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:11"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:12"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:13"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:14"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:15"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDF:PROD01:16"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:17"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:18"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:19"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:20"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:21"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:22"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:23"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:24"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:25"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:PROD01:26"));

    BOOST_CHECK(!summary.hasSummaryKey("SPRDF:INJE01:1"));

    // SPRDA for segments 10 and 16 of all MS wells
    BOOST_CHECK(deck.hasKeyword("SPRDA"));
    BOOST_CHECK(summary.hasKeyword("SPRDA"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:1"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:2"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:3"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:4"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:5"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:6"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:7"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:8"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:9"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDA:PROD01:10"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:11"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:12"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:13"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:14"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:15"));
    BOOST_CHECK(summary.hasSummaryKey("SPRDA:PROD01:16"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:17"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:18"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:19"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:20"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:21"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:22"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:23"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:24"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:25"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:PROD01:26"));

    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:INJE01:1"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:INJE01:10"));
    BOOST_CHECK(!summary.hasSummaryKey("SPRDA:INJE01:16"));
}

BOOST_AUTO_TEST_CASE(ProcessingInstructions) {
    const std::string deck_string = R"(
RPTONLY
RUNSUM
NARROW
SEPARATE
)";

    const auto& summary_config = createSummary(deck_string);

    BOOST_CHECK(!summary_config.hasKeyword("NARROW"));
    BOOST_CHECK(!summary_config.hasKeyword("RPTONLY"));
    BOOST_CHECK(!summary_config.hasKeyword("RUNSUM"));
    BOOST_CHECK(!summary_config.hasKeyword("SEPARATE"));
    BOOST_CHECK(!summary_config.hasKeyword("SUMMARY"));
}


BOOST_AUTO_TEST_CASE(EnableRSM) {
    std::string deck_string1 = "";
    std::string deck_string2 = R"(
RUNSUM
)";
    const auto& summary_config1 = createSummary(deck_string1);
    const auto& summary_config2 = createSummary(deck_string2);

    BOOST_CHECK(!summary_config1.createRunSummary());
    BOOST_CHECK(!summary_config1.hasKeyword("RUNSUM"));

    BOOST_CHECK( summary_config2.createRunSummary());
    BOOST_CHECK(!summary_config2.hasKeyword("RUNSUM"));
}
