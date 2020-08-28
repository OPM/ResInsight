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

#include "EclRegressionTest.hpp"

#include <opm/io/eclipse/EGrid.hpp>
#include <opm/io/eclipse/ERft.hpp>
#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ERsm.hpp>

#include <opm/common/utility/FileSystem.hpp>
#include <opm/common/ErrorMacros.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <set>
#include <type_traits>
#include <typeinfo>
#include <vector>

// helper macro to handle error throws or not
#define HANDLE_ERROR(type, message) \
  { \
    if (throwOnError) \
      OPM_THROW(type, message); \
    else { \
      std::cerr << message << std::endl; \
      ++num_errors; \
    } \
  }

namespace {

template <typename T>
std::vector<T> sorted(std::vector<T> v) {
    std::sort(v.begin(), v.end());
    return v;
}

}


using namespace Opm::EclIO;

ECLRegressionTest::~ECLRegressionTest()
{
    delete grid1;
    delete grid2;
}


static bool fileExists(const std::string& fileName) {
    std::ifstream f(fileName);
    return f.good();
}


bool ECLRegressionTest::checkFileName(const std::string &rootName, const std::string &extension, std::string &filename) {

    if (fileExists(rootName + "." + extension)) {
        filename = rootName + "." + extension;
        return true;
    } else if (fileExists(rootName + ".F" + extension)) {
        filename = rootName + ".F" + extension;
        return true;
    } else {
        filename.clear();
        return false;
    }
}


template <typename T>
bool operator==(const std::vector<T> & t1, const std::vector<T> & t2)
{
    return std::equal(t1.begin(), t1.end(), t2.begin(), t2.end());
}


template <typename T>
void ECLRegressionTest::compareFloatingPointVectors(const std::vector<T>& t1, const std::vector<T>& t2, const std::string& keyword, const std::string& reference) {

    if (typeid(T) != typeid(float) && typeid(T) != typeid(double)) {
        HANDLE_ERROR(std::runtime_error, "\nMember function compareVectors should be used with floating point vectors ");
    }

    if (t1.size() != t2.size()) {
        HANDLE_ERROR(std::runtime_error, "\nError trying to compare two vectors with different size " << keyword << " - " << reference
                     << "\n > size of first vector : " << t1.size() << "\n > size of second vector: " << t2.size());
    }

    auto it = std::find(keywordDisallowNegatives.begin(), keywordDisallowNegatives.end(), keyword);
    bool allowNegatives = it == keywordDisallowNegatives.end() ? true : false;

    it = std::find(keywordsStrictTol.begin(), keywordsStrictTol.end(), keyword);
    bool strictTol = it != keywordsStrictTol.end() ? true : false;

    for (size_t i = 0; i < t1.size(); i++) {
        deviationsForCell(static_cast<double>(t1[i]),
                          static_cast<double>(t2[i]),
                          keyword, reference, t1.size(),
                          i, allowNegatives, strictTol);
    }
}


template <typename T>
void ECLRegressionTest::compareVectors(const std::vector<T>& t1, const std::vector<T>& t2, const std::string& keyword, const std::string& reference) {

    if (t1.size() != t2.size()) {
        HANDLE_ERROR(std::runtime_error, "\nError trying to compare two vectors with different size " << keyword << " - " << reference
                     << "\n > size of first vector : " << t1.size() << "\n > size of second vector: " << t2.size());
    }

    if (typeid(T) == typeid(float) || typeid(T) == typeid(double)) {
        HANDLE_ERROR(std::runtime_error, "\nMember function compareVectors should not be used with floating point vectors ");
    }

    bool result = t1 == t2 ? true : false ;

    if (!result) {
        for (size_t i = 0; i < t1.size(); i++) {
            deviationsForNonFloatingPoints(t1[i], t2[i], keyword, reference, t1.size(), i);
        }
    }
}


template <typename T>
void ECLRegressionTest::deviationsForNonFloatingPoints(T val1, T val2, const std::string& keyword, const std::string& reference, size_t kw_size, size_t cell)
{
    if (val1 != val2) {
        printValuesForCell(keyword, reference, kw_size, cell, grid1, val1, val2);
        HANDLE_ERROR(std::runtime_error, "Non floating point values not identical ");
    }
}


void ECLRegressionTest::deviationsForCell(double val1, double val2, const std::string& keyword, const std::string& reference, size_t kw_size, size_t cell, bool allowNegativeValues, bool useStrictTol)
{
    double absToleranceLoc = useStrictTol ? strictAbsTol : getAbsTolerance();
    double relToleranceLoc = useStrictTol ? strictAbsTol : getRelTolerance();

    if (!allowNegativeValues) {
        if (val1 < 0) {
            if (std::abs(val1) > absToleranceLoc) {
                printValuesForCell(keyword, reference, kw_size, cell, grid1, val1, val2);
                HANDLE_ERROR(std::runtime_error, "Negative value in first file, "
                             << "which in absolute value exceeds the absolute tolerance of " << absToleranceLoc << ".");
            }
            val1 = 0;
        }

        if (val2 < 0) {
            if (std::abs(val2) > absToleranceLoc) {
                printValuesForCell(keyword, reference, kw_size, cell, grid1, val1, val2);
                HANDLE_ERROR(std::runtime_error, "Negative value in second file, "
                             << "which in absolute value exceeds the absolute tolerance of " << absToleranceLoc << ".");
            }
            val2 = 0;
        }
    }

    Deviation dev = calculateDeviations(val1, val2);
    if (dev.abs > absToleranceLoc && (dev.rel > relToleranceLoc || dev.rel == -1)) {
        if (analysis) {
            std::string keywref = keyword + ": " + reference;
            deviations[keywref].push_back(dev);
        } else {
            printValuesForCell(keyword, reference, kw_size, cell, grid1, val1, val2);

            if (useStrictTol) {
                std::cout << "Keyword: " << keyword << " requires strict tolerances.\n" << std::endl;
            }

            HANDLE_ERROR(std::runtime_error, "Deviations exceed tolerances."
                         << "\nThe absolute deviation is " << dev.abs << ", and the tolerance limit is " << absToleranceLoc << "."
                         << "\nThe relative deviation is " << dev.rel << ", and the tolerance limit is " << relToleranceLoc << ".");
        }
    }

    if (dev.abs != -1) {
        absDeviation.push_back(dev.abs);
    }

    if (dev.rel != -1) {
        relDeviation.push_back(dev.rel);
    }
}


