/*
  Copyright 2019 Equinor

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

#define BOOST_TEST_MODULE OutputStream

#include <boost/test/unit_test.hpp>

#include <opm/io/eclipse/OutputStream.hpp>

#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/EclOutput.hpp>
#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/PaddedOutputString.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/io/eclipse/EclIOdata.hpp>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iterator>
#include <ostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <opm/common/utility/FileSystem.hpp>

namespace Opm { namespace EclIO {

    // Needed by BOOST_CHECK_EQUAL_COLLECTIONS.
    std::ostream&
    operator<<(std::ostream& os, const EclFile::EclEntry& e)
    {
        os << "{ " << std::get<0>(e)
           << ", " << static_cast<int>(std::get<1>(e))
           << ", " << std::get<2>(e)
           << " }";

        return os;
    }
}} // Namespace Opm::ecl

namespace {
    template <class Coll>
    void check_is_close(const Coll& c1, const Coll& c2)
    {
        using ElmType = typename std::remove_cv<
            typename std::remove_reference<
                typename std::iterator_traits<
                    decltype(std::begin(c1))
                >::value_type
            >::type
        >::type;

        for (auto b1  = c1.begin(), e1 = c1.end(), b2 = c2.begin();
                  b1 != e1; ++b1, ++b2)
        {
            BOOST_CHECK_CLOSE(*b1, *b2, static_cast<ElmType>(1.0e-7));
        }
    }
} // Anonymous namespace

BOOST_AUTO_TEST_SUITE(FileName)

BOOST_AUTO_TEST_CASE(ResultSetDescriptor)
{
    const auto odir = std::string{"/x/y/z/"};
    const auto ext  = std::string{"F0123"};

    {
        const auto rset = ::Opm::EclIO::OutputStream::ResultSet {
            odir, "CASE"
        };

        const auto fname = outputFileName(rset, ext);

        BOOST_CHECK_EQUAL(fname, odir + "CASE.F0123");
    }

    {
        const auto rset = ::Opm::EclIO::OutputStream::ResultSet {
            odir, "CASE." // CASE DOT
        };

        const auto fname = outputFileName(rset, ext);

        BOOST_CHECK_EQUAL(fname, odir + "CASE.F0123");
    }

    {
        const auto rset = ::Opm::EclIO::OutputStream::ResultSet {
            odir, "CASE.01"
        };

        const auto fname = outputFileName(rset, ext);

        BOOST_CHECK_EQUAL(fname, odir + "CASE.01.F0123");
    }

    {
        const auto rset = ::Opm::EclIO::OutputStream::ResultSet {
            odir, "CASE.01." // CASE.01 DOT
        };

        const auto fname = outputFileName(rset, ext);

        BOOST_CHECK_EQUAL(fname, odir + "CASE.01.F0123");
    }
}

BOOST_AUTO_TEST_SUITE_END() // FileName

// ==========================================================================
class RSet
{
public:
    explicit RSet(std::string base)
        : odir_(std::filesystem::temp_directory_path() /
                Opm::unique_path("rset-%%%%"))
        , base_(std::move(base))
    {
        std::filesystem::create_directories(this->odir_);
    }

    ~RSet()
    {
        std::filesystem::remove_all(this->odir_);
    }

    operator ::Opm::EclIO::OutputStream::ResultSet() const
    {
        return { this->odir_.string(), this->base_ };
    }

private:
    std::filesystem::path odir_;
    std::string             base_;
};

// ==========================================================================

BOOST_AUTO_TEST_SUITE(Class_Init)

BOOST_AUTO_TEST_CASE(Unformatted)
{
    const auto rset = RSet("CASE");
    const auto fmt  = ::Opm::EclIO::OutputStream::Formatted{ false };

    {
        auto init = ::Opm::EclIO::OutputStream::Init {
            rset, fmt
        };

        init.write("I", std::vector<int>   {1, 7, 2, 9});
        init.write("L", std::vector<bool>  {true, false, false, true});
        init.write("S", std::vector<float> {3.1f, 4.1f, 59.265f});
        init.write("D", std::vector<double>{2.71, 8.21});
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "INIT");

        auto init = ::Opm::EclIO::EclFile{fname};

        BOOST_CHECK(init.hasKey("I"));
        BOOST_CHECK(init.hasKey("L"));
        BOOST_CHECK(init.hasKey("S"));
        BOOST_CHECK(init.hasKey("D"));

        {
            const auto vectors        = init.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        init.loadData();

        {
            const auto& I = init.get<int>("I");
            const auto  expect_I = std::vector<int>{ 1, 7, 2, 9 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = init.get<bool>("L");
            const auto  expect_L = std::vector<bool> {
                true, false, false, true,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = init.get<float>("S");
            const auto  expect_S = std::vector<float>{
                3.1f, 4.1f, 59.265f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = init.get<double>("D");
            const auto  expect_D = std::vector<double>{
                2.71, 8.21,
            };

            check_is_close(D, expect_D);
        }
    }

    // Second write request replaces original contents
    {
        auto init = ::Opm::EclIO::OutputStream::Init {
            rset, fmt
        };

        init.write("I2", std::vector<int>   {1, 2, 3, 4, 5, 6});
        init.write("L2", std::vector<bool>  {false, false, true, true});
        init.write("S2", std::vector<float> {-1.0f, 2.0f, -3.0e-4f});
        init.write("D2", std::vector<double>{2.71, 8.21, 18.28459});
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "INIT");

        auto init = ::Opm::EclIO::EclFile{fname};

        BOOST_CHECK(!init.hasKey("I"));
        BOOST_CHECK(!init.hasKey("L"));
        BOOST_CHECK(!init.hasKey("S"));
        BOOST_CHECK(!init.hasKey("D"));

        BOOST_CHECK(init.hasKey("I2"));
        BOOST_CHECK(init.hasKey("L2"));
        BOOST_CHECK(init.hasKey("S2"));
        BOOST_CHECK(init.hasKey("D2"));

        {
            const auto vectors        = init.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I2", Opm::EclIO::eclArrType::INTE, 6},
                Opm::EclIO::EclFile::EclEntry{"L2", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S2", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"D2", Opm::EclIO::eclArrType::DOUB, 3},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        init.loadData();

        {
            const auto& I = init.get<int>("I2");
            const auto  expect_I = std::vector<int>{ 1, 2, 3, 4, 5, 6 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = init.get<bool>("L2");
            const auto  expect_L = std::vector<bool> {
                false, false, true, true,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = init.get<float>("S2");
            const auto  expect_S = std::vector<float>{
                -1.0f, 2.0f, -3.0e-4f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = init.get<double>("D2");
            const auto  expect_D = std::vector<double>{
                2.71, 8.21, 18.28459,
            };

            check_is_close(D, expect_D);
        }
    }
}

BOOST_AUTO_TEST_CASE(Formatted)
{
    const auto rset = RSet("CASE");
    const auto fmt  = ::Opm::EclIO::OutputStream::Formatted{ true };

    {
        auto init = ::Opm::EclIO::OutputStream::Init {
            rset, fmt
        };

        init.write("I", std::vector<int>   {1, 7, 2, 9});
        init.write("L", std::vector<bool>  {true, false, false, true});
        init.write("S", std::vector<float> {3.1f, 4.1f, 59.265f});
        init.write("D", std::vector<double>{2.71, 8.21});
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "FINIT");

        auto init = ::Opm::EclIO::EclFile{fname};

        BOOST_CHECK(init.hasKey("I"));
        BOOST_CHECK(init.hasKey("L"));
        BOOST_CHECK(init.hasKey("S"));
        BOOST_CHECK(init.hasKey("D"));

        {
            const auto vectors        = init.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        init.loadData();

        {
            const auto& I = init.get<int>("I");
            const auto  expect_I = std::vector<int>{ 1, 7, 2, 9 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = init.get<bool>("L");
            const auto  expect_L = std::vector<bool> {
                true, false, false, true,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = init.get<float>("S");
            const auto  expect_S = std::vector<float>{
                3.1f, 4.1f, 59.265f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = init.get<double>("D");
            const auto  expect_D = std::vector<double>{
                2.71, 8.21,
            };

            check_is_close(D, expect_D);
        }
    }


    // Second write request replaces original contents
    {
        auto init = ::Opm::EclIO::OutputStream::Init {
            rset, fmt
        };

        init.write("I2", std::vector<int>   {1, 2, 3, 4, 5, 6});
        init.write("L2", std::vector<bool>  {false, false, true, true});
        init.write("S2", std::vector<float> {-1.0f, 2.0f, -3.0e-4f});
        init.write("D2", std::vector<double>{2.71, 8.21, 18.28459});
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "FINIT");

        auto init = ::Opm::EclIO::EclFile{fname};

        BOOST_CHECK(!init.hasKey("I"));
        BOOST_CHECK(!init.hasKey("L"));
        BOOST_CHECK(!init.hasKey("S"));
        BOOST_CHECK(!init.hasKey("D"));

        BOOST_CHECK(init.hasKey("I2"));
        BOOST_CHECK(init.hasKey("L2"));
        BOOST_CHECK(init.hasKey("S2"));
        BOOST_CHECK(init.hasKey("D2"));

        {
            const auto vectors        = init.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I2", Opm::EclIO::eclArrType::INTE, 6},
                Opm::EclIO::EclFile::EclEntry{"L2", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S2", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"D2", Opm::EclIO::eclArrType::DOUB, 3},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        init.loadData();

        {
            const auto& I = init.get<int>("I2");
            const auto  expect_I = std::vector<int>{ 1, 2, 3, 4, 5, 6 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = init.get<bool>("L2");
            const auto  expect_L = std::vector<bool> {
                false, false, true, true,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = init.get<float>("S2");
            const auto  expect_S = std::vector<float>{
                -1.0f, 2.0f, -3.0e-4f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = init.get<double>("D2");
            const auto  expect_D = std::vector<double>{
                2.71, 8.21, 18.28459,
            };

            check_is_close(D, expect_D);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

// ==========================================================================

BOOST_AUTO_TEST_SUITE(Class_Restart)

BOOST_AUTO_TEST_CASE(Unformatted_Unified)
{
    const auto rset = RSet("CASE");
    const auto fmt  = ::Opm::EclIO::OutputStream::Formatted{ false };
    const auto unif = ::Opm::EclIO::OutputStream::Unified  { true };

    {
        const auto seqnum = 1;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {1, 7, 2, 9});
        rst.write("L", std::vector<bool>       {true, false, false, true});
        rst.write("S", std::vector<float>      {3.1f, 4.1f, 59.265f});
        rst.write("D", std::vector<double>     {2.71, 8.21});
        rst.write("Z", std::vector<std::string>{"W1", "W2"});
    }

    {
        const auto seqnum = 13;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {35, 51, 13});
        rst.write("L", std::vector<bool>       {true, true, true, false});
        rst.write("S", std::vector<float>      {17.29e-02f, 1.4142f});
        rst.write("D", std::vector<double>     {0.6931, 1.6180, 123.45e6});
        rst.write("Z", std::vector<std::string>{"G1", "FIELD"});
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "UNRST");

        auto rst = ::Opm::EclIO::ERst{fname};

        BOOST_CHECK(rst.hasReportStepNumber( 1));
        BOOST_CHECK(rst.hasReportStepNumber(13));

        {
            const auto seqnum        = rst.listOfReportStepNumbers();
            const auto expect_seqnum = std::vector<int>{1, 13};

            BOOST_CHECK_EQUAL_COLLECTIONS(seqnum.begin(), seqnum.end(),
                                          expect_seqnum.begin(),
                                          expect_seqnum.end());
        }

        {
            const auto vectors        = rst.listOfRstArrays(13);
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"SEQNUM", Opm::EclIO::eclArrType::INTE, 1},
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 3},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 2},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rst.loadReportStepNumber(13);

        {
            const auto& I = rst.getRestartData<int>("I", 13, 0);
            const auto  expect_I = std::vector<int>{ 35, 51, 13};
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = rst.getRestartData<bool>("L", 13, 0);
            const auto  expect_L = std::vector<bool> {
                true, true, true, false,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = rst.getRestartData<float>("S", 13, 0);
            const auto  expect_S = std::vector<float>{
                17.29e-02f, 1.4142f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = rst.getRestartData<double>("D", 13, 0);
            const auto  expect_D = std::vector<double>{
                0.6931, 1.6180, 123.45e6,
            };

            check_is_close(D, expect_D);
        }

        {
            const auto& Z = rst.getRestartData<std::string>("Z", 13, 0);
            const auto  expect_Z = std::vector<std::string>{
                "G1", "FIELD",  // ERst trims trailing blanks
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(),
                                          expect_Z.end());
        }
    }

    {
        const auto seqnum = 5;  // Before 13.  Should overwrite 13
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {1, 2, 3, 4});
        rst.write("L", std::vector<bool>       {false, false, false, true});
        rst.write("S", std::vector<float>      {1.23e-04f, 1.234e5f, -5.4321e-9f});
        rst.write("D", std::vector<double>     {0.6931, 1.6180});
        rst.write("Z", std::vector<std::string>{"HELLO", ", ", "WORLD"});
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "UNRST");

        auto rst = ::Opm::EclIO::ERst{fname};

        BOOST_CHECK( rst.hasReportStepNumber( 1));
        BOOST_CHECK( rst.hasReportStepNumber( 5));
        BOOST_CHECK(!rst.hasReportStepNumber(13));

        {
            const auto seqnum        = rst.listOfReportStepNumbers();
            const auto expect_seqnum = std::vector<int>{1, 5};

            BOOST_CHECK_EQUAL_COLLECTIONS(seqnum.begin(), seqnum.end(),
                                          expect_seqnum.begin(),
                                          expect_seqnum.end());
        }

        {
            const auto vectors        = rst.listOfRstArrays(5);
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"SEQNUM", Opm::EclIO::eclArrType::INTE, 1},
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 2},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 3},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rst.loadReportStepNumber(5);

        {
            const auto& I = rst.getRestartData<int>("I", 5, 0);
            const auto  expect_I = std::vector<int>{ 1, 2, 3, 4 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = rst.getRestartData<bool>("L", 5, 0);
            const auto  expect_L = std::vector<bool> {
                false, false, false, true,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = rst.getRestartData<float>("S", 5, 0);
            const auto  expect_S = std::vector<float>{
                1.23e-04f, 1.234e5f, -5.4321e-9f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = rst.getRestartData<double>("D", 5, 0);
            const auto  expect_D = std::vector<double>{
                0.6931, 1.6180,
            };

            check_is_close(D, expect_D);
        }

        {
            const auto& Z = rst.getRestartData<std::string>("Z", 5, 0);
            const auto  expect_Z = std::vector<std::string>{
                "HELLO", ",", "WORLD",  // ERst trims trailing blanks
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(),
                                          expect_Z.end());
        }
    }

    {
        const auto seqnum = 13;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {35, 51, 13});
        rst.write("L", std::vector<bool>       {true, true, true, false});
        rst.write("S", std::vector<float>      {17.29e-02f, 1.4142f});
        rst.write("D", std::vector<double>     {0.6931, 1.6180, 123.45e6});
        rst.write("Z", std::vector<std::string>{"G1", "FIELD"});
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "UNRST");

        auto rst = ::Opm::EclIO::ERst{fname};

        BOOST_CHECK(rst.hasReportStepNumber( 1));
        BOOST_CHECK(rst.hasReportStepNumber( 5));
        BOOST_CHECK(rst.hasReportStepNumber(13));

        {
            const auto seqnum        = rst.listOfReportStepNumbers();
            const auto expect_seqnum = std::vector<int>{1, 5, 13};

            BOOST_CHECK_EQUAL_COLLECTIONS(seqnum.begin(), seqnum.end(),
                                          expect_seqnum.begin(),
                                          expect_seqnum.end());
        }

        {
            const auto vectors        = rst.listOfRstArrays(13);
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"SEQNUM", Opm::EclIO::eclArrType::INTE, 1},
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 3},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 2},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rst.loadReportStepNumber(13);

        {
            const auto& I = rst.getRestartData<int>("I", 13, 0);
            const auto  expect_I = std::vector<int>{ 35, 51, 13};
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = rst.getRestartData<bool>("L", 13, 0);
            const auto  expect_L = std::vector<bool> {
                true, true, true, false,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = rst.getRestartData<float>("S", 13, 0);
            const auto  expect_S = std::vector<float>{
                17.29e-02f, 1.4142f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = rst.getRestartData<double>("D", 13, 0);
            const auto  expect_D = std::vector<double>{
                0.6931, 1.6180, 123.45e6,
            };

            check_is_close(D, expect_D);
        }

        {
            const auto& Z = rst.getRestartData<std::string>("Z", 13, 0);
            const auto  expect_Z = std::vector<std::string>{
                "G1", "FIELD",  // ERst trims trailing blanks
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(),
                                          expect_Z.end());
        }
    }
}

BOOST_AUTO_TEST_CASE(Formatted_Separate)
{
    const auto rset = RSet("CASE.T01.");
    const auto fmt  = ::Opm::EclIO::OutputStream::Formatted{ true };
    const auto unif = ::Opm::EclIO::OutputStream::Unified  { false };

    {
        const auto seqnum = 1;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {1, 7, 2, 9});
        rst.write("L", std::vector<bool>       {true, false, false, true});
        rst.write("S", std::vector<float>      {3.1f, 4.1f, 59.265f});
        rst.write("D", std::vector<double>     {2.71, 8.21});
        rst.write("Z", std::vector<std::string>{"W1", "W2"});
    }

    {
        const auto seqnum = 13;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {35, 51, 13});
        rst.write("L", std::vector<bool>       {true, true, true, false});
        rst.write("S", std::vector<float>      {17.29e-02f, 1.4142f});
        rst.write("D", std::vector<double>     {0.6931, 1.6180, 123.45e6});
        rst.write("Z", std::vector<std::string>{"G1", "FIELD"});
    }

    {
        using ::Opm::EclIO::OutputStream::Restart;

        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "F0013");

        auto rst = ::Opm::EclIO::EclFile{fname};

        {
            const auto vectors        = rst.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                // No SEQNUM in separate output files
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 3},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 2},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rst.loadData();

        {
            const auto& I = rst.get<int>("I");
            const auto  expect_I = std::vector<int>{ 35, 51, 13 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = rst.get<bool>("L");
            const auto  expect_L = std::vector<bool> {
                true, true, true, false,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = rst.get<float>("S");
            const auto  expect_S = std::vector<float>{
                17.29e-02f, 1.4142f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = rst.get<double>("D");
            const auto  expect_D = std::vector<double>{
                0.6931, 1.6180, 123.45e6,
            };

            check_is_close(D, expect_D);
        }

        {
            const auto& Z = rst.get<std::string>("Z");
            const auto  expect_Z = std::vector<std::string>{
                "G1", "FIELD",  // ERst trims trailing blanks
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(),
                                          expect_Z.end());
        }
    }

    {
        // Separate output.  Step 13 should be unaffected.
        const auto seqnum = 5;
        auto rst = ::Opm::EclIO::OutputStream::Restart {
            rset, seqnum, fmt, unif
        };

        rst.write("I", std::vector<int>        {1, 2, 3, 4});
        rst.write("L", std::vector<bool>       {false, false, false, true});
        rst.write("S", std::vector<float>      {1.23e-04f, 1.234e5f, -5.4321e-9f});
        rst.write("D", std::vector<double>     {0.6931, 1.6180});
        rst.write("Z", std::vector<std::string>{"HELLO", ", ", "WORLD"});
    }

    {
        using ::Opm::EclIO::OutputStream::Restart;

        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "F0005");

        auto rst = ::Opm::EclIO::EclFile{fname};

        {
            const auto vectors        = rst.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                // No SEQNUM in separate output files
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 2},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 3},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rst.loadData();

        {
            const auto& I = rst.get<int>("I");
            const auto  expect_I = std::vector<int>{ 1, 2, 3, 4 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = rst.get<bool>("L");
            const auto  expect_L = std::vector<bool> {
                false, false, false, true,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = rst.get<float>("S");
            const auto  expect_S = std::vector<float>{
                1.23e-04f, 1.234e5f, -5.4321e-9f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = rst.get<double>("D");
            const auto  expect_D = std::vector<double>{
                0.6931, 1.6180,
            };

            check_is_close(D, expect_D);
        }

        {
            const auto& Z = rst.get<std::string>("Z");
            const auto  expect_Z = std::vector<std::string>{
                "HELLO", ",", "WORLD",  // ERst trims trailing blanks
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(),
                                          expect_Z.end());
        }
    }

    // -------------------------------------------------------
    // Don't rewrite step 13.  Output file should still exist.
    // -------------------------------------------------------

    {
        using ::Opm::EclIO::OutputStream::Restart;

        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "F0013");

        auto rst = ::Opm::EclIO::EclFile{fname};

        {
            const auto vectors        = rst.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                // No SEQNUM in separate output files.
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 3},
                Opm::EclIO::EclFile::EclEntry{"L", Opm::EclIO::eclArrType::LOGI, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 2},
                Opm::EclIO::EclFile::EclEntry{"D", Opm::EclIO::eclArrType::DOUB, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rst.loadData();

        {
            const auto& I = rst.get<int>("I");
            const auto  expect_I = std::vector<int>{ 35, 51, 13 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& L = rst.get<bool>("L");
            const auto  expect_L = std::vector<bool> {
                true, true, true, false,
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(L.begin(), L.end(),
                                          expect_L.begin(),
                                          expect_L.end());
        }

        {
            const auto& S = rst.get<float>("S");
            const auto  expect_S = std::vector<float>{
                17.29e-02f, 1.4142f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& D = rst.get<double>("D");
            const auto  expect_D = std::vector<double>{
                0.6931, 1.6180, 123.45e6,
            };

            check_is_close(D, expect_D);
        }

        {
            const auto& Z = rst.get<std::string>("Z");
            const auto  expect_Z = std::vector<std::string>{
                "G1", "FIELD",  // ERst trims trailing blanks
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(),
                                          expect_Z.end());
        }
    }
}

BOOST_AUTO_TEST_SUITE_END() // Class_Restart

// ==========================================================================

BOOST_AUTO_TEST_SUITE(Class_RFT)

BOOST_AUTO_TEST_CASE(Unformatted_New)
{
    using Char8 = ::Opm::EclIO::PaddedOutputString<8>;

    const auto rset  = RSet("CASE");
    const auto fmt   = ::Opm::EclIO::OutputStream::Formatted{ false };
    const auto exist = ::Opm::EclIO::OutputStream::RFT::OpenExisting{ false };

    {
        auto rft = ::Opm::EclIO::OutputStream::RFT {
            rset, fmt, exist
        };

        rft.write("I", std::vector<int>   {1, 7, 2, 9});
        rft.write("S", std::vector<float> {3.1f, 4.1f, 59.265f});
        rft.write("Z", std::vector<Char8> {
            Char8{"  Hello "}, Char8{" World "}
        });
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "RFT");

        auto rft = ::Opm::EclIO::EclFile{fname};

        BOOST_CHECK(rft.hasKey("I"));
        BOOST_CHECK(rft.hasKey("S"));
        BOOST_CHECK(rft.hasKey("Z"));
        BOOST_CHECK(!rft.hasKey("C"));

        {
            const auto vectors        = rft.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rft.loadData();

        {
            const auto& I = rft.get<int>("I");
            const auto  expect_I = std::vector<int>{ 1, 7, 2, 9 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& S = rft.get<float>("S");
            const auto  expect_S = std::vector<float>{
                3.1f, 4.1f, 59.265f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& Z = rft.get<std::string>("Z");
            const auto  expect_Z = std::vector<std::string>{
                "  Hello", " World" // Trailing blanks trimmed
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(), expect_Z.end());
        }
    }

    {
        auto rft = ::Opm::EclIO::OutputStream::RFT {
            rset, fmt, exist
        };

        rft.write("I2", std::vector<int>   {11, 22, 33});
        rft.write("S2", std::vector<float> {2.71f, 828.1f, 8.218f});
        rft.write("Z2", std::vector<Char8> {
            Char8{"Good B"}, Char8{" ye"}, Char8{ "W0rlD" },
        });
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "RFT");

        auto rft = ::Opm::EclIO::EclFile{fname};

        BOOST_CHECK(!rft.hasKey("I"));
        BOOST_CHECK(!rft.hasKey("S"));
        BOOST_CHECK(!rft.hasKey("Z"));

        BOOST_CHECK(rft.hasKey("I2"));
        BOOST_CHECK(rft.hasKey("S2"));
        BOOST_CHECK(rft.hasKey("Z2"));

        {
            const auto vectors        = rft.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I2", Opm::EclIO::eclArrType::INTE, 3},
                Opm::EclIO::EclFile::EclEntry{"S2", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"Z2", Opm::EclIO::eclArrType::CHAR, 3},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rft.loadData();

        {
            const auto& I = rft.get<int>("I2");
            const auto  expect_I = std::vector<int>{ 11, 22, 33 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& S = rft.get<float>("S2");
            const auto  expect_S = std::vector<float>{
                2.71f, 828.1f, 8.218f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& Z = rft.get<std::string>("Z2");
            const auto  expect_Z = std::vector<std::string>{
                "Good B", " ye", "W0rlD"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(), expect_Z.end());
        }
    }
}

BOOST_AUTO_TEST_CASE(Unformatted_Existing)
{
    using Char8 = ::Opm::EclIO::PaddedOutputString<8>;

    const auto rset  = RSet("CASE");
    const auto fmt   = ::Opm::EclIO::OutputStream::Formatted{ false };
    const auto exist = ::Opm::EclIO::OutputStream::RFT::OpenExisting{ true };

    {
        auto rft = ::Opm::EclIO::OutputStream::RFT {
            rset, fmt, exist
        };

        rft.write("I", std::vector<int>   {1, 7, 2, 9});
        rft.write("S", std::vector<float> {3.1f, 4.1f, 59.265f});
        rft.write("Z", std::vector<Char8> {
            Char8{"  Hello "}, Char8{" World "}
        });
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "RFT");

        auto rft = ::Opm::EclIO::EclFile{fname};

        BOOST_CHECK(rft.hasKey("I"));
        BOOST_CHECK(rft.hasKey("S"));
        BOOST_CHECK(rft.hasKey("Z"));

        BOOST_CHECK(!rft.hasKey("C"));

        {
            const auto vectors        = rft.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rft.loadData();

        {
            const auto& I = rft.get<int>("I");
            const auto  expect_I = std::vector<int>{ 1, 7, 2, 9 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& S = rft.get<float>("S");
            const auto  expect_S = std::vector<float>{
                3.1f, 4.1f, 59.265f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& Z = rft.get<std::string>("Z");
            const auto  expect_Z = std::vector<std::string>{
                "  Hello", " World" // Trailing blanks trimmed
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(), expect_Z.end());
        }
    }

    {
        auto rft = ::Opm::EclIO::OutputStream::RFT {
            rset, fmt, exist
        };

        rft.write("I2", std::vector<int>   {11, 22, 33});
        rft.write("S2", std::vector<float> {2.71f, 828.1f, 8.218f});
        rft.write("Z2", std::vector<Char8> {
            Char8{"Good B"}, Char8{" ye"}, Char8{ "W0rlD" },
        });
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "RFT");

        auto rft = ::Opm::EclIO::EclFile{fname};

        BOOST_CHECK(rft.hasKey("I"));
        BOOST_CHECK(rft.hasKey("S"));
        BOOST_CHECK(rft.hasKey("Z"));
        BOOST_CHECK(rft.hasKey("I2"));
        BOOST_CHECK(rft.hasKey("S2"));
        BOOST_CHECK(rft.hasKey("Z2"));

        BOOST_CHECK(!rft.hasKey("C"));

        {
            const auto vectors        = rft.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
                Opm::EclIO::EclFile::EclEntry{"I2", Opm::EclIO::eclArrType::INTE, 3},
                Opm::EclIO::EclFile::EclEntry{"S2", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"Z2", Opm::EclIO::eclArrType::CHAR, 3},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rft.loadData();

        {
            const auto& I = rft.get<int>("I");
            const auto  expect_I = std::vector<int>{ 1, 7, 2, 9 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& S = rft.get<float>("S");
            const auto  expect_S = std::vector<float>{
                3.1f, 4.1f, 59.265f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& Z = rft.get<std::string>("Z");
            const auto  expect_Z = std::vector<std::string>{
                "  Hello", " World" // Trailing blanks trimmed
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(), expect_Z.end());
        }
        {
            const auto& I = rft.get<int>("I2");
            const auto  expect_I = std::vector<int>{ 11, 22, 33 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& S = rft.get<float>("S2");
            const auto  expect_S = std::vector<float>{
                2.71f, 828.1f, 8.218f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& Z = rft.get<std::string>("Z2");
            const auto  expect_Z = std::vector<std::string>{
                "Good B", " ye", "W0rlD"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(), expect_Z.end());
        }
    }
}

BOOST_AUTO_TEST_CASE(Formatted_New)
{
    using Char8 = ::Opm::EclIO::PaddedOutputString<8>;

    const auto rset  = RSet("CASE");
    const auto fmt   = ::Opm::EclIO::OutputStream::Formatted{ true };
    const auto exist = ::Opm::EclIO::OutputStream::RFT::OpenExisting{ false };

    {
        auto rft = ::Opm::EclIO::OutputStream::RFT {
            rset, fmt, exist
        };

        rft.write("I", std::vector<int>   {1, 7, 2, 9});
        rft.write("S", std::vector<float> {3.1f, 4.1f, 59.265f});
        rft.write("Z", std::vector<Char8> {
            Char8{"  Hello "}, Char8{" World "}
        });
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "FRFT");

        auto rft = ::Opm::EclIO::EclFile{fname};

        BOOST_CHECK(rft.hasKey("I"));
        BOOST_CHECK(rft.hasKey("S"));
        BOOST_CHECK(rft.hasKey("Z"));
        BOOST_CHECK(!rft.hasKey("C"));

        {
            const auto vectors        = rft.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rft.loadData();

        {
            const auto& I = rft.get<int>("I");
            const auto  expect_I = std::vector<int>{ 1, 7, 2, 9 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& S = rft.get<float>("S");
            const auto  expect_S = std::vector<float>{
                3.1f, 4.1f, 59.265f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& Z = rft.get<std::string>("Z");
            const auto  expect_Z = std::vector<std::string>{
                "  Hello", " World" // Trailing blanks trimmed
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(), expect_Z.end());
        }
    }

    {
        auto rft = ::Opm::EclIO::OutputStream::RFT {
            rset, fmt, exist
        };

        rft.write("I2", std::vector<int>   {11, 22, 33});
        rft.write("S2", std::vector<float> {2.71f, 828.1f, 8.218f});
        rft.write("Z2", std::vector<Char8> {
            Char8{"Good B"}, Char8{" ye"}, Char8{ "W0rlD" },
        });
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "FRFT");

        auto rft = ::Opm::EclIO::EclFile{fname};

        BOOST_CHECK(!rft.hasKey("I"));
        BOOST_CHECK(!rft.hasKey("S"));
        BOOST_CHECK(!rft.hasKey("Z"));

        BOOST_CHECK(rft.hasKey("I2"));
        BOOST_CHECK(rft.hasKey("S2"));
        BOOST_CHECK(rft.hasKey("Z2"));

        {
            const auto vectors        = rft.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I2", Opm::EclIO::eclArrType::INTE, 3},
                Opm::EclIO::EclFile::EclEntry{"S2", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"Z2", Opm::EclIO::eclArrType::CHAR, 3},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rft.loadData();

        {
            const auto& I = rft.get<int>("I2");
            const auto  expect_I = std::vector<int>{ 11, 22, 33 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& S = rft.get<float>("S2");
            const auto  expect_S = std::vector<float>{
                2.71f, 828.1f, 8.218f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& Z = rft.get<std::string>("Z2");
            const auto  expect_Z = std::vector<std::string>{
                "Good B", " ye", "W0rlD"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(), expect_Z.end());
        }
    }
}

BOOST_AUTO_TEST_CASE(Formatted_Existing)
{
    using Char8 = ::Opm::EclIO::PaddedOutputString<8>;

    const auto rset  = RSet("CASE");
    const auto fmt   = ::Opm::EclIO::OutputStream::Formatted{ true };
    const auto exist = ::Opm::EclIO::OutputStream::RFT::OpenExisting{ true };

    {
        auto rft = ::Opm::EclIO::OutputStream::RFT {
            rset, fmt, exist
        };

        rft.write("I", std::vector<int>   {1, 7, 2, 9});
        rft.write("S", std::vector<float> {3.1f, 4.1f, 59.265f});
        rft.write("Z", std::vector<Char8> {
            Char8{"  Hello "}, Char8{" World "}
        });
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "FRFT");

        auto rft = ::Opm::EclIO::EclFile{fname};

        BOOST_CHECK(rft.hasKey("I"));
        BOOST_CHECK(rft.hasKey("S"));
        BOOST_CHECK(rft.hasKey("Z"));

        BOOST_CHECK(!rft.hasKey("C"));

        {
            const auto vectors        = rft.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rft.loadData();

        {
            const auto& I = rft.get<int>("I");
            const auto  expect_I = std::vector<int>{ 1, 7, 2, 9 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& S = rft.get<float>("S");
            const auto  expect_S = std::vector<float>{
                3.1f, 4.1f, 59.265f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& Z = rft.get<std::string>("Z");
            const auto  expect_Z = std::vector<std::string>{
                "  Hello", " World" // Trailing blanks trimmed
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(), expect_Z.end());
        }
    }

    {
        auto rft = ::Opm::EclIO::OutputStream::RFT {
            rset, fmt, exist
        };

        rft.write("I2", std::vector<int>   {11, 22, 33});
        rft.write("S2", std::vector<float> {2.71f, 828.1f, 8.218f});
        rft.write("Z2", std::vector<Char8> {
            Char8{"Good B"}, Char8{" ye"}, Char8{ "W0rlD" },
        });
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "FRFT");

        auto rft = ::Opm::EclIO::EclFile{fname};

        BOOST_CHECK(rft.hasKey("I"));
        BOOST_CHECK(rft.hasKey("S"));
        BOOST_CHECK(rft.hasKey("Z"));
        BOOST_CHECK(rft.hasKey("I2"));
        BOOST_CHECK(rft.hasKey("S2"));
        BOOST_CHECK(rft.hasKey("Z2"));

        BOOST_CHECK(!rft.hasKey("C"));

        {
            const auto vectors        = rft.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"I", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"S", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"Z", Opm::EclIO::eclArrType::CHAR, 2},
                Opm::EclIO::EclFile::EclEntry{"I2", Opm::EclIO::eclArrType::INTE, 3},
                Opm::EclIO::EclFile::EclEntry{"S2", Opm::EclIO::eclArrType::REAL, 3},
                Opm::EclIO::EclFile::EclEntry{"Z2", Opm::EclIO::eclArrType::CHAR, 3},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        rft.loadData();

        {
            const auto& I = rft.get<int>("I");
            const auto  expect_I = std::vector<int>{ 1, 7, 2, 9 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& S = rft.get<float>("S");
            const auto  expect_S = std::vector<float>{
                3.1f, 4.1f, 59.265f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& Z = rft.get<std::string>("Z");
            const auto  expect_Z = std::vector<std::string>{
                "  Hello", " World" // Trailing blanks trimmed
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(), expect_Z.end());
        }
        {
            const auto& I = rft.get<int>("I2");
            const auto  expect_I = std::vector<int>{ 11, 22, 33 };
            BOOST_CHECK_EQUAL_COLLECTIONS(I.begin(), I.end(),
                                          expect_I.begin(),
                                          expect_I.end());
        }

        {
            const auto& S = rft.get<float>("S2");
            const auto  expect_S = std::vector<float>{
                2.71f, 828.1f, 8.218f,
            };

            check_is_close(S, expect_S);
        }

        {
            const auto& Z = rft.get<std::string>("Z2");
            const auto  expect_Z = std::vector<std::string>{
                "Good B", " ye", "W0rlD"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(Z.begin(), Z.end(),
                                          expect_Z.begin(), expect_Z.end());
        }
    }
}

BOOST_AUTO_TEST_SUITE_END() // Class_RFT

// ==========================================================================

BOOST_AUTO_TEST_SUITE(Class_SummarySpecification)

namespace {
    std::time_t advance(const std::time_t tp, const double sec)
    {
        using namespace std::chrono;

        using TP      = time_point<system_clock>;
        using DoubSec = duration<double, seconds::period>;

        const auto t = system_clock::from_time_t(tp) +
            duration_cast<TP::duration>(DoubSec(sec));

        return system_clock::to_time_t(t);
    }

    std::time_t makeUTCTime(const std::tm& timePoint)
    {
        auto       tp    =  timePoint; // Mutable copy.
        const auto ltime =  std::mktime(&tp);
        auto       tmval = *std::gmtime(&ltime); // Mutable.

        // offset =  ltime - tmval
        //        == #seconds by which 'ltime' is AHEAD of tmval.
        const auto offset =
            std::difftime(ltime, std::mktime(&tmval));

        // Advance 'ltime' by 'offset' so that std::gmtime(return value) will
        // have the same broken-down elements as 'tp'.
        return advance(ltime, offset);
    }

    std::string noWGName()
    {
        return ":+:+:+:+";
    }

    int noNum() { return 0; }

    Opm::EclIO::OutputStream::SummarySpecification::StartTime
    start(const int year, const int month, const int day,
          const int hour, const int minute, const int second)
    {
        auto timepoint = std::tm {};

        timepoint.tm_sec  = second;
        timepoint.tm_min  = minute;
        timepoint.tm_hour = hour;
        timepoint.tm_mday = day;
        timepoint.tm_mon  = month - 1;
        timepoint.tm_year = year - 1900;

        return Opm::TimeService::from_time_t(makeUTCTime(timepoint));
    }

    Opm::EclIO::OutputStream::SummarySpecification::RestartSpecification
    noRestart()
    {
        return { "", -1 };
    }

    Opm::EclIO::OutputStream::SummarySpecification::RestartSpecification
    restartedSimulation()
    {
        //       28 characters = 3x8 + 4
        return { "BASE-RUN-WITH-LONG-CASE-NAME", 123 };
    }

    Opm::EclIO::OutputStream::SummarySpecification::RestartSpecification
    restartedSimulationTooLongBasename()
    {
        return { std::string(73, 'X'), 123 };
    }

    Opm::EclIO::OutputStream::SummarySpecification::Parameters
    summaryParameters()
    {
        auto prm = Opm::EclIO::OutputStream::
            SummarySpecification::Parameters{};

        prm.add("TIME", noWGName(), noNum(), "DAYS");
        prm.add("WBHP", "PROD01", noNum(), "BARSA");
        prm.add("GGOR", "N-PROD", noNum(), "SM3/SM3");
        prm.add("BGSAT", noWGName(), 523, "");

        return prm;
    }
} // Anonymous

BOOST_AUTO_TEST_CASE(Unformatted_Base)
{
    using SMSpec = ::Opm::EclIO::OutputStream::SummarySpecification;

    const auto rset = RSet("CASE");
    const auto fmt  = ::Opm::EclIO::OutputStream::Formatted{ false };
    const auto cartDims = std::array<int,3>{ 46, 112, 22 }; // Norne dimensions

    {
        using UConv = SMSpec::UnitConvention;

        // Invalid unit convention
        const auto uconv = static_cast<UConv>(1729);

        BOOST_CHECK_THROW(SMSpec(rset, fmt, uconv, cartDims, noRestart(),
                                 start(2019, 10, 1, 12, 34, 56)),
                          std::invalid_argument);
    }

    // ========================= METRIC =======================
    {
        const auto uconv = SMSpec::UnitConvention::Metric;

        auto smspec = SMSpec {
            rset, fmt, uconv, cartDims, noRestart(),
            start(2019, 10, 1, 12, 34, 56)
        };

        smspec.write(summaryParameters());
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "SMSPEC");

        auto smspec = ::Opm::EclIO::EclFile{fname};

        //BOOST_CHECK_MESSAGE(! smspec.hasKey("RESTART"), "SMSPEC File must NOT have 'RESTART'");

        {
            const auto vectors        = smspec.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"INTEHEAD", Opm::EclIO::eclArrType::INTE, 2},
                Opm::EclIO::EclFile::EclEntry{"RESTART", Opm::EclIO::eclArrType::CHAR, 9},
                Opm::EclIO::EclFile::EclEntry{"DIMENS", Opm::EclIO::eclArrType::INTE, 6},
                Opm::EclIO::EclFile::EclEntry{"KEYWORDS", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"WGNAMES", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"NUMS", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"UNITS", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"STARTDAT", Opm::EclIO::eclArrType::INTE, 6},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        smspec.loadData();

        {
            const auto& Ih = smspec.get<int>("INTEHEAD");
            const auto  expect = std::vector<int>{ 1, 100 };
            BOOST_CHECK_EQUAL_COLLECTIONS(Ih.begin(), Ih.end(),
                                          expect.begin(),
                                          expect.end());
        }

        {
            const auto& D = smspec.get<int>("DIMENS");
            const auto  expect = std::vector<int> {
                4, 46, 112, 22, 0, -1
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(D.begin(), D.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& K = smspec.get<std::string>("KEYWORDS");
            const auto  expect = std::vector<std::string> {
                "TIME", "WBHP", "GGOR", "BGSAT"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(K.begin(), K.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& W = smspec.get<std::string>("WGNAMES");
            const auto  expect = std::vector<std::string> {
                ":+:+:+:+", "PROD01", "N-PROD", ":+:+:+:+"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(W.begin(), W.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& N = smspec.get<int>("NUMS");
            const auto  expect = std::vector<int> { 0, 0, 0, 523 };

            BOOST_CHECK_EQUAL_COLLECTIONS(N.begin(), N.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& U = smspec.get<std::string>("UNITS");
            const auto  expect = std::vector<std::string> {
                "DAYS", "BARSA", "SM3/SM3", ""
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(U.begin(), U.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& S = smspec.get<int>("STARTDAT");
            const auto  expect = std::vector<int> {
                1, 10, 2019, 12, 34,
                56 * 1000 * 1000
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(S.begin(), S.end(),
                                          expect.begin(), expect.end());
        }
    }

    // ========================= FIELD =======================
    {
        const auto uconv = SMSpec::UnitConvention::Field;

        auto smspec = SMSpec {
            rset, fmt, uconv, cartDims, noRestart(),
            start(1970, 1, 1, 0, 0, 0)
        };

        smspec.write(summaryParameters());
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "SMSPEC");

        auto smspec = ::Opm::EclIO::EclFile{fname};

        {
            const auto vectors        = smspec.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"INTEHEAD", Opm::EclIO::eclArrType::INTE, 2},
                Opm::EclIO::EclFile::EclEntry{"RESTART", Opm::EclIO::eclArrType::CHAR, 9},
                Opm::EclIO::EclFile::EclEntry{"DIMENS", Opm::EclIO::eclArrType::INTE, 6},
                Opm::EclIO::EclFile::EclEntry{"KEYWORDS", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"WGNAMES", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"NUMS", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"UNITS", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"STARTDAT", Opm::EclIO::eclArrType::INTE, 6},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        smspec.loadData();

        {
            const auto& Ih = smspec.get<int>("INTEHEAD");
            const auto  expect = std::vector<int>{ 2, 100 };
            BOOST_CHECK_EQUAL_COLLECTIONS(Ih.begin(), Ih.end(),
                                          expect.begin(),
                                          expect.end());
        }

        {
            const auto& D = smspec.get<int>("DIMENS");
            const auto  expect = std::vector<int> {
                4, 46, 112, 22, 0, -1
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(D.begin(), D.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& K = smspec.get<std::string>("KEYWORDS");
            const auto  expect = std::vector<std::string> {
                "TIME", "WBHP", "GGOR", "BGSAT"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(K.begin(), K.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& W = smspec.get<std::string>("WGNAMES");
            const auto  expect = std::vector<std::string> {
                ":+:+:+:+", "PROD01", "N-PROD", ":+:+:+:+"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(W.begin(), W.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& N = smspec.get<int>("NUMS");
            const auto  expect = std::vector<int> { 0, 0, 0, 523 };

            BOOST_CHECK_EQUAL_COLLECTIONS(N.begin(), N.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& U = smspec.get<std::string>("UNITS");
            const auto  expect = std::vector<std::string> {
                //       (!)      (!)
                "DAYS", "BARSA", "SM3/SM3", ""
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(U.begin(), U.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& S = smspec.get<int>("STARTDAT");
            const auto  expect = std::vector<int> {
                1, 1, 1970, 0, 0, 0
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(S.begin(), S.end(),
                                          expect.begin(), expect.end());
        }
    }

    // ========================= LAB =======================
    {
        const auto uconv = SMSpec::UnitConvention::Lab;

        auto smspec = SMSpec {
            rset, fmt, uconv, cartDims, noRestart(),
            start(2018, 12, 24, 17, 0, 0)
        };

        smspec.write(summaryParameters());
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "SMSPEC");

        auto smspec = ::Opm::EclIO::EclFile{fname};

        {
            const auto vectors        = smspec.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"INTEHEAD", Opm::EclIO::eclArrType::INTE, 2},
                Opm::EclIO::EclFile::EclEntry{"RESTART", Opm::EclIO::eclArrType::CHAR, 9},
                Opm::EclIO::EclFile::EclEntry{"DIMENS", Opm::EclIO::eclArrType::INTE, 6},
                Opm::EclIO::EclFile::EclEntry{"KEYWORDS", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"WGNAMES", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"NUMS", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"UNITS", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"STARTDAT", Opm::EclIO::eclArrType::INTE, 6},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        smspec.loadData();

        {
            const auto& Ih = smspec.get<int>("INTEHEAD");
            const auto  expect = std::vector<int>{ 3, 100 };
            BOOST_CHECK_EQUAL_COLLECTIONS(Ih.begin(), Ih.end(),
                                          expect.begin(),
                                          expect.end());
        }

        {
            const auto& D = smspec.get<int>("DIMENS");
            const auto  expect = std::vector<int> {
                4, 46, 112, 22, 0, -1
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(D.begin(), D.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& K = smspec.get<std::string>("KEYWORDS");
            const auto  expect = std::vector<std::string> {
                "TIME", "WBHP", "GGOR", "BGSAT"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(K.begin(), K.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& W = smspec.get<std::string>("WGNAMES");
            const auto  expect = std::vector<std::string> {
                ":+:+:+:+", "PROD01", "N-PROD", ":+:+:+:+"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(W.begin(), W.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& N = smspec.get<int>("NUMS");
            const auto  expect = std::vector<int> { 0, 0, 0, 523 };

            BOOST_CHECK_EQUAL_COLLECTIONS(N.begin(), N.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& U = smspec.get<std::string>("UNITS");
            const auto  expect = std::vector<std::string> {
                //       (!)      (!)
                "DAYS", "BARSA", "SM3/SM3", ""
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(U.begin(), U.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& S = smspec.get<int>("STARTDAT");
            const auto  expect = std::vector<int> {
                24, 12, 2018, 17, 0, 0
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(S.begin(), S.end(),
                                          expect.begin(), expect.end());
        }
    }

    // ========================= PVT-M =======================
    {
        const auto uconv = SMSpec::UnitConvention::Pvt_M;

        auto smspec = SMSpec {
            rset, fmt, uconv, cartDims, noRestart(),
            start(1983, 1, 1, 1, 2, 3)
        };

        smspec.write(summaryParameters());
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "SMSPEC");

        auto smspec = ::Opm::EclIO::EclFile{fname};

        {
            const auto vectors        = smspec.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"INTEHEAD", Opm::EclIO::eclArrType::INTE, 2},
                Opm::EclIO::EclFile::EclEntry{"RESTART", Opm::EclIO::eclArrType::CHAR, 9},
                Opm::EclIO::EclFile::EclEntry{"DIMENS", Opm::EclIO::eclArrType::INTE, 6},
                Opm::EclIO::EclFile::EclEntry{"KEYWORDS", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"WGNAMES", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"NUMS", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"UNITS", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"STARTDAT", Opm::EclIO::eclArrType::INTE, 6},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        smspec.loadData();

        {
            const auto& Ih = smspec.get<int>("INTEHEAD");
            const auto  expect = std::vector<int>{ 4, 100 };
            BOOST_CHECK_EQUAL_COLLECTIONS(Ih.begin(), Ih.end(),
                                          expect.begin(),
                                          expect.end());
        }

        {
            const auto& D = smspec.get<int>("DIMENS");
            const auto  expect = std::vector<int> {
                4, 46, 112, 22, 0, -1
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(D.begin(), D.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& K = smspec.get<std::string>("KEYWORDS");
            const auto  expect = std::vector<std::string> {
                "TIME", "WBHP", "GGOR", "BGSAT"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(K.begin(), K.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& W = smspec.get<std::string>("WGNAMES");
            const auto  expect = std::vector<std::string> {
                ":+:+:+:+", "PROD01", "N-PROD", ":+:+:+:+"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(W.begin(), W.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& N = smspec.get<int>("NUMS");
            const auto  expect = std::vector<int> { 0, 0, 0, 523 };

            BOOST_CHECK_EQUAL_COLLECTIONS(N.begin(), N.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& U = smspec.get<std::string>("UNITS");
            const auto  expect = std::vector<std::string> {
                //       (!)      (!)
                "DAYS", "BARSA", "SM3/SM3", ""
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(U.begin(), U.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& S = smspec.get<int>("STARTDAT");
            const auto  expect = std::vector<int> {
                1, 1, 1983, 1, 2, 3 * 1000 * 1000
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(S.begin(), S.end(),
                                          expect.begin(), expect.end());
        }
    }
}

BOOST_AUTO_TEST_CASE(Formatted_Restarted)
{
    using SMSpec = ::Opm::EclIO::OutputStream::SummarySpecification;

    const auto rset = RSet("CASE");
    const auto fmt  = ::Opm::EclIO::OutputStream::Formatted{ true };
    const auto cartDims = std::array<int,3>{ 46, 112, 22 }; // Norne dimensions

    // === Restart root name too long =========================
    {
        using UConv = SMSpec::UnitConvention;

        auto smspec = SMSpec {
            rset, fmt, UConv::Pvt_M, cartDims,
            restartedSimulationTooLongBasename(),
            start(2019, 10, 1, 12, 34, 56)
        };

        // Should *NOT* write RESTART vector (name too long).
        smspec.write(summaryParameters());
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "FSMSPEC");

        auto smspec = ::Opm::EclIO::EclFile{fname};

#if 0
        BOOST_CHECK_MESSAGE(! smspec.hasKey("RESTART"),
                            "SMSPEC file must NOT have RESTART "
                            "data if root name is too long");
#endif
    }

    // ========================= METRIC =======================
    {
        const auto uconv = SMSpec::UnitConvention::Metric;

        auto smspec = SMSpec {
            rset, fmt, uconv, cartDims, restartedSimulation(),
            start(2019, 10, 1, 12, 34, 56)
        };

        smspec.write(summaryParameters());
    }

    {
        const auto fname = ::Opm::EclIO::OutputStream::
            outputFileName(rset, "FSMSPEC");

        auto smspec = ::Opm::EclIO::EclFile{fname};

        {
            const auto vectors        = smspec.getList();
            const auto expect_vectors = std::vector<Opm::EclIO::EclFile::EclEntry>{
                Opm::EclIO::EclFile::EclEntry{"INTEHEAD", Opm::EclIO::eclArrType::INTE, 2},
                Opm::EclIO::EclFile::EclEntry{"RESTART", Opm::EclIO::eclArrType::CHAR, 9},
                Opm::EclIO::EclFile::EclEntry{"DIMENS", Opm::EclIO::eclArrType::INTE, 6},
                Opm::EclIO::EclFile::EclEntry{"KEYWORDS", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"WGNAMES", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"NUMS", Opm::EclIO::eclArrType::INTE, 4},
                Opm::EclIO::EclFile::EclEntry{"UNITS", Opm::EclIO::eclArrType::CHAR, 4},
                Opm::EclIO::EclFile::EclEntry{"STARTDAT", Opm::EclIO::eclArrType::INTE, 6},
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(vectors.begin(), vectors.end(),
                                          expect_vectors.begin(),
                                          expect_vectors.end());
        }

        smspec.loadData();

        {
            const auto& Ih = smspec.get<int>("INTEHEAD");
            const auto  expect = std::vector<int>{ 1, 100 };
            BOOST_CHECK_EQUAL_COLLECTIONS(Ih.begin(), Ih.end(),
                                          expect.begin(),
                                          expect.end());
        }

        {
            const auto& R = smspec.get<std::string>("RESTART");
            const auto  expect = std::vector<std::string> {
                "BASE-RUN", "-WITH-LO", "NG-CASE-",  // 0 .. 2
                "NAME"    , ""        , ""        ,  // 3 .. 5
                ""        , ""        , ""        ,  // 6 .. 8
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(R.begin(), R.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& D = smspec.get<int>("DIMENS");
            const auto  expect = std::vector<int> {
                4, 46, 112, 22, 0, 123
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(D.begin(), D.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& K = smspec.get<std::string>("KEYWORDS");
            const auto  expect = std::vector<std::string> {
                "TIME", "WBHP", "GGOR", "BGSAT"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(K.begin(), K.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& W = smspec.get<std::string>("WGNAMES");
            const auto  expect = std::vector<std::string> {
                ":+:+:+:+", "PROD01", "N-PROD", ":+:+:+:+"
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(W.begin(), W.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& N = smspec.get<int>("NUMS");
            const auto  expect = std::vector<int> { 0, 0, 0, 523 };

            BOOST_CHECK_EQUAL_COLLECTIONS(N.begin(), N.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& U = smspec.get<std::string>("UNITS");
            const auto  expect = std::vector<std::string> {
                "DAYS", "BARSA", "SM3/SM3", ""
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(U.begin(), U.end(),
                                          expect.begin(), expect.end());
        }

        {
            const auto& S = smspec.get<int>("STARTDAT");
            const auto  expect = std::vector<int> {
                1, 10, 2019, 12, 34,
                56 * 1000 * 1000
            };

            BOOST_CHECK_EQUAL_COLLECTIONS(S.begin(), S.end(),
                                          expect.begin(), expect.end());
        }
    }
}

BOOST_AUTO_TEST_SUITE_END() // Class_SummarySpecification
