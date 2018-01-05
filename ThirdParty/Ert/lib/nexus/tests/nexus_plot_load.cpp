/*
   Copyright (C) 2017 Statoil ASA, Norway.

   The file 'nexus_plot_constructor.cpp' is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>


#include <ert/util/test_util.hpp>
#include <ert/nexus/nexus_plot.hpp>

void test_invalid_header1() {
    std::stringstream stream( "xxxxINVALID_HEADER" );
    test_assert_throw(nex::NexusPlot { stream }, nex::bad_header);
}

void test_invalid_header2() {
    std::stringstream stream( "xxx" );
    test_assert_throw(nex::NexusPlot { stream }, nex::bad_header);
}

void test_valid_header() {
    std::stringstream stream( "xxxxPLOT  BIN " );
    test_assert_throw(nex::NexusPlot { stream }, nex::unexpected_eof);
}

void test_spe1_header(char *argv) {
    std::stringstream ss;
    ss << argv << "/test-data/local/nexus/SPE1.plt";
    auto plt = nex::NexusPlot { ss.str() };

    std::array< std::string, 9 > class_names = {
        "WELL    ", "WLLYR   ", "NODE    ", "CONN    ", "REGION  ", "FIELD   ",
        "CONNLIST", "TARGET  ", "FLOSTA  "
    };
    std::array< int, 9 > vars_in_class = {
        56, 52, 4, 43, 69, 58, 20, 25, 25
    };

    std::array< std::vector< std::string >, 9 > var_names = {
        std::vector< std::string > {
            "COP ", "CGP ", "CWP ", "CGI ", "CWI ", "QOP ", "QGP ", "QWP ",
            "QGI ", "QWI ", "BHP ", "WPH ", "WKH ", "WPAV", "THP ", "COWP",
            "QOWP", "GOR ", "WCUT", "WOR ", "QGLG", "CGLG", "DRDN", "DRMX",
            "CROP", "CRGP", "CRWP", "CROI", "CRGI", "CRWI", "ROP ", "RGP ",
            "RWP ", "ROI ", "RGI ", "RWI ", "ONTM", "ALQ ", "API ", "QCDP",
            "CCDP", "YCDP", "ACTV", "STAT", "Q1P ", "Q1I ", "C1P ", "C1I ",
            "X1P ", "Y1P ", "Q2P ", "Q2I ", "C2P ", "C2I ", "X2P ", "Y2P " },
        std::vector< std::string > {
            "QOP ", "QGP ", "QWP ", "COP ", "CGP ", "CWP ", "CGI ", "CWI ",
            "PRES", "HEAD", "RW  ", "SKIN", "WI  ", "WBC ", "PWB ", "QGI ",
            "QWI ", "COWP", "QOWP", "GOR ", "WCUT", "ACTV", "CROP", "CRGP",
            "CRWP", "CROI", "CRGI", "CRWI", "ROP ", "RGP ", "RWP ", "ROI ",
            "RGI ", "RWI ", "QCDP", "CCDP", "YCDP", "API ", "MD  ", "FSEC",
            "Q1P ", "Q1I ", "C1P ", "C1I ", "X1P ", "Y1P ", "Q2P ", "Q2I ",
            "C2P ", "C2I ", "X2P ", "Y2P " },
        std::vector< std::string > { "PNOD", "PDAT", "TNOD", "ACTV" },
        std::vector< std::string > {
            "QGAS", "QOIL", "QWTR", "CGAS", "COIL", "CWTR", "CBFG", "CBFO",
            "CBFW", "QGIS", "QOIS", "QWIS", "P_IN", "POUT", "T_IN", "TOUT",
            "ACTV", "STAT", "CSTR", "ITRG", "ONTM", "ALQ ", "SETM", "SETA",
            "POWM", "POWA", "SPDM", "SPDA", "API ", "DELP", "QTOT", "GVF ",
            "EFF ", "POSN", "WCUT", "GOR ", "WOR ", "Q1  ", "Q2  ", "X1  ",
            "X2  ", "Y1  ", "Y2  " },
        std::vector< std::string > {
            "COP ", "CGP ", "CWP ", "COI ", "CGI ", "CWI ", "PAVT", "PAVH",
            "OIP ", "GIP ", "WIP ", "QOP ", "QGP ", "QWP ", "QOI ", "QGI ",
            "QWI ", "OIN ", "GIN ", "WIN ", "SO  ", "SG  ", "SW  ", "OREC",
            "FGIP", "CIP ", "PAVE", "PAVD", "ROIP", "RGIP", "RWIP", "MRO ",
            "MRG ", "MRW ", "NFLX", "PV  ", "HCPV", "TAVT", "TAVH", "CROP",
            "CRGP", "CRWP", "CROI", "CRGI", "CRWI", "ROP ", "RGP ", "RWP ",
            "ROI ", "RGI ", "RWI ", "QCDP", "CCDP", "YCDP", "API ", "GOR ",
            "WCUT", "WOR ", "Z1  ", "Z2  ", "MC1 ", "MC2 ", "MC3 ", "C1P ",
            "C2P ", "C3P ", "C1I ", "C2I ", "C3I " },
        std::vector< std::string > {
            "COP ", "CGP ", "CWP ", "CGI ", "CWI ", "QOP ", "QGP ", "QWP ",
            "QGI ", "QWI ", "COWP", "QOWP", "GOR ", "OREC", "GREC", "PAVT",
            "PAVH", "QGLG", "CGLG", "WCUT", "NFLX", "CROP", "CRGP", "CRWP",
            "CROI", "CRGI", "CRWI", "ROP ", "RGP ", "RWP ", "ROI ", "RGI ",
            "RWI ", "OIP ", "GIP ", "WIP ", "QCDP", "CCDP", "YCDP", "WLLS",
            "PRDW", "GLFW", "WINJ", "GINJ", "ACTW", "API ", "Q1P ", "Q1I ",
            "C1P ", "C1I ", "X1P ", "Y1P ", "Q2P ", "Q2I ", "C2P ", "C2I ",
            "X2P ", "Y2P " },
        std::vector< std::string > {
            "QOP ", "QGP ", "QWP ", "QOI ", "QGI ", "QWI ", "COP ", "CGP ",
            "CWP ", "COI ", "CGI ", "CWI ", "API ", "WCUT", "GOR ", "WOR ",
            "Q1P ", "Q1I ", "Q2P ", "Q2I " },
        std::vector< std::string > {
            "SQO ", "SQG ", "SQW ", "SQL ", "SQA ", "SQH ", "RQO ", "RQG ",
            "RQW ", "RQL ", "RQA ", "RQH ", "TSQO", "TSQG", "TSQW", "TSQL",
            "TSQA", "TSQH", "TRQO", "TRQG", "TRQW", "TRQL", "TRQA", "TRQH",
            "P   " },
        std::vector< std::string > {
            "QOP ", "QGP ", "QWP ", "QOI ", "QGI ", "QWI ", "COP ", "CGP ",
            "CWP ", "COI ", "CGI ", "CWI ", "WLLS", "PRDW", "GLFW", "WINJ",
            "GINJ", "ACTW", "WCUT", "GOR ", "WOR ", "Q1P ", "Q1I ", "Q2P ",
            "Q2I " }
    };

    test_assert_int_equal(plt.num_classes, 9);
    test_assert_int_equal(plt.day,         1);
    test_assert_int_equal(plt.month,       1);
    test_assert_int_equal(plt.year,        1980);
    test_assert_int_equal(plt.nx,          1);
    test_assert_int_equal(plt.ny,          1);
    test_assert_int_equal(plt.nz,          1);
    test_assert_int_equal(plt.ncomp,       2);
    for (int i = 0; i < plt.num_classes; i++)
        test_assert_std_string_equal(class_names[i], plt.class_names[i]);
    for (int i = 0; i < plt.num_classes; i++)
        test_assert_int_equal(vars_in_class[i], plt.vars_in_class[i]);
    for (int i = 0; i < plt.num_classes; ++i) {
        for (int k = 0; k < plt.vars_in_class[i]; ++k) {
            test_assert_std_string_equal(var_names[i][k], plt.var_names[i][k]);
        }
    }
}

int main(int argc, char* argv[]) {
    test_invalid_header1();
    test_invalid_header2();
    test_valid_header();
    test_spe1_header(argv[1]);
    return 0;
}
