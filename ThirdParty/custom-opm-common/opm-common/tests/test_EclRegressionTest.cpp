/*
   Copyright 2019 Equinor ASA.

   This file is part of the Open Porous Media project (OPM).

   OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   OPM is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with OPM.  If not, see <http://www.gnu.org/licenses/>.
   */

#include "config.h"

#define BOOST_TEST_MODULE EclRegressionTest

#include <boost/test/unit_test.hpp>

#include <test_util/EclRegressionTest.hpp>
#include <opm/output/eclipse/VectorItems/group.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>

#include <opm/io/eclipse/EGrid.hpp>
#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/EclOutput.hpp>

#include <iomanip>
#include "tests/WorkArea.hpp"

using Opm::EclIO::EGrid;
using Opm::EclIO::ESmry;
using Opm::EclIO::EclOutput;

void makeEgridFile(const std::string &fileName, const std::vector<float> &coord,
                   const std::vector<float> &zcorn, const std::vector<int> &gridhead,
		               const std::vector<int> &filehead,
                   const std::vector<std::string> &gridunits,
                   const std::vector<int> actnum, const std::vector<int> &nnc1,
                   const std::vector<int> &nnc2)
{
    EclOutput eclTest(fileName, false);

    eclTest.write("FILEHEAD", filehead);
    eclTest.write("GRIDUNIT", gridunits);
    eclTest.write("GRIDHEAD", gridhead);
    eclTest.write("COORD", coord);
    eclTest.write("ZCORN", zcorn);
    if (actnum.size() > 0) {
        eclTest.write("ACTNUM", actnum);
    }

    eclTest.write("ENDGRID",std::vector<int>());

    if (!nnc1.empty() && !nnc2.empty()) {
        std::vector<int> nnchead(10,0);
        nnchead[0] = nnc1.size();

        eclTest.write("NNCHEAD", nnchead);
	      eclTest.write("NNC1", nnc1);
        eclTest.write("NNC2", nnc2);
    }
}


void makeInitFile(const std::string &fileName, std::vector<std::string> floatKeys, std::vector<std::vector<float>> floatData, std::vector<std::string> intKeys, std::vector<std::vector<int>> intData){

    std::vector<double> doubhead = {0.0,1,0,365,0.10000000149012E+00,0.15000000596046E+00,0.30000000000000E+01};
    std::vector<int> intehead = {-957688424,201702,1,-2345,-2345,-2345,-2345,-2345,2,3,2,12,6,0,1,-2345,0,10,0,10,11,0,0,0,155,122,130,3,107,112,1,-2345,25,40,58,
                -2345,107,112,180,5,0,1,18,24,10,7,2,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,2000,0,0,0,1,0,0,0,0,0,1,10,0,0,12,1,25,1,-2345,-2345,
	              8,8,3,4,2,3,2,1,100,0,6,0,-17,1,0,1,0,1,0,2,3,2,12,1,1,1,1,2,3,2,25,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,14,11,10,17,2,1,1,1,1,11,1,1,1,
                1,1,1,98,122,0,0,0,0,0,0,1,10,4,5,9,0,6,8,8,12,1,25,1,-1073741823,-1073741823,-1073741823,-1073741823,0,1,1,1,22,126,10,1,1,1,1,22,
                122,-1073741823,-1073741823,0,0,130,58,180,10,0,25,155,0,0,1,10,122,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,0,-1,12,0,0,10,13,1,0,0,0,0,2,0,0,
                3600,1,6,1,10,1,10,1,1,1,0,30,3,26,16,13,6,0,0,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,1,0,0,1,2,0,0,1,1,1,1,2,1,0,2,0,2,0,2,1,12,0,0,0,0,1,0,
                0,0,1,0,0,0,1,0,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,
                -2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,
                -2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,
                -2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,
                -2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,-2345,
                -2345,0};

    std::vector<bool> logihead = {false,false,false,true,false,false,false,false,true,false,false,false,false,false,false};

    EclOutput eclTest(fileName, false);

    eclTest.write("INTEHEAD",intehead);
    eclTest.write("LOGIHEAD",logihead);
    eclTest.write("DOUBHEAD",doubhead);

    for (size_t i = 0; i < floatKeys.size(); i++) {
        eclTest.write(floatKeys[i], floatData[i]);
    }

    for (size_t i = 0; i < intKeys.size(); i++) {
        eclTest.write(intKeys[i], intData[i]);
    }
}

namespace VI = Opm::RestartIO::Helpers::VectorItems;

void makeUnrstFile(const std::string &fileName, std::vector<int> seqnum,
                   const std::vector<std::tuple<int,int,int>>& dates,
                   const std::vector<double>& time,
		               const std::vector<bool>& logihead,
                   std::vector<double>& doubhead,
                   const std::vector<std::string>& zgrp,
                   const std::vector<int>& iwel,
		               const std::vector<std::string>& solutionNames,
                   const std::vector<std::vector<std::vector<float>>>& solutions)
{
    std::vector<int> intehead= {-957688424,201702,1,-2345,-2345,-2345,-2345,-2345,2,3,2,12,6,0,1,-2345,0,10,0,10,11,0,0,0,155,122,130,3,107,112,1,-2345,25,40,58,
                                -2345,107,112,180,5,0,1,18,24,10,7,2,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,2000,0,0,0,1,0,0,0,0,0,1,10,0,0,12,1,25,1,-2345,-2345,8,8,3,4,2,3,2,1,100};

    intehead.resize(411, 0);
    intehead[VI::intehead::NWMAXZ] = intehead[VI::intehead::NWELLS];

    EclOutput eclTest(fileName, false);

    for (size_t i = 0; i < seqnum.size(); i++) {
        std::vector<int> seqnumVect;
        seqnumVect.push_back(seqnum[i]);

        eclTest.write("SEQNUM", seqnumVect);

        intehead[66]=std::get<0>(dates[i]);
        intehead[65]=std::get<1>(dates[i]);
        intehead[64]=std::get<2>(dates[i]);

        eclTest.write("INTEHEAD", intehead);

        eclTest.write("LOGIHEAD", logihead);

        doubhead[0] = time[i];

        eclTest.write("DOUBHEAD", doubhead);
        eclTest.write("ZGRP", zgrp);
        eclTest.write("IWEL", iwel);


        // The blocks added below for groups, wells and connections respectively
        // is just adding default data of correct consistent size to be able to
        // load restart data. The content of these vectors is never used/checked.
        {
            std::size_t num_groups = intehead[VI::intehead::NGRP] + 1;
            std::size_t nigrpz = intehead[VI::intehead::NIGRPZ];
            std::size_t nsgrpz = intehead[VI::intehead::NSGRPZ];
            std::size_t nxgrpz = intehead[VI::intehead::NXGRPZ];

            std::vector<int> igrp( num_groups * nigrpz );
            std::vector<float> sgrp( num_groups * nsgrpz );
            std::vector<double> xgrp( num_groups * nxgrpz );

            eclTest.write("IGRP", igrp);
            eclTest.write("SGRP", sgrp);
            eclTest.write("XGRP", xgrp);
        }
        {
            std::size_t num_wells = intehead[VI::intehead::NWELLS];
            std::size_t nzwelz = intehead[VI::intehead::NZWELZ];
            std::size_t nswelz = intehead[VI::intehead::NSWELZ];
            std::size_t nxwelz = intehead[VI::intehead::NXWELZ];

            std::vector<std::string> zwel( num_wells * nzwelz );
            std::vector<float> swel( num_wells * nswelz );
            std::vector<double> xwel( num_wells * nxwelz );

            eclTest.write("ZWEL", zwel);
            eclTest.write("SWEL", swel);
            eclTest.write("XWEL", xwel);
        }
        {
            std::size_t num_wells = intehead[VI::intehead::NWELLS];
            std::size_t num_connections = num_wells * intehead[VI::intehead::NCWMAX];
            std::size_t niconz = intehead[VI::intehead::NICONZ];
            std::size_t nsconz = intehead[VI::intehead::NSCONZ];
            std::size_t nxconz = intehead[VI::intehead::NXCONZ];

            std::vector<int> icon( num_connections * niconz );
            std::vector<float> scon( num_connections * nsconz );
            std::vector<double> xcon( num_connections * nxconz );

            eclTest.write("ICON", icon);
            eclTest.write("SCON", scon);
            eclTest.write("XCON", xcon);
        }
        eclTest.write("STARTSOL", std::vector<char>());

        for (size_t n = 0; n < solutionNames.size(); n++) {
            eclTest.write(solutionNames[n], solutions[n][i]);
        }

        eclTest.write("ENDSOL", std::vector<char>());
    }
}


