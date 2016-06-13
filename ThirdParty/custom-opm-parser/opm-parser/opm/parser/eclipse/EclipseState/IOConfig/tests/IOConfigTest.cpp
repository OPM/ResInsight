/*
  Copyright 2015 Statoil ASA.

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



#define BOOST_TEST_MODULE IOConfigTests

#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>


using namespace Opm;

const std::string& deckStr =  "RUNSPEC\n"
                              "\n"
                              "DIMENS\n"
                              " 10 10 10 /\n"
                              "GRID\n"
                              "GRIDFILE\n"
                              " 0 1 /\n"
                              "\n"
                              "START\n"
                              " 21 MAY 1981 /\n"
                              "\n"
                              "SCHEDULE\n"
                              "DATES\n"
                              " 22 MAY 1981 /\n"              // timestep 1
                              " 23 MAY 1981 /\n"              // timestep 2
                              " 24 MAY 1981 /\n"              // timestep 3
                              " 25 MAY 1981 /\n"              // timestep 4
                              " 26 MAY 1981 /\n"              // timestep 5
                              " 1 JAN 1982 /\n"               // timestep 6
                              " 1 JAN 1982 13:55:44 /\n"      // timestep 7
                              " 3 JAN 1982 14:56:45.123 /\n"  // timestep 8
                              " 4 JAN 1982 14:56:45.123 /\n"  // timestep 9
                              " 5 JAN 1982 14:56:45.123 /\n"  // timestep 10
                              " 6 JAN 1982 14:56:45.123 /\n"  // timestep 11
                              " 7 JAN 1982 14:56:45.123 /\n"  // timestep 12
                              " 8 JAN 1982 14:56:45.123 /\n"  // timestep 13
                              " 9 JAN 1982 14:56:45.123 /\n"  // timestep 14
                              " 10 JAN 1982 14:56:45.123 /\n" // timestep 15
                              " 11 JAN 1982 14:56:45.123 /\n" // timestep 16
                              " 1 JAN 1983 /\n"               // timestep 17
                              " 2 JAN 1983 /\n"               // timestep 18
                              " 3 JAN 1983 /\n"               // timestep 19
                              " 1 JAN 1984 /\n"               // timestep 20
                              " 2 JAN 1984 /\n"               // timestep 21
                              " 1 JAN 1985 /\n"               // timestep 22
                              " 3 JAN 1986 14:56:45.123 /\n"  // timestep 23
                              " 4 JAN 1986 14:56:45.123 /\n"  // timestep 24
                              " 5 JAN 1986 14:56:45.123 /\n"  // timestep 25
                              " 1 JAN 1987 /\n"               // timestep 26
                              " 1 JAN 1988 /\n"               // timestep 27
                              " 2 JAN 1988 /\n"               // timestep 28
                              " 3 JAN 1988 /\n"               // timestep 29
                              " 1 JAN 1989 /\n"               // timestep 30
                              " 2 JAN 1989 /\n"               // timestep 31
                              " 2 JAN 1990 /\n"               // timestep 32
                              " 2 JAN 1991 /\n"               // timestep 33
                              " 3 JAN 1991 /\n"               // timestep 34
                              " 4 JAN 1991 /\n"               // timestep 35
                              " 1 JAN 1992 /\n"               // timestep 36
                              " 1 FEB 1992 /\n"               // timestep 37
                              " 1 MAR 1992 /\n"               // timestep 38
                              " 2 MAR 1992 /\n"               // timestep 39
                              " 3 MAR 1992 /\n"               // timestep 40
                              " 4 MAR 1992 /\n"               // timestep 41
                              " 1 APR 1992 /\n"               // timestep 42
                              " 2 APR 1992 /\n"               // timestep 43
                              " 1 MAY 1992 /\n"               // timestep 44
                              " 2 MAY 1992 /\n"               // timestep 45
                              " 3 MAY 1992 /\n"               // timestep 46
                              " 3 JUN 1992 /\n"               // timestep 47
                              " 3 JUL 1992 /\n"               // timestep 48
                              " 3 AUG 1992 /\n"               // timestep 49
                              " 4 AUG 1992 /\n"               // timestep 50
                              " 5 AUG 1992 /\n"               // timestep 51
                              " 6 AUG 1992 /\n"               // timestep 52
                              " 7 AUG 1992 /\n"               // timestep 53
                              " 8 AUG 1992 /\n"               // timestep 54
                              " 9 AUG 1992 /\n"               // timestep 55
                              " 10 AUG 1992 /\n"              // timestep 56
                              " 11 AUG 1992 /\n"              // timestep 57
                              " 12 AUG 1992 /\n"              // timestep 58
                              " 13 AUG 1992 /\n"              // timestep 59
                              " 14 AUG 1992 /\n"              // timestep 60
                              " 15 AUG 1992 /\n"              // timestep 61
                                                        "/\n"
                                                        "\n";


const std::string& deckStr_NOGGF = "RUNSPEC\n"
                                   "UNIFIN\n"
                                   "UNIFOUT\n"
                                   "FMTIN\n"
                                   "FMTOUT\n"
                                   "\n"
                                   "DIMENS\n"
                                   "10 10 10 /\n"
                                   "GRID\n"
                                   "INIT\n"
                                   "NOGGF\n"
                                   "\n";

const std::string& deckStr_NO_GRIDFILE = "RUNSPEC\n"
                                         "\n"
                                         "DIMENS\n"
                                        " 10 10 10 /\n"
                                         "GRID\n"
                                         "GRIDFILE\n"
                                         " 0 0 /\n"
                                         "\n";




const std::string deckStr_RFT = "RUNSPEC\n"
                                "OIL\n"
                                "GAS\n"
                                "WATER\n"
                                "DIMENS\n"
                                " 10 10 10 /\n"
                                "GRID\n"
                                "DXV\n"
                                "10*0.25 /\n"
                                "DYV\n"
                                "10*0.25 /\n"
                                "DZV\n"
                                "10*0.25 /\n"
                                "TOPS\n"
                                "100*0.25 /\n"
                                "\n"
                                 "START             -- 0 \n"
                                "1 NOV 1979 / \n"
                                "SCHEDULE\n"
                                "DATES             -- 1\n"
                                " 1 DES 1979/ \n"
                                "/\n"
                                "WELSPECS\n"
                                "    'OP_1'       'OP'   9   9 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                                "    'OP_2'       'OP'   4   4 1*     'OIL' 1*      1*  1*   1*  1*   1*  1*  / \n"
                                "/\n"
                                "COMPDAT\n"
                                " 'OP_1'  9  9   1   1 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                                " 'OP_1'  9  9   2   2 'OPEN' 1*   46.825   0.311  4332.346 1*  1*  'X'  22.123 / \n"
                                " 'OP_1'  9  9   3  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                                " 'OP_2'  4  4   4  9 'OPEN' 1*   32.948   0.311  3047.839 1*  1*  'X'  22.100 / \n"
                                "/\n"
                                "DATES             -- 2\n"
                                " 10  OKT 2008 / \n"
                                "/\n"
                                "WRFT \n"
                                "/ \n"
                                "WELOPEN\n"
                                " 'OP_1' OPEN / \n"
                                " 'OP_2' OPEN / \n"
                                "/\n"
                                "DATES             -- 3\n"
                                " 10  NOV 2008 / \n"
                                "/\n";



static DeckPtr createDeck(const std::string& input) {
    Opm::Parser parser;
    return parser.parseString(input, Opm::ParseContext());
}


BOOST_AUTO_TEST_CASE( RFT_TIME) {
    DeckPtr deck = createDeck(deckStr_RFT);
    EclipseState state( deck , Opm::ParseContext() );
    std::shared_ptr<const IOConfig> ioConfig = state.getIOConfigConst();


    BOOST_CHECK_EQUAL( ioConfig->getFirstRFTStep() , 2 );
}


BOOST_AUTO_TEST_CASE(IOConfigTest) {

    DeckPtr deck = createDeck(deckStr);
    std::shared_ptr<const EclipseGrid> grid = std::make_shared<const EclipseGrid>( 10 , 10 , 10 );
    IOConfigPtr ioConfigPtr;
    BOOST_CHECK_NO_THROW(ioConfigPtr = std::make_shared<IOConfig>());

    std::shared_ptr<const GRIDSection> gridSection = std::make_shared<const GRIDSection>(*deck);
    std::shared_ptr<const RUNSPECSection> runspecSection = std::make_shared<const RUNSPECSection>(*deck);
    ioConfigPtr->handleGridSection(gridSection);
    ioConfigPtr->handleRunspecSection(runspecSection);

    Schedule schedule(ParseContext() , grid , deck, ioConfigPtr);

    TimeMapConstPtr timemap   = schedule.getTimeMap();
    const TimeMap* const_tmap = timemap.get();
    TimeMap* writableTimemap  = const_cast<TimeMap*>(const_tmap);

    writableTimemap->initFirstTimestepsYears();
    writableTimemap->initFirstTimestepsMonths();


    //If no BASIC keyord has been handled, no restart files should be written
    for (size_t ts = 0; ts < timemap->numTimesteps(); ++ts) {
        BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(ts));
    }

    /*BASIC=1, write restart file for every timestep*/
    size_t timestep = 3;
    int basic       = 1;
    ioConfigPtr->handleRPTRSTBasic(schedule.getTimeMap(),timestep, basic);
    for (size_t ts = 0; ts < timemap->numTimesteps(); ++ts) {
        if (ts < 3) {
            BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(ts));
        } else {
            BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteRestartFile(ts));
        }
    }


    /* BASIC=3, restart files are created every nth report time, n=3 */
    timestep      = 11;
    basic         = 3;
    int frequency = 3;
    ioConfigPtr->handleRPTRSTBasic(schedule.getTimeMap(),timestep, basic, frequency);

    for (size_t ts = timestep ; ts < timemap->numTimesteps(); ++ts) {
        if ((ts >= timestep) && ((ts % frequency) == 0)) {
            BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteRestartFile(ts));
        } else {
            BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(ts));
        }
    }


    /* BASIC=4, restart file is written at the first report step of each year.
       Optionally, if the mnemonic FREQ is set >1 the restart is written only every n'th year*/
    timestep      = 17;
    basic         = 4;
    frequency     = 0;
    ioConfigPtr->handleRPTRSTBasic(schedule.getTimeMap(),timestep, basic, frequency);


    for (size_t ts = timestep; ts < timemap->numTimesteps(); ++ts) {
        ioConfigPtr->getWriteRestartFile(ts);
        if ((17 == ts) || (20 == ts) || (22 == ts) || (23 == ts) || (26 == ts) ||
            (27 == ts) || (30 == ts) || (32 == ts) || (33 == ts) || (36 == ts)) {
            BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteRestartFile(ts));
        } else {
            BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(ts));
        }
    }


    /* BASIC=4, restart file is written at the first report step of each year.
       Optionally, if the mnemonic FREQ is set >1 the restart is written only every n'th year*/
    timestep      = 18;
    basic         = 4;
    frequency     = 0;
    ioConfigPtr->handleRPTRSTBasic(schedule.getTimeMap(),timestep, basic, frequency);

    for (size_t ts = timestep; ts < timemap->numTimesteps(); ++ts) {
        ioConfigPtr->getWriteRestartFile(ts);
        if ((20 == ts) || (22 == ts) || (23 == ts) || (26 == ts) ||
            (27 == ts) || (30 == ts) || (32 == ts) || (33 == ts) || (36 == ts)) {
            BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteRestartFile(ts));
        } else {
            BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(ts));
        }
    }


    /* BASIC=4, FREQ = 2 restart file is written at the first report step of each year.
       Optionally, if the mnemonic FREQ is set >1 the restart is written only every n'th year*/
    timestep      = 27;
    basic         = 4;
    frequency     = 2;
    ioConfigPtr->handleRPTRSTBasic(schedule.getTimeMap(), timestep, basic, frequency);

    for (size_t ts = timestep; ts < timemap->numTimesteps(); ++ts) {
        if ((30 == ts) || (33 == ts)) {
            BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteRestartFile(ts));
        } else {
            BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(ts));
        }
    }



    /* BASIC=5, restart file is written at the first report step of each month.
       Optionally, if the mnemonic FREQ is set >1 the restart is written only every n'th month*/
    timestep      = 37;
    basic         = 5;
    frequency     = 2;
    ioConfigPtr->handleRPTRSTBasic(schedule.getTimeMap(), timestep, basic, frequency);

    for (size_t ts = timestep; ts < timemap->numTimesteps(); ++ts) {
        if ((38 == ts) || (44 == ts) || (48 == ts)) {
            BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteRestartFile(ts));
        } else {
            BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(ts));
        }
    }


    /* BASIC=0, no restart files are written*/
    timestep      = 47;
    basic         = 0;
    frequency     = 0;
    ioConfigPtr->handleRPTRSTBasic(schedule.getTimeMap(), timestep, basic, frequency);

    for (size_t ts = timestep; ts < timemap->numTimesteps(); ++ts) {
        BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(ts));
    }




    /************************* Test RPTSCHED RESTART *************************/

    /* RPTSCHED RESTART=1, restart files are written*/
    timestep       = 50;
    size_t restart = 1;
    ioConfigPtr->handleRPTSCHEDRestart(schedule.getTimeMap(), timestep, restart);

    BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteRestartFile(50));
    BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteRestartFile(51));

    /* RPTSCHED RESTART=0, no restart files are written*/
    timestep      = 52;
    restart       = 0;
    ioConfigPtr->handleRPTSCHEDRestart(schedule.getTimeMap(), timestep, restart);

    BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(52));
    BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(53));

    timestep      = 54;
    basic         = 0;
    ioConfigPtr->handleRPTSCHEDRestart(schedule.getTimeMap(), timestep, restart);


    /* RPTSCHED RESTART IGNORED IF RPTRST BASIC > 2 */
    timestep      = 56;
    basic         = 3;
    frequency     = 1;
    ioConfigPtr->handleRPTRSTBasic(schedule.getTimeMap(), timestep, basic, frequency);

    timestep      = 58;
    restart       = 0;
    writableTimemap->addTStep(boost::posix_time::hours(24));
    writableTimemap->addTStep(boost::posix_time::hours(24));
    BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteRestartFile(timestep));

    /* RPTSCHED RESTART IGNORED IF RPTRST BASIC > 2 */
    ioConfigPtr->handleRPTSCHEDRestart(schedule.getTimeMap(), timestep, restart);
    BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteRestartFile(timestep));

    /* RPTSCHED RESTART NOT IGNORED IF RPTRST BASIC <= 2 */
    timestep      = 60;
    basic         = 1;
    frequency     = 0;
    ioConfigPtr->handleRPTRSTBasic(schedule.getTimeMap(), timestep, basic, frequency);

    timestep      = 61;
    restart       = 0;
    writableTimemap->addTStep(boost::posix_time::hours(24));
    writableTimemap->addTStep(boost::posix_time::hours(24));
    BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteRestartFile(timestep));

    ioConfigPtr->handleRPTSCHEDRestart(schedule.getTimeMap(), timestep, restart);
    BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(timestep));

    /*Override, interval = 2*/
    ioConfigPtr->overrideRestartWriteInterval(2);
    for (size_t tstep = 0; tstep <= 61; ++tstep) {
        if ((tstep % 2) == 0) {
            BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteRestartFile(tstep));
        } else {
            BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(tstep));
        }
    }

    /*Override, turn off RESTART write*/
    ioConfigPtr->overrideRestartWriteInterval(0);
    for (size_t tstep = 0; tstep <= 61; ++tstep) {
        BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteRestartFile(tstep));
    }

    /*If no GRIDFILE nor NOGGF keywords are specified, default output an EGRID file*/
    BOOST_CHECK_EQUAL(true, ioConfigPtr->getWriteEGRIDFile());

    /*If no INIT keyword is specified, verify no write of INIT file*/
    BOOST_CHECK_EQUAL(false, ioConfigPtr->getWriteINITFile());

    /*If no UNIFIN keyword is specified, verify UNIFIN false (default is multiple) */
    BOOST_CHECK_EQUAL(false, ioConfigPtr->getUNIFIN());

    /*If no UNIFOUT keyword is specified, verify UNIFOUT false (default is multiple) */
    BOOST_CHECK_EQUAL(false, ioConfigPtr->getUNIFOUT());

    /*If no FMTIN keyword is specified, verify FMTIN false (default is unformatted) */
    BOOST_CHECK_EQUAL(false, ioConfigPtr->getFMTIN());

    /*If no FMTOUT keyword is specified, verify FMTOUT false (default is unformatted) */
    BOOST_CHECK_EQUAL(false, ioConfigPtr->getFMTOUT());

    /*If NOGGF keyword is present, no EGRID file is written*/
    DeckPtr deck3 = createDeck(deckStr_NOGGF);
    IOConfigPtr ioConfigPtr3;
    BOOST_CHECK_NO_THROW(ioConfigPtr3 = std::make_shared<IOConfig>());

    std::shared_ptr<const GRIDSection> gridSection3 = std::make_shared<const GRIDSection>(*deck3);
    std::shared_ptr<const RUNSPECSection> runspecSection3 = std::make_shared<const RUNSPECSection>(*deck3);
    ioConfigPtr3->handleGridSection(gridSection3);
    ioConfigPtr3->handleRunspecSection(runspecSection3);

    BOOST_CHECK_EQUAL(false, ioConfigPtr3->getWriteEGRIDFile());

    /*If INIT keyword is specified, verify write of INIT file*/
    BOOST_CHECK_EQUAL(true, ioConfigPtr3->getWriteINITFile());

    /*If UNIFOUT keyword is specified, verify unified write*/
    BOOST_CHECK_EQUAL(true, ioConfigPtr3->getUNIFOUT());

    /*If FMTOUT keyword is specified, verify formatted write*/
    BOOST_CHECK_EQUAL(true, ioConfigPtr3->getFMTOUT());

    /*If GRIDFILE 0 0 is specified, no EGRID file is written */
    DeckPtr deck4 = createDeck(deckStr_NO_GRIDFILE);
    IOConfigPtr ioConfigPtr4;
    BOOST_CHECK_NO_THROW(ioConfigPtr4 = std::make_shared<IOConfig>());

    std::shared_ptr<const GRIDSection> gridSection4 = std::make_shared<const GRIDSection>(*deck4);
    std::shared_ptr<const RUNSPECSection> runspecSection4 = std::make_shared<const RUNSPECSection>(*deck4);
    ioConfigPtr4->handleGridSection(gridSection4);
    ioConfigPtr4->handleRunspecSection(runspecSection4);

    BOOST_CHECK_EQUAL(false, ioConfigPtr4->getWriteEGRIDFile());

    IOConfigPtr ioConfigPtr5;
    BOOST_CHECK_NO_THROW(ioConfigPtr5 = std::make_shared<IOConfig>());
    BOOST_CHECK_EQUAL("", ioConfigPtr5->getBaseName());

    std::string testString = "testString.DATA";
    IOConfigPtr ioConfigPtr6 = std::make_shared<IOConfig>(testString);
    std::string output_dir6 =  ".";
    ioConfigPtr6->setOutputDir(output_dir6);
    BOOST_CHECK_EQUAL("testString", ioConfigPtr6->getBaseName());

    std::string absTestPath = "/path/to/testString.DATA";
    IOConfigPtr ioConfigPtr7 = std::make_shared<IOConfig>(absTestPath);
    std::string output_dir7 =  "/path/to";
    ioConfigPtr7->setOutputDir(output_dir7);
    BOOST_CHECK_EQUAL(output_dir7, ioConfigPtr7->getOutputDir());
    BOOST_CHECK_EQUAL("testString", ioConfigPtr7->getBaseName());

}