void ECLRegressionTest::printDeviationReport()
{
    if (analysis) {
        std::cout << " \n" << deviations.size() << " keyword"
                  << (deviations.size() > 1 ? "s":"") << " exhibit failures" << std::endl;
        for (const auto& iter : deviations) {
            std::cout << "\t" << iter.first << std::endl;
            std::cout << "\t\tFails for " << iter.second.size() << " entries" << std::endl;
            std::cout.precision(7);
            double absErr = std::max_element(iter.second.begin(), iter.second.end(),
                                             [](const Deviation& a, const Deviation& b)
            {
                return a.abs < b.abs;
            })->abs;
            double relErr = std::max_element(iter.second.begin(), iter.second.end(),
                                             [](const Deviation& a, const Deviation& b)
            {
                return a.rel < b.rel;
            })->rel;
            std::cout << "\t\tLargest absolute error: "
                      <<  std::scientific << absErr << std::endl;
            std::cout << "\t\tLargest relative error: "
                      <<  std::scientific << relErr << std::endl;
        }
    }
}


void ECLRegressionTest::compareKeywords(const std::vector<std::string> &keywords1, const std::vector<std::string> &keywords2, const std::string &reference) {

    if (!acceptExtraKeywords) {
        if (keywords1 != keywords2) {
            std::cout << "not same keywords in " << reference << std::endl;

            if (keywords1.size() > 50) {
                printMissingKeywords(keywords1,keywords2);
            } else {
                printComparisonForKeywordLists(keywords1,keywords2);
            }
            OPM_THROW(std::runtime_error, "\nKeywords not identical in " << reference);
        }
    } else {
        for (auto& keyword : keywords1) {
            auto it1 = std::find(keywords2.begin(), keywords2.end(), keyword);
            if (it1 == keywords2.end()) {
                std::cout << "Keyword " << keyword << " missing in second file " << std::endl;

                if (keywords1.size() > 50) {
                    printMissingKeywords(keywords1, keywords2);
                } else {
                    printComparisonForKeywordLists(keywords1, keywords2);
                }

                OPM_THROW(std::runtime_error, "\nKeyword " << keyword << " missing in second file ");
            }
        }

        if (keywords2.size() > keywords1.size()) {
            std::cout << "\nExtra keywords ("
                      << std::to_string(keywords2.size() - keywords1.size())
                      << ") accepted in second file " << std::endl;
        }
    }
}


void ECLRegressionTest::checkSpesificKeyword(std::vector<std::string>& keywords1,
                                             std::vector<std::string>& keywords2,
                                             std::vector<eclArrType>& arrayType1,
                                             std::vector<eclArrType>& arrayType2,
                                             const std::string& reference)
{
    auto search1 = std::find(keywords1.begin(), keywords1.end(), spesificKeyword);
    auto search2 = std::find(keywords2.begin(), keywords2.end(), spesificKeyword);

    if (search1 == keywords1.end() && search2 == keywords2.end()) {
        std::cout << "Testing specific keyword \"" << spesificKeyword << "\" in " << reference << ". Keyword not found in any of the cases ." << std::endl;
        OPM_THROW(std::runtime_error, "\nTesting spesific keyword \"" << spesificKeyword << "\" in " << reference << ". Keyword not found in any of the cases .");
    }

    eclArrType arrType;
    if (search1 != keywords1.end()) {
        int ind = std::distance(keywords1.begin(), search1);
        arrType = arrayType1[ind];

        if (search2 == keywords2.end()) {
            std::cout << "Testing spesific kewyword in " << reference << ". Keyword found in fist case but not in second case." << std::endl;
            OPM_THROW(std::runtime_error, "\nTesting spesific kewyword in " << reference << ". Keyword found in fist case but not in second case.");
        }

        keywords1.clear();
        arrayType1.clear();
        keywords1.push_back(spesificKeyword);
        arrayType1.push_back(arrType);

        keywords2.clear();
        arrayType2.clear();
        keywords2.push_back(spesificKeyword);
        arrayType2.push_back(arrType);
    } else {
        if (search2 != keywords2.end()) {
            std::cout << "Testing spesific kewyword in " << reference << ". Keyword not found in fist case but found in second case." << std::endl;
            OPM_THROW(std::runtime_error, "\nTesting spesific kewyword in " << reference << ". Keyword not found in fist case but found in second case.");
        }

        keywords1.clear();
        arrayType1.clear();

        keywords2.clear();
        arrayType2.clear();
    }
}


void ECLRegressionTest::loadGrids()
{
    bool foundEGrid1, foundEGrid2;
    std::string fileName1, fileName2;

    foundEGrid1 = checkFileName(rootName1, "EGRID", fileName1);
    foundEGrid2 = checkFileName(rootName2, "EGRID", fileName2);

    if (foundEGrid1) {
        std::cout << "\nLoading EGrid " << fileName1 << "  .... ";
        grid1 = new EGrid(fileName1);
        std::cout << " done." << std::endl;
    }

    if (foundEGrid2) {
        std::cout << "Loading EGrid " << fileName2 << "  .... ";
        grid2 = new EGrid(fileName2);
        std::cout << " done." << std::endl;
    }

    if ((not foundEGrid1) || (not foundEGrid2)) {
        std::cout << "\nWarning! Both grids could not be loaded. Not possible to reference cell values to grid indices." << std::endl;
        std::cout << "Grid compare may also fail. SMRY, RFT, UNRST and INIT files can be checked \n" << std::endl;
    }
}