void makeSmryFile(const std::string &fileName,
                  const std::vector<std::string>& keywords,
                  const std::vector<std::string>& wgnames,
                  const std::vector<int>& nums,
                  const std::vector<std::string>& units,
                  const std::vector<std::vector<float>>& params)
{
    std::vector<int> intehead = {1,100};
    std::vector<std::string> restart = {"","","","","","","","",""};
    std::vector<int> dimens = {-1, 2, 3, 1, 0, -1};
    std::vector<int> startd= {1,1,2000,0,0,0};

    std::vector<int> seqhdr = {0,4,5,6,7,8,9};

    std::vector<int> seqhdrValue = {-957426774};

    dimens[0] = params[0].size();

    EclOutput eclSmspecTest(fileName, false);

    eclSmspecTest.write("INTEHEAD", intehead);
    eclSmspecTest.write("RESTART", restart);
    eclSmspecTest.write("DIMENS", dimens);
    eclSmspecTest.write("KEYWORDS", keywords);
    eclSmspecTest.write("WGNAMES", wgnames);
    eclSmspecTest.write("NUMS", nums);
    eclSmspecTest.write("UNITS", units);
    eclSmspecTest.write("STARTDAT", startd);

    int strL = fileName.size();
    std::string unsmryFilename = fileName.substr(0,strL-6)+"UNSMRY";

    EclOutput eclUnsmryTest(unsmryFilename, false);

    int nSteps = params.size();

    for (int i = 0; i < nSteps; i++) {
        auto search = std::find(seqhdr.begin(), seqhdr.end(), i);

        if (search != seqhdr.end()) {
            eclUnsmryTest.write<int>("SEQHDR", {1});
        }

        std::vector<int> ministep;
        ministep.push_back(i);

        eclUnsmryTest.write<int>("MINISTEP", {1});
        eclUnsmryTest.write("PARAMS", params[i]);
    }
}


void makeRftFile(const std::string &fileName,
                 const std::vector<float>& time,
                 const std::vector<std::tuple<int, int, int>>& date,
                 const std::vector<std::string>& wellN,
                 const std::vector<std::vector<int>>& conipos,
                 const std::vector<std::vector<int>>& conjpos,
                 const std::vector<std::vector<int>>& conkpos,
                 const std::vector<std::vector<float>>& depth,
                 const std::vector<std::string>& solutionNames,
                 const std::vector<std::vector<std::vector<float>>>& solutions)
{
    std::vector<std::string> welletc = {"  DAYS", "A-1H", "", " METRES", "  BARSA", "R", "STANDARD", " SM3/DAY", " SM3/DAY", " RM3/DAY", " M/SEC", "", "   CP", " KG/SM3", " KG/DAY ", "  KG/KG"};

    int nRfts = time.size();

    EclOutput eclRftTest(fileName, false);
    for (int i = 0; i < nRfts; i++){
        std::vector<float> timeVect;
        timeVect.push_back(time[i]);

        eclRftTest.write("TIME",timeVect);

        std::vector<int> dateVect;
        dateVect.push_back(std::get<2>(date[i]));
        dateVect.push_back(std::get<1>(date[i]));
        dateVect.push_back(std::get<0>(date[i]));

        eclRftTest.write("DATE", dateVect);

        welletc[1] = wellN[i];
        eclRftTest.write("WELLETC", welletc);

        eclRftTest.write("CONIPOS", conipos[i]);
        eclRftTest.write("CONJPOS", conjpos[i]);
        eclRftTest.write("CONKPOS", conkpos[i]);

        eclRftTest.write("DEPTH", depth[i]);

        for (size_t n = 0; n < solutionNames.size(); n++) {
           eclRftTest.write(solutionNames[n], solutions[n][i]);
        }
    }
}


 
BOOST_AUTO_TEST_CASE(gridCompare) {

    std::vector<float> coord = {2000,2000,2000,1999.9127,1999.8691,2009.9951,2099.9849,2000,2001.7452,2099.8975,1999.8691,2011.7404,2199.9695,2000,2003.4905,2199.8823,
           1999.8691,2013.4855,2000,2099.9658,2002.6177,1999.9127,2099.8347,2012.6127,2099.9849,2099.9658,2007.3629,2099.8975,2099.8347,2017.358,2199.9695,2099.9658,
	   2009.1082,2199.8823,2099.8347,2019.1031,2000,2199.9314,2005.2354,1999.9127,2199.8005,2015.2303,2099.9849,2199.9314,2009.9806,2099.8975,2199.8005,2019.9757,
	   2199.9695,2199.9314,2011.726,2199.8823,2199.8005,2021.7209,2000,2299.8972,2007.8531,1999.9127,2299.7664,2017.8481,2099.9849,2299.8972,2012.5983,2099.8975,
	   2299.7664,2022.5934,2199.9695,2299.8972,2014.3436,2199.8823,2299.7664,2024.3386};

    std::vector<float> zcorn = {2000,2001.7452,2001.7452,2003.4905,2002.6177,2004.3629,2004.3629,2006.1082,2002.6177,2004.3629,2007.3629,2009.1082,2005.2354,
           2006.9806,2009.9806,2011.726,2005.2354,2006.9806,2009.9806,2011.726,2007.8531,2009.5983,2012.5983,2014.3436,2004.9976,2006.7428,2006.7428,2008.488,
	   2007.6152,2009.3605,2009.3605,2011.1057,2007.6152,2009.3605,2012.3605,2014.1057,2010.2329,2011.9781,2014.9781,2016.7234,2010.2329,2011.9781,2014.9781,
	   2016.7234,2012.8506,2014.5959,2017.5959,2019.3411,2004.9976,2006.7428,2006.7428,2008.488,2007.6152,2009.3605,2009.3605,2011.1057,2007.6152,2009.3605,
	   2012.3605,2014.1057,2010.2329,2011.9781,2014.9781,2016.7234,2010.2329,2011.9781,2014.9781,2016.7234,2012.8506,2014.5959,2017.5959,2019.3411,2009.9951,
	   2011.7404,2011.7404,2013.4855,2012.6127,2014.358,2014.358,2016.1031,2012.6127,2014.358,2017.358,2019.1031,2015.2303,2016.9757,2019.9757,2021.7209,
	   2015.2303,2016.9757,2019.9757,2021.7209,2017.8481,2019.5934,2022.5934,2024.3386};

    std::vector<int> gridhead = {1,2,3,2,0,0};
    std::vector<int> filehead = {3,0,0,0,0,0};

    std::vector<int> nnc1;
    std::vector<int> nnc2;
    std::vector<int> actnum;
    WorkArea work;

    //-------------------------------------------------------------
    // base:  identical grids

    std::vector<std::string> gridunits= {"METRES", ""};
    std::vector<std::string> gdorient= {"INC", "INC", "INC", "DOWN", "RIGHT"};

    makeEgridFile("TMP1.EGRID",coord, zcorn, gridhead, filehead, gridunits, actnum, nnc1, nnc2);
    makeEgridFile("TMP2.EGRID",coord, zcorn, gridhead, filehead, gridunits, actnum, nnc1, nnc2);

    ECLRegressionTest base("TMP1", "TMP2", 1e-4, 1e-4);

    base.loadGrids();
    base.gridCompare();

    //-------------------------------------------------------------
    // test1:  add actnum array, identical

    actnum.assign(12,1);
    actnum[0]=0;

    makeEgridFile("TMP1.EGRID",coord, zcorn, gridhead, filehead, gridunits, actnum, nnc1, nnc2);
    makeEgridFile("TMP2.EGRID",coord, zcorn, gridhead, filehead, gridunits, actnum, nnc1, nnc2);

    ECLRegressionTest test1("TMP1", "TMP2", 1e-4, 1e-4);

    test1.loadGrids();
    test1.gridCompare();


    //-------------------------------------------------------------
    // test2:  adding nncs, still identical

    nnc1 = {8, 9, 11};
    nnc2 = {4, 4, 6};

    makeEgridFile("TMP1.EGRID",coord, zcorn, gridhead, filehead, gridunits, actnum, nnc1, nnc2);
    makeEgridFile("TMP2.EGRID",coord, zcorn, gridhead, filehead, gridunits, actnum, nnc1, nnc2);

    ECLRegressionTest test2("TMP1", "TMP2", 1e-4, 1e-4);

    test2.loadGrids();
    test2.gridCompare();

    //-------------------------------------------------------------
    // test 3: changing two grid pillars, should fail

    std::vector<float> coordTest3=coord;
    coordTest3[12]=coordTest3[12]+0.1;
    coordTest3[52]=coordTest3[52]+0.1;

    makeEgridFile("TMP2.EGRID",coordTest3, zcorn, gridhead, filehead, gridunits, actnum, nnc1, nnc2);

    ECLRegressionTest test3("TMP1", "TMP2", 1e-3, 1e-3);

    test3.loadGrids();
    BOOST_CHECK_THROW(test3.gridCompare(),std::runtime_error);

    // do full analysis
    test3.doAnalysis(true);
    test3.gridCompare();

    // 2 keywords should exhibit failures (x and y coordinates)
    BOOST_CHECK_EQUAL(test3.countDev(),2);
    //-------------------------------------------------------------
    // test 4: changing two zcorn values, should fail

    std::vector<float> zcornTest4=zcorn;
    zcornTest4[15]=zcornTest4[15]+0.1;
    zcornTest4[52]=zcornTest4[52]+0.1;

    makeEgridFile("TMP2.EGRID",coord, zcornTest4, gridhead, filehead, gridunits, actnum, nnc1, nnc2);

    ECLRegressionTest test4("TMP1", "TMP2", 1e-3, 1e-3);

    test4.loadGrids();

    BOOST_CHECK_THROW(test4.gridCompare(),std::runtime_error);

    test4.doAnalysis(true);
    test4.gridCompare();

    BOOST_CHECK_EQUAL(test4.countDev(),3);

    //-------------------------------------------------------------
    // test 5: add one nnc for TMP2, should fail

    std::vector<int> nnc1_5=nnc1;
    std::vector<int> nnc2_5=nnc2;

    nnc1_5.push_back(1);
    nnc2_5.push_back(12);

    makeEgridFile("TMP2.EGRID",coord, zcorn, gridhead, filehead, gridunits, actnum, nnc1_5, nnc2_5);

    ECLRegressionTest test5("TMP1", "TMP2", 1e-3, 1e-3);

    test5.loadGrids();

    BOOST_CHECK_THROW(test5.gridCompare(),std::runtime_error);

    //-------------------------------------------------------------
    // test 6: different definition of active cells in TMP2, should fail

    std::vector<int> actnum6=actnum;
    actnum6[10]=0;

    makeEgridFile("TMP2.EGRID",coord, zcorn, gridhead, filehead, gridunits, actnum6, nnc1, nnc2);

    ECLRegressionTest test6("TMP1", "TMP2", 1e-3, 1e-3);

    test6.loadGrids();

    BOOST_CHECK_THROW(test6.gridCompare(),std::runtime_error);
}

