/*
   Copyright 2019 Equinor ASA.

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

#ifndef ECLREGRESSIONTEST_HPP
#define ECLREGRESSIONTEST_HPP

#include "EclFilesComparator.hpp"

#include <opm/io/eclipse/EclIOdata.hpp>

namespace Opm { namespace EclIO {
    class EGrid;
}}

namespace EIOD = Opm::EclIO;


/*! \brief A class for executing a regression test for two ECLIPSE files.
    \details This class inherits from ECLFilesComparator, which opens and
             closes the input cases and stores keywordnames.
             The three public functions gridCompare(), results() and
             resultsForKeyword() can be invoked to compare griddata
             or keyworddata for all keywords or a given keyword (resultsForKeyword()).
 */
class ECLRegressionTest: public ECLFilesComparator {
public:
    //! \brief Sets up the regression test.
    //! \param[in] basename1 Full path without file extension to the first case.
    //! \param[in] basename2 Full path without file extension to the second case.
    //! \param[in] absTolerance Tolerance for absolute deviation.
    //! \param[in] relTolerance Tolerance for relative deviation.
    //! \details This constructor only calls the constructor of the superclass, see the docs for ECLFilesComparator for more information.
    ECLRegressionTest(const std::string& basename1, const std::string& basename2,
                      double absToleranceArg, double relToleranceArg):
        ECLFilesComparator(basename1, basename2, absToleranceArg, relToleranceArg) {}

    ~ECLRegressionTest();

    //! \brief Option to only compare last occurrence
    void setOnlyLastReportNumber(bool onlyLastSequenceArg) {
        this->onlyLastSequence = onlyLastSequenceArg;
    }

    int countDev() { return  deviations.size(); }

    // Accept extra keywords: If this switch is set to true the comparison
    // will ignore extra keywords which are only present
    // in the new simulation.

    void setReportStepOnly(bool reportStepOnlyArg) {
        this->reportStepOnly = reportStepOnlyArg;
    }

    void setAcceptExtraKeywords(bool acceptExtraKeywordsArg) {
        this->acceptExtraKeywords = acceptExtraKeywordsArg;
    }

    void setIntegrationTest(bool inregrationTestArg) {
        this->integrationTest = inregrationTestArg;
    }

    void setPrintKeywordOnly(bool printArg) {
        this->printKeywordOnly = printArg;
    }

    void compareSpesificKeyword(std::string keyword) {
        this->spesificKeyword = std::move(keyword);
    }
    void compareSpesificRstReportStepNumber(int seqn) {
        this->spesificSequence = seqn;
    }

    void setLoadBaseRunData(bool loadArg) {
        this->loadBaseRunData = loadArg;
    }

    void loadGrids();
    void printDeviationReport();

    void gridCompare();

    void results_rst();
    void results_init();
    void results_smry();
    void results_rft();

private:
    bool checkFileName(const std::string& rootName, const std::string& extension, std::string& filename);

    // Prints results stored in absDeviation and relDeviation.
    void printResultsForKeyword(const std::string& keyword) const;
    void printComparisonForKeywordLists(const std::vector<std::string>& arrayList1,
                                        const std::vector<std::string>& arrayList2) const;

    void printComparisonForKeywordLists(const std::vector<std::string>& arrayList1,
                                        const std::vector<std::string>& arrayList2,
                                        const std::vector<EIOD::eclArrType>& arrayType1,
                                        const std::vector<EIOD::eclArrType>& arrayType2) const;

    void printMissingKeywords(const std::vector<std::string>& arrayList1,
                              const std::vector<std::string>& arrayList2) const;

    void compareKeywords(const std::vector<std::string>& keywords1,
                         const std::vector<std::string>& keywords2,
                         const std::string& reference);

    void checkSpesificKeyword(std::vector<std::string>& keywords1,
                              std::vector<std::string>& keywords2,
                              std::vector<EIOD::eclArrType>& arrayType1,
                              std::vector<EIOD::eclArrType>& arrayType2,
                              const std::string& reference);

    template <typename T>
    void compareVectors(const std::vector<T>& t1, const std::vector<T>& t2,
                        const std::string& keyword, const std::string& reference);

    template <typename T>
    void compareFloatingPointVectors(const std::vector<T>& t1, const std::vector<T> &t2,
                                     const std::string& keyword, const std::string& reference);

    // deviationsForCell throws an exception if both the absolute deviation AND the relative deviation
    // are larger than absTolerance and relTolerance, respectively. In addition,
    // if allowNegativeValues is passed as false, an exception will be thrown when the absolute value
    // of a negative value exceeds absTolerance. If no exceptions are thrown, the absolute and relative deviations are added to absDeviation and relDeviation.
    // void deviationsForCell(double val1, double val2, const std::string& keyword, const std::string reference, size_t kw_size, size_t cell, bool allowNegativeValues = true);

    void deviationsForCell(double val1, double val2, const std::string& keyword,
                           const std::string& reference, size_t kw_size, size_t cell,
                           bool allowNegativeValues, bool useStrictTol);

    template <typename T>
    void deviationsForNonFloatingPoints(T val1, T val2, const std::string& keyword,
                                        const std::string& reference,
                                        size_t kw_size, size_t cell);

    // These vectors store absolute and relative deviations, respecively. Note that they are whiped clean for every new keyword comparison.
    std::vector<double> absDeviation, relDeviation;

    // Keywords which should not contain negative values, i.e. uses allowNegativeValues = false in deviationsForCell():
    const std::vector<std::string> keywordDisallowNegatives = {"SGAS", "SWAT", "PRESSURE"};

    double strictAbsTol = 1e-6;
    double strictRelTol = 1e-6;

    // keywords that triggers strict tolerances
    const std::vector<std::string> keywordsStrictTol = {"COORD", "ZCORN", "PORV", "DEPTH", "DX", "DY", "DZ", "PERMX", "PERMY", "PERMZ", "NTG",
                                                        "TRANX", "TRANY", "TRANZ", "TRANNNC", "SGRP", "SCON", "DOUBHEAD"
                                                       };
    // keywords that should not be compared
    const std::vector<std::string> keywordsBlackList = {"TCPU", "ELAPSED", "TCPUDAY", "TCPUTS", "TELAPLIN", "TCPUH", "TCPUHT", "TCPUSCH", "TCPUTSH", "TCPUTSHT", "TELAPDAY", "TELAPTS"};

    bool reportStepOnly = false;

    // Only compare last occurrence
    bool onlyLastSequence = false;

    bool integrationTest = false;

    bool printKeywordOnly = false;

    bool loadBaseRunData = false;

    // spesific keyword to be compared
    std::string spesificKeyword;

    // spesific restart sequence to be compared
    int spesificSequence = -1;

    // Accept extra keywords in the restart file of the 'new' simulation.
    bool acceptExtraKeywords = false;

    Opm::EclIO::EGrid* grid1 = nullptr;
    Opm::EclIO::EGrid* grid2 = nullptr;
};

#endif