void ECLRegressionTest::gridCompare()
{
    deviations.clear();

    if ((grid1) && (not grid2)){
        std::string message ="test case egrid file " + rootName2 + ".EGRID could not be loaded";
	std::cout << message << std::endl;
        OPM_THROW(std::runtime_error, message);
    }

    if (grid1 && grid2) {

        std::cout << "comparing grids " << std::endl;

        const auto& dim1 = grid1->dimension();
        const auto& dim2 = grid2->dimension();

        if (printKeywordOnly) {

            auto arrayList1 = grid1->getList();
            auto arrayList2 = grid2->getList();

            std::vector<std::string> keywords1;
            std::vector<eclArrType> arrayType1;

            for (auto& array : arrayList1) {
                keywords1.push_back(std::get<0>(array));
                arrayType1.push_back(std::get<1>(array));
            }

            std::vector<std::string> keywords2;
            std::vector<eclArrType> arrayType2;

            for (auto& array : arrayList2) {
                keywords2.push_back(std::get<0>(array));
                arrayType2.push_back(std::get<1>(array));
            }

            printComparisonForKeywordLists(keywords1, keywords2, arrayType1, arrayType2);
            return;
        }

        std::cout << "\nComparing egrid files \n" << std::endl;

        std::cout << "Dimensions             " << " ... ";

        if (dim1[0] != dim2[0]  || dim1[1] != dim2[1] || dim1[2] != dim2[2]) {
            OPM_THROW(std::runtime_error, "\n Grid1 and grid2 have different dimensions.  "
                      << "\n grid1 : "  << dim1[0] << "x" << dim1[1] << "x"<< dim1[2]
                      << "\n grid2 : "  << dim2[0] << "x" << dim2[1] << "x"<< dim2[2]);
        }

        std::cout << " done." << std::endl;

        std::cout << "Active cells           " << " ... ";

        for (int k = 0; k < dim1[2]; k++) {
            for (int j=0; j < dim1[1]; j++) {
                for (int i = 0; i < dim2[0]; i++) {
                    if (grid1->active_index(i,j,k) != grid2->active_index(i,j,k)) {
                        OPM_THROW(std::runtime_error, "\nGrid1 and grid2 have different definition of active cells. "
                                  " First difference found for cell i="<< i+1 << " j=" << j+1 << " k=" << k+1);
                    }
                }
            }
        }

        std::cout << " done." << std::endl;

        std::cout << "X, Y and Z coordinates " << " ... ";

        std::array<double,8> X1 = {0.0};
        std::array<double,8> Y1 = {0.0};
        std::array<double,8> Z1 = {0.0};

        std::array<double,8> X2 = {0.0};
        std::array<double,8> Y2 = {0.0};
        std::array<double,8> Z2 = {0.0};

        for (int k = 0; k < dim1[2]; k++) {
            for (int j = 0; j < dim1[1]; j++) {
                for (int i = 0; i < dim1[0]; i++) {
                    if (grid1->active_index(i,j,k) > -1) {
                        grid1->getCellCorners({i,j,k}, X1, Y1, Z1);
                        grid2->getCellCorners({i,j,k}, X2, Y2, Z2);

                        for (int n = 0; n < 8; n++) {
                            Deviation devX = calculateDeviations(X1[n], X2[n]);
                            Deviation devY = calculateDeviations(Y1[n], Y2[n]);
                            Deviation devZ = calculateDeviations(Z1[n], Z2[n]);

                            if (devX.abs > strictAbsTol) {
                                if (analysis) {
                                    deviations["xcoordinate"].push_back(devX);
                                } else {
                                    OPM_THROW(std::runtime_error, "\nGrid1 and grid2 have different X, Y and/or Z coordinates . "
                                              " First difference found for cell i="<< i+1 << " j=" << j+1 << " k=" << k+1);
                                }
                            }

                            if (devY.abs > strictAbsTol) {
                                if (analysis) {
                                    deviations["ycoordinate"].push_back(devY);
                                } else {
                                    OPM_THROW(std::runtime_error, "\nGrid1 and grid2 have different X, Y and/or Z coordinates . "
                                              " First difference found for cell i="<< i+1 << " j=" << j+1 << " k=" << k+1);
                                }
                            }

                            if (devZ.abs > strictAbsTol) {
                                if (analysis) {
                                    deviations["zcoordinate"].push_back(devZ);
                                } else {
                                    OPM_THROW(std::runtime_error, "\nGrid1 and grid2 have different X, Y and/or Z coordinates . "
                                              " First difference found for cell i="<< i+1 << " j=" << j+1 << " k=" << k+1);
                                }
                            }
                        }
                    }
                }
            }
        }

        std::cout << " done." << std::endl;

        std::cout << "NNC indices            " << " ... ";

        // check / compare NNC definitions

        if (grid1->hasKey("NNC1")) {
            std::vector<int> NNC11 = grid1->get<int>("NNC1");
            std::vector<int> NNC21 = grid1->get<int>("NNC2");

            if (!grid2->hasKey("NNC1")) {
                OPM_THROW(std::runtime_error, "\nFirst Grid have NNC1 keyword but not second grid  ");
            }

            std::vector<int> NNC12 = grid2->get<int>("NNC1");
            std::vector<int> NNC22 = grid2->get<int>("NNC2");

            if (NNC11.size() != NNC12.size() || NNC21.size() != NNC22.size()) {
                OPM_THROW(std::runtime_error, "\n Grid1 and grid2 have different number of NNCs. "
                          << " \n Grid1:  " << NNC11.size() << ",  Grid2:  " << NNC12.size() );
            }

            for (size_t n = 0; n < NNC11.size(); n++) {
                if (NNC11[n] != NNC12[n] || NNC21[n] != NNC22[n]) {
                    std::cout << "Differences in NNCs. First found for " << NNC11[n] << " -> " <<  NNC21[n];
                    std::cout << " not same as " << NNC12[n] << " -> " <<  NNC22[n] << std::endl;

                    auto ijk1 = grid1->ijk_from_global_index(NNC11[n]-1);
                    auto ijk2  = grid1->ijk_from_global_index(NNC21[n]-1);

                    std::cout << "In grid1 " << ijk1[0]+1 << "," << ijk1[1]+1 <<"," << ijk1[2]+1  << " -> " << ijk2[0]+1 << "," << ijk2[1]+1 <<"," << ijk2[2]+1 << std::endl;

                    ijk1 = grid2->ijk_from_global_index(NNC12[n]-1);
                    ijk2 = grid2->ijk_from_global_index(NNC22[n]-1);

                    std::cout << "In grid2 " << ijk1[0]+1 << "," << ijk1[1]+1 <<"," << ijk1[2]+1  << " -> " << ijk2[0]+1 << "," << ijk2[1]+1 <<"," << ijk2[2]+1 << std::endl;

                    OPM_THROW(std::runtime_error, "\n Grid1 and grid2 have different definitions of NNCs. ");
                }
            }
        }

        std::cout << " done." << std::endl;

        if (!deviations.empty()) {
            printDeviationReport();
        }

    } else {
        std::cout << "\n!Warning, grid files not found, hence not compared. \n" << std::endl;
    }

}