BOOST_AUTO_TEST_CASE(results_init_1) {
    WorkArea work;

    std::vector<std::vector<int>> intData1;
    std::vector<std::vector<float>> floatData1;

    std::vector<std::vector<int>> intData2;
    std::vector<std::vector<float>> floatData2;

    std::vector<float> permx1(12,1000.0);
    std::vector<float> porv1(12,1000.0);
    std::vector<float> poro1(12,0.25);
    std::vector<int> fipnum1(12,1);

    std::vector<float> permx2(12,1000.0);
    std::vector<float> porv2(12,1000.0);
    std::vector<float> poro2(12,0.25);
    std::vector<int> fipnum2(12,1);

    // -- TMP1 vectors

    floatData1.push_back(porv1);
    floatData1.push_back(permx1);
    intData1.push_back(fipnum1);

    std::vector<std::string> intKeys1={"FIPNUM"};
    std::vector<std::string> floatKeys1={"PORV","PERMX"};

    // -- TMP2 vectors

    floatData2.push_back(porv2);
    floatData2.push_back(permx2);
    floatData2.push_back(poro2);
    intData2.push_back(fipnum2);

    std::vector<std::string> intKeys2={"FIPNUM"};
    std::vector<std::string> floatKeys2={"PORV","PERMX","PORO"};

    makeInitFile("TMP1.INIT", floatKeys1,floatData1,intKeys1, intData1);
    makeInitFile("TMP2.INIT", floatKeys2,floatData2,intKeys2, intData2);

    // ---------------------------------------------------------------------------
    // test accept extra keywords in second case

    ECLRegressionTest test1("TMP1", "TMP2", 1e-3, 1e-3);

    // should fail, since extra vector PORO in second case (TMP2)
    BOOST_CHECK_THROW(test1.results_init(),std::runtime_error);

    // enabeling extra keywords in second case, all vectors are identical except for extra vector PORO
    test1.setAcceptExtraKeywords(true);

    // should be ok
    test1.results_init();

    // new test with extra keyword in first case
    ECLRegressionTest test1a("TMP2", "TMP1", 1e-3, 1e-3);

    test1a.setAcceptExtraKeywords(true);

    // should fail, even though using accept extra keywords option
    BOOST_CHECK_THROW(test1a.results_init(),std::runtime_error);

    // check with spesific kewyword PORV, found in both cases and should be ok
    test1a.setAcceptExtraKeywords(false);
    test1a.compareSpesificKeyword("PORV");

    test1a.results_init();

    // check with spesific kewyword PORO, found in second case only, should throw exeption
    test1a.compareSpesificKeyword("PORO");

    BOOST_CHECK_THROW(test1a.results_init(),std::runtime_error);

    // check with spesific kewyword not found in any of the cases, should throw exeption
    test1a.compareSpesificKeyword("XXXXX");

    BOOST_CHECK_THROW(test1a.results_init(),std::runtime_error);

}

BOOST_AUTO_TEST_CASE(results_init_2) {

    std::vector<std::vector<int>> intData1;
    std::vector<std::vector<float>> floatData1;

    std::vector<std::vector<int>> intData2;
    std::vector<std::vector<float>> floatData2;

    std::vector<float> permx1(12,1000.0);
    std::vector<float> porv1(12,1000.0);
    std::vector<float> poro1(12,0.25);
    std::vector<int> fipnum1(12,1);

    std::vector<float> permx2(12,1000.0);
    std::vector<float> porv2(12,1000.0);
    std::vector<float> poro2(12,0.25);
    std::vector<int> fipnum2(12,1);

    WorkArea work;
    // ---------------------------------------------------------------------------
    // array PORV requires strict tolerances, 1e-6

    floatData1.push_back(porv1);
    floatData1.push_back(permx1);
    intData1.push_back(fipnum1);

    porv2[2]=999.9999;      // deviation less that 1e-6

    floatData2.push_back(porv2);
    floatData2.push_back(permx2);
    intData2.push_back(fipnum2);

    std::vector<std::string> intKeys={"FIPNUM"};
    std::vector<std::string> floatKeys={"PORV","PERMX"};

    makeInitFile("TMP1.INIT", floatKeys,floatData1,intKeys, intData1);
    makeInitFile("TMP2.INIT", floatKeys,floatData2,intKeys, intData2);

    ECLRegressionTest test2("TMP1", "TMP2", 1e-3, 1e-3);
    test2.results_init();

    porv2=porv1;
    porv2[2]=999.998;    // relativ deviation 2e-6 > tolerances, abs deviaton 2e-3 > tolerances

    floatData2.clear();
    floatData2.push_back(porv2);
    floatData2.push_back(permx2);

    makeInitFile("TMP1.INIT", floatKeys,floatData1,intKeys, intData1);
    makeInitFile("TMP2.INIT", floatKeys,floatData2,intKeys, intData2);

    ECLRegressionTest test2a("TMP1", "TMP2", 1e-3, 1e-3);

   // should fail, both relativ and absolute error > tolerance
    BOOST_CHECK_THROW(test2a.results_init(),std::runtime_error);

    test2a.doAnalysis(true);
    test2a.results_init();

    // should be one keyword with failure
    BOOST_CHECK_EQUAL(test2a.countDev(),1);

    // ---------------------------------------------------------------------------
    // compare specific keyword, should be ok sinze PORV not checked in this case

    test2a.compareSpesificKeyword("PERMX");
    test2a.doAnalysis(false);
    test2a.results_init();

    // ---------------------------------------------------------------------------
    // fipnum is different (not floading point vector)

    floatData1.clear();
    floatData2.clear();
    intData1.clear();
    intData2.clear();

    fipnum2=fipnum1;
    fipnum2[8]=2;

    floatData1.push_back(porv1);
    floatData1.push_back(permx1);

    intData1.push_back(fipnum1);

    floatData2.push_back(porv2);
    floatData2.push_back(permx2);

    intData2.push_back(fipnum2);

    makeInitFile("TMP1.INIT", floatKeys,floatData1,intKeys, intData1);
    makeInitFile("TMP2.INIT", floatKeys,floatData2,intKeys, intData2);

    ECLRegressionTest test3("TMP1", "TMP2", 1e-3, 1e-3);

    BOOST_CHECK_THROW(test3.results_init(),std::runtime_error);
}

BOOST_AUTO_TEST_CASE(results_unrst_1) {
    WorkArea work;
    using Date = std::tuple<int, int, int>;

    std::vector<int> seqnum1 = {0,1,4,7};
    std::vector<Date> dates1 = {
        Date{2000,1, 1},
        Date{2000,1,10},
        Date{2000,2, 1},
        Date{2000,3, 1}
    };
    std::vector<bool> logihead1(121, false);
    logihead1[3] = logihead1[8] = true;
    std::vector<double> doubhead1 = {0.0,1,0, 365, 0.10000000149012E+00,0.15000000596046E+00,0.30000000000000E+01};
    doubhead1.resize(229, 0.0);
    std::vector<double> time1 = {0, 9, 31,60};

    std::vector<std::vector<float>> pressure1 = {{210,210.1,210.2,210.05,210.15,210.25},{200,200.1,200.2,200.05,200.15,200.25},
                                                 {190,190.1,190.2,190.05,190.15,190.25},{185,185.1,185.2,185.05,185.15,185.25}};

    std::vector<std::vector<float>> rs1 = {{150,150.1,150.2,150.05,150.15,150.25},{160,160.1,160.2,160.05,160.15,160.25},
                                            {165,165.1,165.2,165.05,165.15,165.25},{168,168.1,168.2,168.05,168.15,168.25}};

    std::vector<std::string> zgrp1 = {"GRP1", "GRP2"};
    std::vector<int> iwel1 = {1,4,6,8};

    std::vector<std::string> solutionNames1;
    std::vector<std::vector<std::vector<float>>> solutions1;

    solutionNames1={"PRESSURE","RS"};
    solutions1.push_back(pressure1);
    solutions1.push_back(rs1);

// -------------------------

    std::vector<int> seqnum2 = {0,1,4,7};
    std::vector<Date> dates2 = {
        Date{2000,1, 1},
        Date{2000,1,10},
        Date{2000,2, 1},
        Date{2000,3, 1}
    };

    std::vector<bool> logihead2(121, false);
    logihead2[3] = logihead2[8] = true;
    std::vector<double> doubhead2 = {0.0,1,0, 365, 0.10000000149012E+00,0.15000000596046E+00,0.30000000000000E+01};
    doubhead2.resize(229, 0.0);
    std::vector<double> time2 = {0, 9, 31,60};

    std::vector<std::vector<float>> pressure2 = {{210,210.1,210.2,210.05,210.15,210.25},{200,200.1,200.2,200.05,200.15,200.25},
                                                 {190,190.1,190.2,190.05,190.15,190.25},{185,185.1,185.2,185.05,185.15,185.25}};

    std::vector<std::vector<float>> rs2 = {{150,150.1,150.2,150.05,150.15,150.25},{160,160.1,160.2,160.05,160.15,160.25},
                                            {165,165.1,165.2,165.05,165.15,165.25},{168,168.1,168.2,168.05,168.15,168.25}};

    std::vector<std::string> zgrp2 = {"GRP1", "GRP2"};

    std::vector<int> iwel2 = {1,4,6,8};

    std::vector<std::string> solutionNames2;
    std::vector<std::vector<std::vector<float>>> solutions2;

    solutionNames2={"PRESSURE","RS"};
    solutions2.push_back(pressure2);
    solutions2.push_back(rs2);

// -------------------------

    makeUnrstFile("TMP1.UNRST", seqnum1, dates1, time1, logihead1, doubhead1, zgrp1, iwel1,solutionNames1,solutions1);
    makeUnrstFile("TMP2.UNRST", seqnum2, dates2, time2, logihead2, doubhead2, zgrp2, iwel2,solutionNames2,solutions2);

    ECLRegressionTest test1("TMP1", "TMP2", 1e-3, 1e-3);
    test1.results_rst();

    // different length of vector zgrp, should throw exception
    zgrp2 = {"GRP1"};
    makeUnrstFile("TMP2.UNRST", seqnum2, dates2, time2, logihead2, doubhead2, zgrp2, iwel2,solutionNames2,solutions2);

    ECLRegressionTest test1a("TMP1", "TMP2", 1e-3, 1e-3);
    BOOST_CHECK_THROW(test1a.results_rst(),std::runtime_error);

    // rs vector missing in second case
    // zgrp2 equal to zgrp1

    zgrp2 = {"GRP1", "GRP2"};

    solutionNames2={"PRESSURE"};
    solutions2.clear();
    solutions2.push_back(pressure2);

    makeUnrstFile("TMP2.UNRST", seqnum2, dates2, time2, logihead2, doubhead2, zgrp2, iwel2,solutionNames2,solutions2);

    // should fail, not same keywords missing keywords in first case
    ECLRegressionTest test2("TMP1", "TMP2", 1e-3, 1e-3);

    BOOST_CHECK_THROW(test2.results_rst(),std::runtime_error);

    // use accept extra keywords, should still fail
    test2.setAcceptExtraKeywords(true);

    BOOST_CHECK_THROW(test2.results_rst(),std::runtime_error);

    // new test, using TMP2 as first case
    ECLRegressionTest test2a("TMP2", "TMP1", 1e-3, 1e-3);

    test2a.setAcceptExtraKeywords(true);
    test2a.results_rst();

    // checking for spesific keyword PRESSURE, found in both cases
    test2a.setAcceptExtraKeywords(false);
    test2a.compareSpesificKeyword("PRESSURE");

    // test should be ok
    test2a.results_rst();

    // checking for spesific keyword RS, only present in one of the cases
    test2a.compareSpesificKeyword("RS");

    // should fail
    BOOST_CHECK_THROW(test2a.results_rst(),std::runtime_error);

    // checking for spesific keyword XXXX, not found in any of the cases
    test2a.compareSpesificKeyword("XXXX");

    // should fail
    BOOST_CHECK_THROW(test2a.results_rst(),std::runtime_error);
}