void ECLRegressionTest::results_init()
{
    std::string fileName1, fileName2;

    bool foundInit1 = checkFileName(rootName1, "INIT", fileName1);
    bool foundInit2 = checkFileName(rootName2, "INIT", fileName2);

    if ((foundInit1) && (not foundInit2)){
        std::string message ="test case init file " + rootName2 + ".INIT not found";
	std::cout << message << std::endl;
        OPM_THROW(std::runtime_error, message);
    }

    if (foundInit1 && foundInit2) {
        EclFile init1(fileName1);
        std::cout << "\nLoading INIT file " << fileName1 << "  .... done" << std::endl;

        EclFile init2(fileName2);
        std::cout << "Loading INIT file " << fileName2 << "  .... done\n" << std::endl;

        deviations.clear();

        init1.loadData();
        init2.loadData();

        std::string reference = "Init file";

        auto arrayList1 = init1.getList();
        auto arrayList2 = init2.getList();

        std::vector<std::string> keywords1;
        std::vector<eclArrType> arrayType1;

        for (const auto& array : arrayList1) {
            keywords1.push_back(std::get<0>(array));
            arrayType1.push_back(std::get<1>(array));
        }

        std::vector<std::string> keywords2;
        std::vector<eclArrType> arrayType2;

        for (const auto& array : arrayList2) {
            keywords2.push_back(std::get<0>(array));
            arrayType2.push_back(std::get<1>(array));
        }

        if (printKeywordOnly) {
            printComparisonForKeywordLists(keywords1,keywords2, arrayType1, arrayType2);
        } else {
            std::cout << "\nComparing init files \n" << std::endl;

            if (spesificKeyword.empty()) {
                if (keywords1.size() == keywords2.size() && keywords1 != keywords2) {
                    /*
                      If the keywords come in different order in the two files
                      we mark that as a test failure, but all comparisons are
                      still performed, and the keyword reordering can be
                      identified by the error message.
                    */
                    auto kw1 = sorted(keywords1);
                    auto kw2 = sorted(keywords2);
                    compareKeywords(kw1,kw2,reference);
                    std::cerr << "Keyword reordering detected in INIT file" << std::endl;
                    /*
                      The keyword reordering should eventually be marked as a an
                      error, but temporarily during the refactoring of 3D
                      properties it is only marked with a message to stderr.
                      This is to avoid deviations between ci builds and
                      developers builds. December 2019 - Joakim Hove
                    */
                    //HANDLE_ERROR(std::runtime_error, "Keyword reordering detected in INIT file");
                } else
                    compareKeywords(keywords1, keywords2, reference);
            } else {
                checkSpesificKeyword(keywords1, keywords2, arrayType1, arrayType2, reference);
            }

            for (size_t i = 0; i < keywords1.size(); i++) {
                auto it1 = std::find(keywords2.begin(), keywords2.end(), keywords1[i]);
                int ind2 = std::distance(keywords2.begin(),it1);

                if (arrayType1[i] != arrayType2[ind2]) {
                    printComparisonForKeywordLists(keywords1, keywords2, arrayType1, arrayType2);
                    OPM_THROW(std::runtime_error, "\nArray with same name '"<< keywords1[i] << "', but of different type. Init file ");
                }

                auto it = std::find(keywordsBlackList.begin(), keywordsBlackList.end(), keywords1[i]);

                if (it != keywordsBlackList.end()){
                    std::cout << "Skipping  " << keywords1[i] << std::endl;
                } else {
                    std::cout << "Comparing " << keywords1[i] << " ... ";

                    if (arrayType1[i] == INTE) {
                        auto vect1 = init1.get<int>(keywords1[i]);
                        auto vect2 = init2.get<int>(keywords2[ind2]);
                        compareVectors(vect1, vect2, keywords1[i],reference);
                    } else if (arrayType1[i] == REAL) {
                        auto vect1 = init1.get<float>(keywords1[i]);
                        auto vect2 = init2.get<float>(keywords2[ind2]);
                        compareFloatingPointVectors(vect1, vect2, keywords1[i], reference);
                    } else if (arrayType1[i] == DOUB) {
                        auto vect1 = init1.get<double>(keywords1[i]);
                        auto vect2 = init2.get<double>(keywords2[ind2]);
                        compareFloatingPointVectors(vect1, vect2, keywords1[i], reference);
                    } else if (arrayType1[i] == LOGI) {
                        auto vect1 = init1.get<bool>(keywords1[i]);
                        auto vect2 = init2.get<bool>(keywords2[ind2]);
                        compareVectors(vect1, vect2, keywords1[i], reference);
                    } else if (arrayType1[i] == CHAR) {
                        auto vect1 = init1.get<std::string>(keywords1[i]);
                        auto vect2 = init2.get<std::string>(keywords2[ind2]);
                        compareVectors(vect1, vect2, keywords1[i], reference);
                    } else if (arrayType1[i] == MESS) {
                        // shold not be any associated data
                    } else {
                        std::cout << "unknown array type " << std::endl;
                        exit(1);
                    }

                    std::cout << " done." << std::endl;
                }
            }

            if (!deviations.empty()) {
                printDeviationReport();
            }
        }
    } else {
        std::cout << "\n!Warning, init files not found, hence not compared. \n" << std::endl;
    }

}