BOOST_AUTO_TEST_CASE(results_unrst_2) {
    WorkArea work;
    using Date = std::tuple<int, int, int>;

    std::vector<int> seqnum1 = {0,1,4,7};
    std::vector<Date> dates1 = {
        Date{2000,1, 1},
        Date{2000,1,10},
        Date{2000,2, 1},
        Date{2000,3, 1}
    };

    std::vector<bool> logihead1(121, false);
    logihead1[3] = logihead1[8] = true;
    std::vector<double> doubhead1 = {0.0,1,0, 365, 0.10000000149012E+00,0.15000000596046E+00,0.30000000000000E+01};
    doubhead1.resize(229, 0.0);
    std::vector<double> time1 = {0, 9, 31,60};

    std::vector<std::vector<float>> pressure1 = {{210,210.1,210.2,210.05,210.15,210.25},{200,200.1,200.2,200.05,200.15,200.25},
                                                 {190,190.1,190.2,190.05,190.15,190.25},{185,185.1,185.2,185.05,185.15,185.25}};

    std::vector<std::vector<float>> rs1 = {{150,150.1,150.2,150.05,150.15,150.25},{160,160.1,160.2,160.05,160.15,160.25},
                                            {165,165.1,165.2,165.05,165.15,165.25},{168,168.1,168.2,168.05,168.15,168.25}};

    std::vector<std::string> zgrp1 = {"GRP1", "GRP2"};
    std::vector<int> iwel1 = {1,4,6,8};

    std::vector<std::string> solutionNames1;
    std::vector<std::vector<std::vector<float>>> solutions1;

    solutionNames1={"PRESSURE","RS"};
    solutions1.push_back(pressure1);
    solutions1.push_back(rs1);

// -------------------------

// reportStepNumber #4 missing in second case

    std::vector<int> seqnum2 = {0,1,7};
    std::vector<Date> dates2 = {
        Date{2000,1, 1},
        Date{2000,1,10},
        Date{2000,3, 1}
    };

    std::vector<bool> logihead2(121, false);
    logihead2[3] = logihead2[8] = true;
    std::vector<double> doubhead2 = {0.0,1,0, 365, 0.10000000149012E+00,0.15000000596046E+00,0.30000000000000E+01};
    doubhead2.resize(229, 0.0);
    std::vector<double> time2 = {0, 9, 60};

    std::vector<std::vector<float>> pressure2 = {{210,210.1,210.2,210.05,210.15,210.25},{200,200.1,200.2,200.05,200.15,200.25},
                                                 {185,185.1,185.2,185.05,185.15,185.25}};

    std::vector<std::vector<float>> rs2 = {{150,150.1,150.2,150.05,150.15,150.25},{160,160.1,160.2,160.05,160.15,160.25},
                                            {168,168.1,168.2,168.05,168.15,168.25}};

    std::vector<std::string> zgrp2 = {"GRP1", "GRP2"};

    std::vector<int> iwel2 = {1,4,6,8};

    std::vector<std::string> solutionNames2;
    std::vector<std::vector<std::vector<float>>> solutions2;

    solutionNames2={"PRESSURE","RS"};
    solutions2.push_back(pressure2);
    solutions2.push_back(rs2);

// -------------------------

    makeUnrstFile("TMP1.UNRST", seqnum1, dates1, time1, logihead1, doubhead1, zgrp1, iwel1,solutionNames1,solutions1);
    makeUnrstFile("TMP2.UNRST", seqnum2, dates2, time2, logihead2, doubhead2, zgrp2, iwel2,solutionNames2,solutions2);

    // should fail since sequence #4 is missing in second file"

    ECLRegressionTest test1("TMP1", "TMP2", 1e-3, 1e-3);
    BOOST_CHECK_THROW(test1.results_rst(),std::runtime_error);

    // compare only last report step number, should be ok
    test1.setOnlyLastReportNumber(true);
    test1.results_rst();

    // compare only sequence # 1, should be ok
    test1.setOnlyLastReportNumber(false);
    test1.compareSpesificRstReportStepNumber(1);

    test1.results_rst();

}


BOOST_AUTO_TEST_CASE(results_unrst_3) {
    WorkArea work;
    using Date = std::tuple<int, int, int>;

    std::vector<int> seqnum1 = {0,1,4,7};
    std::vector<Date> dates1 = {
        Date{2000,1, 1},
        Date{2000,1,10},
        Date{2000,2, 1},
        Date{2000,3, 1},
    };
    std::vector<bool> logihead1(121, false);
    logihead1[3] = logihead1[8] = true;
    std::vector<double> doubhead1 = {0.0,1,0, 365, 0.10000000149012E+00,0.15000000596046E+00,0.30000000000000E+01};
    doubhead1.resize(229, 0.0);
    std::vector<double> time1 = {0, 9, 31,60};

    std::vector<std::vector<float>> pressure1 = {{210,210.1,210.2,210.05,210.15,210.25},{200,200.1,200.2,200.05,200.15,200.25},
                                                 {190,190.1,190.2,190.05,190.15,190.25},{185,185.1,185.2,185.05,185.15,185.25}};

    std::vector<std::vector<float>> rs1 = {{150,150.1,150.2,150.05,150.15,150.25},{160,160.1,160.2,160.05,160.15,160.25},
                                            {165,165.1,165.2,165.05,165.15,165.25},{168,168.1,168.2,168.05,168.15,168.25}};

    std::vector<std::string> zgrp1 = {"GRP1", "GRP2"};
    std::vector<int> iwel1 = {1,4,6,8};

    std::vector<std::string> solutionNames1;
    std::vector<std::vector<std::vector<float>>> solutions1;

    solutionNames1={"PRESSURE","RS"};
    solutions1.push_back(pressure1);
    solutions1.push_back(rs1);

// -------------------------

    std::vector<int> seqnum2 = {0,1,4,7};
    std::vector<Date> dates2 = {
        Date{2000,1, 1},
        Date{2000,1,10},
        Date{2000,2, 1},
        Date{2000,3, 1},
    };
    std::vector<bool> logihead2(121, false);
    logihead2[3] = logihead2[8] = true;
    std::vector<double> doubhead2 = {0.0,1,0, 365, 0.10000000149012E+00,0.15000000596046E+00,0.30000000000000E+01};
    doubhead2.resize(229, 0.0);
    std::vector<double> time2 = {0, 9, 31,60};

    std::vector<std::vector<float>> pressure2 = {{210,210.1,210.2,210.05,210.15,210.25},{200,200.1,200.2,200.05,200.15,200.25},
                                                 {190,190.1,190.2,190.05,190.15,190.25},{185,185.1,185.2,185.05,185.15,185.25}};

    std::vector<std::vector<float>> rs2 = {{150,150.1,150.2,150.05,150.15,150.25},{160,160.1,160.2,160.05,160.15,160.25},
                                            {165,165.1,165.2,165.05,165.15,165.25},{168,168.1,168.2,168.05,168.15,168.25}};

    std::vector<std::string> zgrp2 = {"GRP1", "GRP2"};

    std::vector<int> iwel2 = {1,4,6,8};

    std::vector<std::string> solutionNames2;
    std::vector<std::vector<std::vector<float>>> solutions2;

    solutionNames2={"PRESSURE","RS"};
    solutions2.push_back(pressure2);
    solutions2.push_back(rs2);

// -------------------------

    makeUnrstFile("TMP1.UNRST", seqnum1, dates1, time1, logihead1, doubhead1, zgrp1, iwel1,solutionNames1,solutions1);
    makeUnrstFile("TMP2.UNRST", seqnum2, dates2, time2, logihead2, doubhead2, zgrp2, iwel2,solutionNames2,solutions2);

    ECLRegressionTest test1("TMP1", "TMP2", 1e-3, 1e-3);
    test1.results_rst();

   // different pressure in sequence 4 cell 4
   // changed from 190.05 to 191.05

    pressure2[2][3]=191.05;

   // different pressure in sequence 7, cell 3
   // changed from 185.2 to 185.82

    pressure2[3][2]=185.82;

    solutions2.clear();
    solutions2.push_back(pressure2);
    solutions2.push_back(rs2);

    makeUnrstFile("TMP2.UNRST", seqnum2, dates2, time2, logihead2, doubhead2, zgrp2, iwel2,solutionNames2,solutions2);

    ECLRegressionTest test2("TMP1", "TMP2", 1e-3, 1e-3);
    BOOST_CHECK_THROW(test2.results_rst(),std::runtime_error);

    // check spesific keyword RS, should be OK

    test2.compareSpesificKeyword("RS");
    test2.results_rst();

   // run full analysis, will not throw on first error
    test2.compareSpesificKeyword("");
    test2.doAnalysis(true);
    test2.results_rst();

    // should get deviations for two keywords
    BOOST_CHECK_EQUAL(test2.countDev(),2);

}


BOOST_AUTO_TEST_CASE(results_unsmry_1) {
    WorkArea work;
    std::vector<std::string> keywords1 = {"TIME", "YEARS", "FOPR", "FOPT", "WOPR", "WOPR", "WBHP", "WBHP", "ROIP"};
    std::vector<std::string> wgnames1 = {":+:+:+:+", ":+:+:+:+", "FIELD", "FIELD", "A-1H", "A-2H", "A-1H", "A-2H", ":+:+:+:+"};
    std::vector<int> nums1 = {-32767, -32767, 0, 0, 1, 2, 1, 2, 1};
    std::vector<std::string> units1={"DAYS", "YEARS", "SM3/DAY", "SM3", "SM3/DAY", "SM3/DAY", "BARSA", "BARSA", "SM3"};

    std::vector<std::vector<float>> params1 = {{0,0,0,0,0,0,208.7515,0,56288.06},
                                               {1,0.002737851,25,25,25,0,206.7109,0,56263.06},
                                               {4,0.0109514,25,100,25,0,201.8462,0,56188.06},
                                               {9,0.02464066,25,225,25,0,194.1811,0,56063.06},
                                               {19,0.05201916,25,475,25,0,179.2262,0,55813.06},
                                               {31,0.08487337,25,775,25,0,161.2498,0,55513.06},
                                               {40,0.109514,25,1000,25,0,147.4961,0,55288.06},
                                               {50,0.1368925,25,1250,25,0,130.1973,0,55038.06},
                                               {60,0.164271,25,1500,25,0,112.8681,0,54788.06},
                                               {69,0.1889117,25,1725,25,0,98.51605,0,54563.07}};

    std::vector<std::string> keywords2 = {"TIME", "YEARS", "FOPR", "FOPT", "WOPR", "WOPR", "WBHP", "WBHP", "ROIP"};
    std::vector<std::string> wgnames2 = {":+:+:+:+", ":+:+:+:+", "FIELD", "FIELD", "A-1H", "A-2H", "A-1H", "A-2H", ":+:+:+:+"};
    std::vector<int> nums2 = {-32767, -32767, 0, 0, 1, 2, 1, 2, 1};
    std::vector<std::string> units2={"DAYS", "YEARS", "SM3/DAY", "SM3", "SM3/DAY", "SM3/DAY", "BARSA", "BARSA", "SM3"};

    std::vector<std::vector<float>> params2 = {{0,0,0,0,0,0,208.7515,0,56288.06},
                                               {1,0.002737851,25,25,25,0,206.7109,0,56263.06},
                                               {4,0.0109514,25,100,25,0,201.8462,0,56188.06},
                                               {9,0.02464066,25,225,25,0,194.1811,0,56063.06},
                                               {19,0.05201916,25,475,25,0,179.2262,0,55813.06},
                                               {31,0.08487337,25,775,25,0,161.2498,0,55513.06},
                                               {40,0.109514,25,1000,25,0,147.4961,0,55288.06},
                                               {50,0.1368925,25,1250,25,0,130.1973,0,55038.06},
                                               {60,0.164271,25,1500,25,0,112.8681,0,54788.06},
                                               {69,0.1889117,25,1725,25,0,98.51605,0,54563.07}};

    makeSmryFile("TMP1.SMSPEC", keywords1, wgnames1, nums1, units1, params1);
    makeSmryFile("TMP2.SMSPEC", keywords2, wgnames2, nums2, units2, params2);

    ECLRegressionTest test1("TMP1", "TMP2", 1e-3, 1e-3);

    test1.results_smry();

    // test with option integration test (-i)

    test1.setIntegrationTest(true);
    test1.results_smry();

    // remove vector ROIP from case2, rest is identical to first case

    keywords2 = {"TIME", "YEARS", "FOPR", "FOPT", "WOPR", "WOPR", "WBHP", "WBHP"};
    wgnames2 = {":+:+:+:+", ":+:+:+:+", "FIELD", "FIELD", "A-1H", "A-2H", "A-1H", "A-2H"};
    nums2 = {-32767, -32767, 0, 0, 1, 2, 1, 2};
    units2={"DAYS", "YEARS", "SM3/DAY", "SM3", "SM3/DAY", "SM3/DAY", "BARSA", "BARSA"};

    params2 = {{0,0,0,0,0,0,208.7515,0},
               {1,0.002737851,25,25,25,0,206.7109,0},
               {4,0.0109514,25,100,25,0,201.8462,0},
               {9,0.02464066,25,225,25,0,194.1811,0},
               {19,0.05201916,25,475,25,0,179.2262,0},
               {31,0.08487337,25,775,25,0,161.2498,0},
               {40,0.109514,25,1000,25,0,147.4961,0},
               {50,0.1368925,25,1250,25,0,130.1973,0},
               {60,0.164271,25,1500,25,0,112.8681,0},
               {69,0.1889117,25,1725,25,0,98.51605,0}};


    makeSmryFile("TMP2.SMSPEC", keywords2, wgnames2, nums2, units2, params2);

    ECLRegressionTest test2("TMP1", "TMP2", 1e-3, 1e-3);

    BOOST_CHECK_THROW(test2.results_smry(),std::runtime_error);

    ECLRegressionTest test2a("TMP2", "TMP1", 1e-3, 1e-3);

    BOOST_CHECK_THROW(test2a.results_smry(),std::runtime_error);

    test2a.setAcceptExtraKeywords(true);
    test2a.results_smry();

    test2a.setAcceptExtraKeywords(false);

    // should be ok, since both cases have vector FOPT
    test2a.compareSpesificKeyword("FOPT");
    test2a.results_smry();

    // should fail since vector ROIP only found in first case
    test2a.compareSpesificKeyword("ROIP:1");
    BOOST_CHECK_THROW(test2a.results_smry(),std::runtime_error);

    // should fail since not found in any of the cases
    test2a.compareSpesificKeyword("XXXXX");
    BOOST_CHECK_THROW(test2a.results_smry(),std::runtime_error);
}