void ECLRegressionTest::results_rst()
{
    std::string fileName1, fileName2;
    bool foundRst1 = checkFileName(rootName1,"UNRST",fileName1);
    bool foundRst2 = checkFileName(rootName2,"UNRST",fileName2);

    if ((foundRst1) && (not foundRst2)){
        std::string message ="test case restart file " + rootName2 + ".UNRST not found";
	std::cout << message << std::endl;
        OPM_THROW(std::runtime_error, message);
    }

    if (foundRst1 && foundRst2) {
        ERst rst1(fileName1);
        std::cout << "\nLoading restart file " << fileName1 << "  .... done" << std::endl;

        ERst rst2(fileName2);
        std::cout << "Loading restart file " << fileName2 << "  .... done\n" << std::endl;

        std::vector<int> seqnums1 = rst1.listOfReportStepNumbers();
        std::vector<int> seqnums2 = rst2.listOfReportStepNumbers();

        deviations.clear();

        if (spesificSequence > -1) {
            auto search1 = std::find(seqnums1.begin(), seqnums1.end(), spesificSequence);
            auto search2 = std::find(seqnums2.begin(), seqnums2.end(), spesificSequence);

            if (search1 == seqnums1.end()) {
                OPM_THROW(std::runtime_error, "\nSpecified sequence " << spesificSequence << " not found in restart files for case 1");
            }

            if (search2 == seqnums2.end()) {
                OPM_THROW(std::runtime_error, "\nSpecified sequence " << spesificSequence << " not found in restart files for case 2");
            }

            seqnums1.clear();
            seqnums1.push_back(spesificSequence);

            seqnums2.clear();
            seqnums2.push_back(spesificSequence);

        } else if (onlyLastSequence) {

            if (seqnums1.back() != seqnums2.back()) {
                OPM_THROW(std::runtime_error, "\nLast sequence not same for for case 1 and case 2");
            }

            seqnums1.clear();
            seqnums1.push_back(seqnums2.back());

            seqnums2.clear();
            seqnums2.push_back(seqnums1.back());
        }

        if (seqnums1 != seqnums2) {
            std::vector<std::string> seqnStrList1;
            std::vector<std::string> seqnStrList2;

            for (auto& val : seqnums1) {
                seqnStrList1.push_back(std::to_string(val));
            }

            for (auto& val : seqnums2) {
                seqnStrList2.push_back(std::to_string(val));
            }
            std::cout << "\nrestart sequences " << std::endl;

            printComparisonForKeywordLists(seqnStrList1, seqnStrList2);
            OPM_THROW(std::runtime_error, "\nRestart files not having the same report steps: ");
        }

        for (int& seqn : seqnums1) {
            std::cout << "\nUnified restart files, sequence  " << std::to_string(seqn) << "\n" << std::endl;

            std::string reference = "Restart, sequence "+std::to_string(seqn);

            rst1.loadReportStepNumber(seqn);
            rst2.loadReportStepNumber(seqn);

            auto arrays1 = rst1.listOfRstArrays(seqn);
            auto arrays2 = rst2.listOfRstArrays(seqn);

            std::vector<std::string> keywords1;
            std::vector<eclArrType> arrayType1;
            for (const auto& array : arrays1) {
                keywords1.push_back(std::get<0>(array));
                arrayType1.push_back(std::get<1>(array));
            }

            std::vector<std::string> keywords2;
            std::vector<eclArrType> arrayType2;

            for (const auto& array : arrays2) {
                keywords2.push_back(std::get<0>(array));
                arrayType2.push_back(std::get<1>(array));
            }

            if (integrationTest) {
                std::vector<std::string> keywords;

                for (size_t i = 0; i < keywords1.size(); i++) {
                    if (keywords1[i] == "PRESSURE" ||
                        keywords1[i] == "SWAT" ||
                        keywords1[i] =="SGAS") {
                        auto search2 = std::find(keywords2.begin(), keywords2.end(), keywords1[i]);
                        if (search2 != keywords2.end()) {
                            keywords.push_back(keywords1[i]);
                        }
                    }
                }

                keywords1 = keywords2 = keywords;

                int nKeys = keywords.size();
                arrayType1.assign(nKeys, REAL);
                arrayType2.assign(nKeys, REAL);
            }

            if (printKeywordOnly) {
                printComparisonForKeywordLists(keywords1, keywords2, arrayType1, arrayType2);
            } else {
                if (spesificKeyword.empty()) {
                    compareKeywords(keywords1, keywords2, reference);
                } else {
                    checkSpesificKeyword(keywords1, keywords2, arrayType1, arrayType2, reference);
                }

                for (size_t i = 0; i < keywords1.size(); i++) {
                    auto it1 = std::find(keywords2.begin(), keywords2.end(), keywords1[i]);
                    int ind2 = std::distance(keywords2.begin(), it1);

                    if (arrayType1[i] != arrayType2[ind2]) {
                        printComparisonForKeywordLists(keywords1, keywords2, arrayType1, arrayType2);
                        OPM_THROW(std::runtime_error, "\nArray with same name '"<< keywords1[i] << "', but of different type. Restart file" <<
                                  " sequenze " << std::to_string(seqn));
                    }

                    auto it = std::find(keywordsBlackList.begin(), keywordsBlackList.end(), keywords1[i]);

                    if (it != keywordsBlackList.end()){
                        std::cout << "Skipping  " << keywords1[i] << std::endl;
                    } else {

                        std::cout << "Comparing " << keywords1[i] << " ... ";

                        if (arrayType1[i] == INTE) {
                            auto vect1 = rst1.getRst<int>(keywords1[i], seqn, 0);
                            auto vect2 = rst2.getRst<int>(keywords2[ind2], seqn, 0);
                            compareVectors(vect1, vect2, keywords1[i], reference);
                        } else if (arrayType1[i] == REAL) {
                            auto vect1 = rst1.getRst<float>(keywords1[i], seqn, 0);
                            auto vect2 = rst2.getRst<float>(keywords2[ind2], seqn, 0);
                            compareFloatingPointVectors(vect1, vect2, keywords1[i], reference);
                        } else if (arrayType1[i] == DOUB) {
                            auto vect1 = rst1.getRst<double>(keywords1[i], seqn, 0);
                            auto vect2 = rst2.getRst<double>(keywords2[ind2], seqn, 0);

                            // hack in order to not test doubhead[1], dependent on simulation results
                            // All ohter items in DOUBHEAD are tested with strict tolerances
                            if (keywords1[i]=="DOUBHEAD"){
                                vect2[1] = vect1[1];
                            }
                            compareFloatingPointVectors(vect1, vect2, keywords1[i], reference);
                        } else if (arrayType1[i] == LOGI) {
                            auto vect1 = rst1.getRst<bool>(keywords1[i], seqn, 0);
                            auto vect2 = rst2.getRst<bool>(keywords2[ind2], seqn, 0);
                            compareVectors(vect1, vect2, keywords1[i], reference);
                        } else if (arrayType1[i] == CHAR) {
                            auto vect1 = rst1.getRst<std::string>(keywords1[i], seqn, 0);
                            auto vect2 = rst2.getRst<std::string>(keywords2[ind2], seqn, 0);
                            compareVectors(vect1, vect2, keywords1[i], reference);
                        } else if (arrayType1[i] == MESS) {
                            // shold not be any associated data
                        } else {
                            std::cout << "unknown array type " << std::endl;
                            exit(1);
                        }

                        std::cout << " done." << std::endl;
                    }
                }
            }
        }

        if (!deviations.empty()) {
            printDeviationReport();
        }
    } else {
        std::cout << "\n!Warning, restart files not found, hence not compared. \n" << std::endl;
    }

}


void ECLRegressionTest::results_smry()
{
    std::string fileName1, fileName2;
    bool foundSmspec1 = checkFileName(rootName1, "SMSPEC", fileName1);
    bool foundSmspec2 = checkFileName(rootName2, "SMSPEC", fileName2);

    if ((foundSmspec1) && (not foundSmspec2)){
        std::string message ="test case summary file " + rootName2 + ".SMSPEC not found";
	std::cout << message << std::endl;
        OPM_THROW(std::runtime_error, message);
    }

    if (foundSmspec1 && foundSmspec2) {
        ESmry smry1(fileName1, loadBaseRunData);
        smry1.LoadData();
        std::cout << "\nLoading summary file " << fileName1 << "  .... done" << std::endl;

        ESmry smry2(fileName2, loadBaseRunData);
        smry2.LoadData();
        std::cout << "\nLoading summary file " << fileName2 << "  .... done" << std::endl;

        deviations.clear();

        std::string reference = "Summary file";

        std::cout << "\nComparing summary files " << std::endl;

        if (reportStepOnly){
            std::cout << " -- Values at report steps will be compared. Time steps in between reports are ignored " << std::endl;
        }

        std::vector<std::string> keywords1 = smry1.keywordList();
        std::vector<std::string> keywords2 = smry2.keywordList();

        std::vector<eclArrType> arrayType1(keywords1.size(), REAL);
        std::vector<eclArrType> arrayType2 (keywords1.size(), REAL);

        if (integrationTest) {
            std::vector<std::string> keywords;

            for (size_t i = 0; i < keywords1.size(); i++) {
                if (keywords1[i].substr(0,5) == "WOPR:" ||
                    keywords1[i].substr(0,5) == "WWPR:" ||
                    keywords1[i].substr(0,5) == "WGPR:" ||
                    keywords1[i].substr(0,5) == "WBHP:" ||
                    keywords1[i].substr(0,4) == "SPR:")  {
                    auto search2 = std::find(keywords2.begin(), keywords2.end(), keywords1[i]);
                    if (search2 != keywords2.end()) {
                        keywords.push_back(keywords1[i]);
                    }
                }
            }

            keywords1 = keywords2 = keywords;

            int nKeys = keywords.size();

            arrayType1.assign(nKeys, REAL);
            arrayType2.assign(nKeys, REAL);
        }

        if (printKeywordOnly) {
            if (keywords1.size() < 50) {
                printComparisonForKeywordLists(keywords1, keywords2);
            } else {
                printMissingKeywords(keywords1, keywords2);
            }
        } else {
            if (spesificKeyword.empty()) {
                compareKeywords(keywords1, keywords2, reference);
            } else {
                checkSpesificKeyword(keywords1, keywords2, arrayType1, arrayType2, reference);
            }

            std::vector<std::string> blackListed;

            for (std::vector<std::string>::iterator keywit = keywords1.begin(); keywit != keywords1.end(); ++keywit) {
                auto it = std::find(keywordsBlackList.begin(), keywordsBlackList.end(), *keywit );

                if (it != keywordsBlackList.end()){
                    blackListed.push_back(*keywit);
                    keywit = keywords1.erase(keywit);
                    if (keywit != keywords1.end())
                      --keywit;
                }
             }

            std::cout << "\nChecking " << keywords1.size() << "  vectors  ... ";

            for (size_t i = 0; i < keywords1.size(); i++) {

                std::vector<float> vect1;
                std::vector<float> vect2;

                if (reportStepOnly){
                    vect1 = smry1.get_at_rstep( keywords1[i]);
                    vect2 = smry2.get_at_rstep( keywords1[i]);
                } else {
                    vect1 = smry1.get( keywords1[i]);
                    vect2 = smry2.get( keywords1[i]);
                }

                if (vect1.size() != vect2.size()) {
                    OPM_THROW(std::runtime_error, "\nKeyword " << keywords1[i] << " summary vector of different length ("
                              << vect1.size() << " != " << vect2.size() <<")");
                }

                compareFloatingPointVectors(vect1, vect2, keywords1[i], reference);
            }

            std::cout << " done." << std::endl;

            if (blackListed.size()>0){
                std::cout << "Number of black listed vectors " << blackListed.size() << " (not compared) " << std::endl;
            }

            if (!deviations.empty()) {
                printDeviationReport();
            }
        }

        namespace fs = Opm::filesystem;
        std::string rsm_file = rootName2 + ".RSM";
        if (fs::is_regular_file(fs::path(rsm_file))) {
            auto rsm = ERsm(rsm_file);
            if (!cmp(smry2, rsm))
                HANDLE_ERROR(std::runtime_error, "The RSM file did not compare equal to the summary file");
        }

    } else {
        std::cout << "\n!Warning, summary files not found, hence not compared. \n" << std::endl;
    }

}