BOOST_AUTO_TEST_CASE(results_unsmry_2) {
    WorkArea work;
    std::vector<std::string> keywords1 = {"TIME", "YEARS", "FOPR", "FOPT", "WOPR", "WOPR", "WBHP", "WBHP", "ROIP"};
    std::vector<std::string> wgnames1 = {":+:+:+:+", ":+:+:+:+", "FIELD", "FIELD", "A-1H", "A-2H", "A-1H", "A-2H", ":+:+:+:+"};
    std::vector<int> nums1 = {-32767, -32767, 0, 0, 1, 2, 1, 2, 1};
    std::vector<std::string> units1={"DAYS", "YEARS", "SM3/DAY", "SM3", "SM3/DAY", "SM3/DAY", "BARSA", "BARSA", "SM3"};

    std::vector<std::vector<float>> params1 = {{0,0,0,0,0,0,208.7515,0,56288.06},
                                               {1,0.002737851,25,25,25,0,206.7109,0,56263.06},
                                               {4,0.0109514,25,100,25,0,201.8462,0,56188.06},
                                               {9,0.02464066,25,225,25,0,194.1811,0,56063.06},
                                               {19,0.05201916,25,475,25,0,179.2262,0,55813.06},
                                               {31,0.08487337,25,775,25,0,161.2498,0,55513.06},
                                               {40,0.109514,25,1000,25,0,147.4961,0,55288.06},
                                               {50,0.1368925,25,1250,25,0,130.1973,0,55038.06},
                                               {60,0.164271,25,1500,25,0,112.8681,0,54788.06},
                                               {69,0.1889117,25,1725,25,0,98.51605,0,54563.07}};

    std::vector<std::string> keywords2 = {"TIME", "YEARS", "FOPR", "FOPT", "WOPR", "WOPR", "WBHP", "WBHP", "ROIP"};
    std::vector<std::string> wgnames2 = {":+:+:+:+", ":+:+:+:+", "FIELD", "FIELD", "A-1H", "A-2H", "A-1H", "A-2H", ":+:+:+:+"};
    std::vector<int> nums2 = {-32767, -32767, 0, 0, 1, 2, 1, 2, 1};
    std::vector<std::string> units2={"DAYS", "YEARS", "SM3/DAY", "SM3", "SM3/DAY", "SM3/DAY", "BARSA", "BARSA", "SM3"};

    std::vector<std::vector<float>> params2 = {{0,0,0,0,0,0,208.7515,0,56288.06},
                                               {1,0.002737851,25,25,25,0,206.7109,0,56263.06},
                                               {4,0.0109514,25,100,25,0,201.8462,0,56188.06},
                                               {9,0.02464066,25,225,25,0,194.1811,0,56063.06},
                                               {19,0.05201916,25,475,25,0,179.2262,0,55813.06},
                                               {31,0.08487337,25,775,25,0,161.2498,0,55513.06},
                                               {40,0.109514,25,1000,25,0,147.4961,0,55288.06},
                                               {50,0.1368925,25,1250,25,0,130.1973,0,55038.06},
                                               {60,0.164271,25,1500,25,0,112.8681,0,54788.06},
                                               {69,0.1889117,25,1725,25,0,98.51605,0,54563.07}};

    makeSmryFile("TMP1.SMSPEC", keywords1, wgnames1, nums1, units1, params1);
    makeSmryFile("TMP2.SMSPEC", keywords2, wgnames2, nums2, units2, params2);

    ECLRegressionTest test1("TMP1", "TMP2", 1e-3, 1e-3);

    test1.results_smry();

    // changing wbhp:A-1H for timestep 5, from 161.2498 to 161.3498
    params2[5][6]=161.3498;

    // changing FOPT for timestep 3, from 225 to 223
    params2[3][3]=223;

    makeSmryFile("TMP2.SMSPEC", keywords2, wgnames2, nums2, units2, params2);

    ECLRegressionTest test2("TMP1", "TMP2", 1e-4, 1e-4);

    BOOST_CHECK_THROW(test2.results_smry(),std::runtime_error);

   // run full analysis, will not throw on first error
    test2.doAnalysis(true);

    test2.results_smry();

    // should get deviations for two keywords
    BOOST_CHECK_EQUAL(test2.countDev(),2);
}



BOOST_AUTO_TEST_CASE(results_unsmry_3) {


    ECLRegressionTest test1("SPE1CASE1", "SPE1CASE1A", 1e-3, 1e-3);

    BOOST_CHECK_THROW(test1.results_smry(),std::runtime_error);

    test1.setReportStepOnly(true);

    BOOST_CHECK_NO_THROW(test1.results_smry());

}


BOOST_AUTO_TEST_CASE(results_rft_1) {
    WorkArea work;
    using Date = std::tuple<int, int, int>;

    std::vector<float> time1 = {0.0, 40.0, 50.0};
    std::vector<Date> date1 = {
        Date{2000,1, 1},
        Date{2000,2,10},
        Date{2000,2,20},
    };
    std::vector<std::string> wellN1 = {"A-1H", "A-1H", "A-2H"};

    std::vector<std::vector<int>> conipos1 = {{1,1},{1,1},{2,2}};
    std::vector<std::vector<int>> conjpos1 = {{1,1},{1,1},{3,3}};
    std::vector<std::vector<int>> conkpos1 = {{1,2},{1,2},{1,2}};

    std::vector<std::vector<float>> depth1 = {{2004.68,2009.67},{2004.68,2009.67},{2014.66, 2019.66}};

    std::vector<std::vector<float>> pressure1 = {{208.75, 209.07},{178.045, 178.361},{171.278, 171.594}};
    std::vector<std::vector<float>> swat1 = {{0.0, 0.0},{0.0, 0.0},{0.0, 0.0}};
    std::vector<std::vector<float>> sgas1 = {{0.0, 0.0},{0.0, 0.0},{0.0, 0.0}};
    std::vector<std::string> solutionNames1 = {"PRESSURE", "SWAT", "SGAS"};

    std::vector<std::vector<std::vector<float>>> solutions1;

    solutions1.push_back(pressure1);
    solutions1.push_back(swat1);
    solutions1.push_back(sgas1);

    // ------ second case dentical with first case ---------------

    std::vector<float> time2 = {0.0, 40.0, 50.0};
    std::vector<Date> date2 = {
        Date{2000,1 ,1},
        Date{2000,2,10},
        Date{2000,2,20},
    };
    std::vector<std::string> wellN2 = {"A-1H", "A-1H", "A-2H"};

    std::vector<std::vector<int>> conipos2 = {{1,1},{1,1},{2,2}};
    std::vector<std::vector<int>> conjpos2 = {{1,1},{1,1},{3,3}};
    std::vector<std::vector<int>> conkpos2 = {{1,2},{1,2},{1,2}};

    std::vector<std::vector<float>> depth2 = {{2004.68,2009.67},{2004.68,2009.67},{2014.66, 2019.66}};

    std::vector<std::vector<float>> pressure2 = {{208.75, 209.07},{178.045, 178.361},{171.278, 171.594}};
    std::vector<std::vector<float>> swat2 = {{0.0, 0.0},{0.0, 0.0},{0.0, 0.0}};
    std::vector<std::vector<float>> sgas2 = {{0.0, 0.0},{0.0, 0.0},{0.0, 0.0}};
    std::vector<std::string> solutionNames2 = {"PRESSURE", "SWAT", "SGAS"};

    std::vector<std::vector<std::vector<float>>> solutions2;

    solutions2.push_back(pressure2);
    solutions2.push_back(swat2);
    solutions2.push_back(sgas2);

    // -----------------third case, only two rfts, second rft in A-1H removed

    std::vector<float> time3 = {0.0, 50.0};
    std::vector<Date> date3 = {
        Date{2000,1,1}, Date{2000,2,20},
    };
    std::vector<std::string> wellN3 = {"A-1H", "A-2H"};

    std::vector<std::vector<int>> conipos3 = {{1,1},{2,2}};
    std::vector<std::vector<int>> conjpos3 = {{1,1},{3,3}};
    std::vector<std::vector<int>> conkpos3 = {{1,2},{1,2}};

    std::vector<std::vector<float>> depth3 = {{2004.68,2009.67},{2014.66, 2019.66}};

    std::vector<std::vector<float>> pressure3 = {{208.75, 209.07},{171.278, 171.594}};
    std::vector<std::vector<float>> swat3 = {{0.0, 0.0},{0.0, 0.0}};
    std::vector<std::vector<float>> sgas3 = {{0.0, 0.0},{0.0, 0.0}};
    std::vector<std::string> solutionNames3 = {"PRESSURE", "SWAT", "SGAS"};

    std::vector<std::vector<std::vector<float>>> solutions3;

    solutions3.push_back(pressure3);
    solutions3.push_back(swat3);
    solutions3.push_back(sgas3);

    // -----------------------------------

    makeRftFile("TMP1.RFT", time1, date1, wellN1, conipos1, conjpos1, conkpos1, depth1, solutionNames1, solutions1);
    makeRftFile("TMP2.RFT", time2, date2, wellN2, conipos2, conjpos2, conkpos2, depth2, solutionNames2, solutions2);

    ECLRegressionTest test1("TMP1", "TMP2", 1e-3, 1e-3);

    test1.results_rft();

    // remove SGAS in second case TMP2, should fail

    solutionNames2 = {"PRESSURE", "SWAT"};

    solutions2.clear();

    solutions2.push_back(pressure2);
    solutions2.push_back(swat2);

    makeRftFile("TMP2.RFT", time2, date2, wellN2, conipos2, conjpos2, conkpos2, depth2, solutionNames2, solutions2);

    ECLRegressionTest test1a("TMP1", "TMP2", 1e-3, 1e-3);

    BOOST_CHECK_THROW(test1a.results_rft(),std::runtime_error);

    // use TMP2 as first case, SGAS then one extra solution in TMP1
    // should still fail

    ECLRegressionTest test1b("TMP2", "TMP1", 1e-3, 1e-3);
    BOOST_CHECK_THROW(test1b.results_rft(),std::runtime_error);

    // accept extra keyword, test should now be ok
    test1b.setAcceptExtraKeywords(true);

    test1b.results_rft();

    // accept extra keyword to false, but check for spesific keyword (PRESSSURE)

    test1b.setAcceptExtraKeywords(false);
    test1b.compareSpesificKeyword("PRESSURE");

    // should be ok since both cases have solution PRESSURE, only solution checked
    test1b.results_rft();

    // SGAS, only present in second case, should fail
    test1b.compareSpesificKeyword("SGAS");

    BOOST_CHECK_THROW(test1b.results_rft(),std::runtime_error);

    // testing third case, missing one rft and should fail
    makeRftFile("TMP3.RFT", time3, date3, wellN3, conipos3, conjpos3, conkpos3, depth3, solutionNames3, solutions3);
    ECLRegressionTest test2("TMP1", "TMP3", 1e-3, 1e-3);

    BOOST_CHECK_THROW(test2.results_rft(),std::runtime_error);
}