void ECLRegressionTest::results_rft()
{
    std::string fileName1, fileName2;
    bool foundRft1 = checkFileName(rootName1, "RFT", fileName1);
    bool foundRft2 = checkFileName(rootName2, "RFT", fileName2);

    if ((foundRft1) && (not foundRft2)){
        std::string message ="test case rft file " + rootName2 + ".RFT not found";
	std::cout << message << std::endl;
        OPM_THROW(std::runtime_error, message);
    }

    if (foundRft1 && foundRft2) {
        ERft rft1(fileName1);
        std::cout << "\nLoading rft file " << fileName1 << "  .... done" << std::endl;

        ERft rft2(fileName2);
        std::cout << "Loading rft file " << fileName2 << "  .... done\n" << std::endl;

        auto rftReportList1 = rft1.listOfRftReports();
        auto rftReportList2 = rft2.listOfRftReports();

        deviations.clear();

        if (rftReportList1 != rftReportList2) {
            std::vector<std::string> rftList1;
            for (auto& report : rftReportList1) {
                std::string well =  std::get<0>(report);
                std::tuple<int, int, int> date =  std::get<1>(report);
                std::string str1 = well +" (" + std::to_string(std::get<0>(date)) + "/" + std::to_string(std::get<1>(date)) + "/"  +   std::to_string(std::get<2>(date)) + ")";
                rftList1.push_back(str1);
            }

            std::vector<std::string> rftList2;
            for (auto& report : rftReportList2) {
                std::string well =  std::get<0>(report);
                std::tuple<int, int, int> date =  std::get<1>(report);
                std::string str2 = well +" (" + std::to_string(std::get<0>(date)) + "/" + std::to_string(std::get<1>(date)) + "/"  +   std::to_string(std::get<2>(date)) + ")";
                rftList2.push_back(str2);
            }

            printComparisonForKeywordLists(rftList1, rftList2);

            OPM_THROW(std::runtime_error, "\nNot same RFTs in in RFT file ");
        }

        for (auto& report : rftReportList2) {
            std::string well =  std::get<0>(report);
            std::tuple<int, int, int> date =  std::get<1>(report);

            std::string dateStr = std::to_string(std::get<0>(date)) + "/" + std::to_string(std::get<1>(date)) + "/" + std::to_string(std::get<2>(date));

            std::cout << "Well: " << well << " date: " << dateStr << std::endl;

            std::string reference = "RFT: " + well + ", " + dateStr;

            auto vectList1 = rft1.listOfRftArrays(well, date);
            auto vectList2 = rft2.listOfRftArrays(well, date);

            std::vector<std::string> keywords1;
            std::vector<eclArrType> arrayType1;
            for (auto& array : vectList1 ) {
                keywords1.push_back(std::get<0>(array)) ;
                arrayType1.push_back(std::get<1>(array)) ;
            }

            std::vector<std::string> keywords2;
            std::vector<eclArrType> arrayType2;
            for (auto& array : vectList2 ) {
                keywords2.push_back(std::get<0>(array)) ;
                arrayType2.push_back(std::get<1>(array)) ;
            }

            if (printKeywordOnly) {
                printComparisonForKeywordLists(keywords1, keywords2, arrayType1, arrayType2);
            } else {
                if (spesificKeyword.empty()) {
                    compareKeywords(keywords1, keywords2, reference);
                } else {
                    checkSpesificKeyword(keywords1, keywords2, arrayType1, arrayType2, reference);
                }

                for (auto& array : vectList1 ) {
                    std::string keyword = std::get<0>(array);
                    eclArrType arrayType = std::get<1>(array);

                    auto it = std::find(keywordsBlackList.begin(), keywordsBlackList.end(), keyword);

                    if (it != keywordsBlackList.end()){
                        std::cout << "Skipping  " << keyword << std::endl;
                    } else {
                        std::cout << "Comparing: " << keyword << " ... ";

                        if (arrayType == INTE) {
                            auto vect1 = rft1.getRft<int>(keyword, well, date);
                            auto vect2 = rft2.getRft<int>(keyword, well, date);
                            compareVectors(vect1, vect2, keyword, reference);
                        } else if (arrayType == REAL) {
                            auto vect1 = rft1.getRft<float>(keyword, well, date);
                            auto vect2 = rft2.getRft<float>(keyword, well, date);
                            compareFloatingPointVectors(vect1, vect2, keyword, reference);
                        } else if (arrayType == DOUB) {
                            auto vect1 = rft1.getRft<double>(keyword, well, date);
                            auto vect2 = rft2.getRft<double>(keyword, well, date);
                            compareFloatingPointVectors(vect1, vect2, keyword, reference);
                        } else if (arrayType == LOGI) {
                            auto vect1 = rft1.getRft<bool>(keyword, well, date);
                            auto vect2 = rft2.getRft<bool>(keyword, well, date);
                            compareVectors(vect1, vect2, keyword, reference);
                        } else if (arrayType == CHAR) {
                            auto vect1 = rft1.getRft<std::string>(keyword, well, date);
                            auto vect2 = rft2.getRft<std::string>(keyword, well, date);
                            compareVectors(vect1, vect2, keyword, reference);
                        } else if (arrayType == MESS) {
                            // shold not be any associated data
                        } else {
                            std::cout << "unknown array type " << std::endl;
                            exit(1);
                        }

                        std::cout << " done." << std::endl;
                    }
                }
            }
            std::cout << std::endl;
        }

        if (!deviations.empty()) {
            printDeviationReport();
        }
    } else {
        std::cout << "\n!Warning, rft files not found, hence not compared. \n" << std::endl;
    }
}


void ECLRegressionTest::printComparisonForKeywordLists(const std::vector<std::string>& arrayList1,
                                                       const std::vector<std::string>& arrayList2,
                                                       const std::vector<eclArrType>& arrayType1,
                                                       const std::vector<eclArrType>& arrayType2) const
{
    unsigned int maxLen = 0;

    std::vector<std::string> arrTypeStrList = {"INTE", "REAL", "DOUB", "CHAR", "LOGI", "MESS"};

    std::set<std::string> commonList;

    for (auto& key : arrayList1) {
        commonList.insert(key);
    }

    for (auto& key : arrayList2) {
        commonList.insert(key);
    }

    for (auto& key : commonList) {
        if (key.size() > maxLen) {
            maxLen = key.size();
        }
    }

    maxLen += 4;

    std::cout << std::endl;

    for (auto& it : commonList) {
        auto it1 = std::find(arrayList1.begin(), arrayList1.end(), it);
        int ind1 = std::distance(arrayList1.begin(), it1);

        auto it2 = std::find(arrayList2.begin(), arrayList2.end(), it);
        int ind2 = std::distance(arrayList2.begin(),it2);

        if (arrayType1[ind1] != arrayType2[ind2]) {
            std::cout << "\033[1;31m";
        }

        if (std::find(arrayList1.begin(), arrayList1.end(), it) != arrayList1.end()) {
            std::cout <<  std::setw(maxLen) << it << " (" <<  arrTypeStrList[arrayType1[ind1]] << ") | ";
        } else {
            std::cout <<  std::setw(maxLen) << "" << "        | ";
        }

        if (std::find(arrayList2.begin(), arrayList2.end(), it) != arrayList2.end()) {
            std::cout <<  std::setw(maxLen) << it << " (" <<  arrTypeStrList[arrayType2[ind2]] << ") ";
        } else {
            std::cout <<  std::setw(maxLen) << "";
        }

        if (arrayType1[ind1] != arrayType2[ind2]) {
            std::cout << " !" << "\033[0m";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl << std::endl;
}


void ECLRegressionTest::printMissingKeywords(const std::vector<std::string>& arrayList1,
                                             const std::vector<std::string>& arrayList2) const
{
    std::set<std::string> commonList;

    for (auto& key : arrayList1) {
        commonList.insert(key);
    }

    for (auto& key : arrayList2) {
        commonList.insert(key);
    }

    std::cout << "\nKeywords found in second case, but missing in first case: \n" << std::endl;

    for (auto& it : commonList) {
        if (std::find(arrayList1.begin(), arrayList1.end(), it) == arrayList1.end()) {
            std::cout << "  > '" << it  << "'" << std::endl;
        }
    }

    std::cout << "\nKeywords found in first case, but missing in second case: \n" << std::endl;

    for (auto& it : commonList) {
        if (std::find(arrayList2.begin(), arrayList2.end(), it) == arrayList2.end()) {
            std::cout << "  > '" << it  << "'" << std::endl;
        }
    }
}


void ECLRegressionTest::printComparisonForKeywordLists(const std::vector<std::string>& arrayList1,
                                                       const std::vector<std::string>& arrayList2) const
{
    std::set<std::string> commonList;
    unsigned int maxLen = 0;

    for (auto& key : arrayList1) {
        commonList.insert(key);
    }

    for (auto& key : arrayList2) {
        commonList.insert(key);
    }

    for (auto& key : commonList) {
        if (key.size() > maxLen) {
            maxLen = key.size();
        }
    }

    maxLen += 2;

    std::cout << std::endl;

    for (auto& it : commonList) {
        if (std::find(arrayList1.begin(), arrayList1.end(), it) != arrayList1.end()) {
            std::cout <<  std::setw(maxLen) << it  << " | ";
        } else {
            std::cout <<  std::setw(maxLen) << "" << " | ";
        }

        if (std::find(arrayList2.begin(), arrayList2.end(), it) != arrayList2.end()) {
            std::cout <<  std::setw(maxLen) << it << "";
        } else {
            std::cout <<  std::setw(maxLen) << "" ;
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
}