BOOST_AUTO_TEST_CASE(results_rft_2) {
    WorkArea work;
    using Date = std::tuple<int, int, int>;

    std::vector<float> time1 = {0.0, 40.0, 50.0};
    std::vector<Date> date1 = {
        Date{2000,1,1}, Date{2000,2,10}, Date{2000,2,20},
    };
    std::vector<std::string> wellN1 = {"A-1H", "A-1H", "A-2H"};

    std::vector<std::vector<int>> conipos1 = {{1,1},{1,1},{2,2}};
    std::vector<std::vector<int>> conjpos1 = {{1,1},{1,1},{3,3}};
    std::vector<std::vector<int>> conkpos1 = {{1,2},{1,2},{1,2}};

    std::vector<std::vector<float>> depth1 = {{2004.68,2009.67},{2004.68,2009.67},{2014.66, 2019.66}};

    std::vector<std::vector<float>> pressure1 = {{208.75, 209.07},{178.045, 178.361},{171.278, 171.594}};
    std::vector<std::vector<float>> swat1 = {{0.0, 0.0},{0.0, 0.0},{0.0, 0.0}};
    std::vector<std::vector<float>> sgas1 = {{0.0, 0.0},{0.0, 0.0},{0.0, 0.0}};
    std::vector<std::string> solutionNames1 = {"PRESSURE", "SWAT", "SGAS"};

    std::vector<std::vector<std::vector<float>>> solutions1;

    solutions1.push_back(pressure1);
    solutions1.push_back(swat1);
    solutions1.push_back(sgas1);

    // ------ second case dentical with first case ---------------

    std::vector<float> time2 = {0.0, 40.0, 50.0};
    std::vector<Date> date2 = {
        Date{2000,1,1}, Date{2000,2,10}, Date{2000,2,20},
    };
    std::vector<std::string> wellN2 = {"A-1H", "A-1H", "A-2H"};

    std::vector<std::vector<int>> conipos2 = {{1,1},{1,1},{2,2}};
    std::vector<std::vector<int>> conjpos2 = {{1,1},{1,1},{3,3}};
    std::vector<std::vector<int>> conkpos2 = {{1,2},{1,2},{1,2}};

    std::vector<std::vector<float>> depth2 = {{2004.68,2009.67},{2004.68,2009.67},{2014.66, 2019.66}};

    std::vector<std::vector<float>> pressure2 = {{208.75, 209.07},{178.045, 178.361},{171.278, 171.594}};
    std::vector<std::vector<float>> swat2 = {{0.0, 0.0},{0.0, 0.0},{0.0, 0.0}};
    std::vector<std::vector<float>> sgas2 = {{0.0, 0.0},{0.0, 0.0},{0.0, 0.0}};
    std::vector<std::string> solutionNames2 = {"PRESSURE", "SWAT", "SGAS"};

    std::vector<std::vector<std::vector<float>>> solutions2;

    solutions2.push_back(pressure2);
    solutions2.push_back(swat2);
    solutions2.push_back(sgas2);

    // -----------------------------------

    makeRftFile("TMP1.RFT", time1, date1, wellN1, conipos1, conjpos1, conkpos1, depth1, solutionNames1, solutions1);
    makeRftFile("TMP2.RFT", time2, date2, wellN2, conipos2, conjpos2, conkpos2, depth2, solutionNames2, solutions2);

    ECLRegressionTest test1("TMP1", "TMP2", 1e-3, 1e-3);

    //  solutions1 and solutions2 are identical so far. Test should be ok.
    test1.results_rft();

    // changing pressure solutions in second case, rft #2

    // modify pressure within tolerances
    pressure2[1][1]=pressure2[1][1]*(1+1.0e-5);

    solutions2.clear();
    solutions2.push_back(pressure2);
    solutions2.push_back(swat2);
    solutions2.push_back(sgas2);

    makeRftFile("TMP2.RFT", time2, date2, wellN2, conipos2, conjpos2, conkpos2, depth2, solutionNames2, solutions2);

    // test should be ok, changes within tolerances
    ECLRegressionTest test2("TMP1", "TMP2", 1e-3, 1e-3);
    test2.results_rft();

    // modify pressure for second rft to be outside tolerances
    pressure2[1][1]=pressure2[1][1]*(1+1.0e-3);

    // Also modify modify gas saturation for third rft to be outside tolerances
    sgas2[2][1] = 0.01;

    solutions2.clear();
    solutions2.push_back(pressure2);
    solutions2.push_back(swat2);
    solutions2.push_back(sgas2);

    makeRftFile("TMP2.RFT", time2, date2, wellN2, conipos2, conjpos2, conkpos2, depth2, solutionNames2, solutions2);

    // test should fail
    ECLRegressionTest test3("TMP1", "TMP2", 1e-3, 1e-3);
    BOOST_CHECK_THROW(test3.results_rft(),std::runtime_error);

    // run full analysis, will not throw on first error
    test3.doAnalysis(true);
    test3.results_rft();

    // should get deviations for two keywords
    BOOST_CHECK_EQUAL(test3.countDev(), 2);
}

