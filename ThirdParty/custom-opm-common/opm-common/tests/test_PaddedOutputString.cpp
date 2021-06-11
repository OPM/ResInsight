#define BOOST_TEST_MODULE Padded_Output_String

#include <boost/test/unit_test.hpp>

#include <opm/io/eclipse/PaddedOutputString.hpp>
#include <opm/common/utility/String.hpp>

// Convenience alias.
template <std::size_t N>
using PadString = ::Opm::EclIO::PaddedOutputString<N>;

// =====================================================================

BOOST_AUTO_TEST_SUITE(PadString8)

BOOST_AUTO_TEST_CASE (Basic_Operations)
{
    // Default Constructor
    {
        const auto s = PadString<8>{};

        BOOST_CHECK_EQUAL(s.c_str(), std::string(8, ' '));
    }

    // Construct from Constant String
    {
        const auto s = PadString<8>{"Inj-1"};

        BOOST_CHECK_EQUAL(s.c_str(), std::string{"Inj-1   "});
    }

    // Copy Construction
    {
        const auto s1 = PadString<8>{"Inj-1"};
        const auto s2 = s1;

        BOOST_CHECK_EQUAL(s2.c_str(), std::string{"Inj-1   "});
    }

    // Move Construction
    {
        auto s1 = PadString<8>{"Inj-1"};
        const auto s2 = std::move(s1);

        BOOST_CHECK_EQUAL(s2.c_str(), std::string{"Inj-1   "});
    }

    // Assignment Operator
    {
        const auto s1 = PadString<8>{"Inj-1"};
        auto s2 = PadString<8>{"Prod-2"};

        s2 = s1;
        BOOST_CHECK_EQUAL(s2.c_str(), std::string{"Inj-1   "});
    }

    // Move Assignment Operator
    {
        auto s1 = PadString<8>{"Inj-1"};
        auto s2 = PadString<8>{"Prod-2"};

        s2 = std::move(s1);
        BOOST_CHECK_EQUAL(s2.c_str(), std::string{"Inj-1   "});
    }

    // Assign std::string
    {
        auto s = PadString<8>{"@Hi Hoo@"};

        s = "Prod-2";
        BOOST_CHECK_EQUAL(s.c_str(), std::string{"Prod-2  "});
    }
}

BOOST_AUTO_TEST_CASE (String_Shortening)
{
    // Construct from string of more than N characters
    {
        const auto s = PadString<10>{
            "String too long"
        };

        BOOST_CHECK_EQUAL(s.c_str(), std::string{"String too"});
    }

    // Assign string of more than N characters
    {
        auto s = PadString<11>{};

        s = "This string has too many characters";

        BOOST_CHECK_EQUAL(s.c_str(), std::string{"This string"});
    }
}

BOOST_AUTO_TEST_CASE (Trim) {
    const auto s1 = PadString<4>{"X"};
    BOOST_CHECK_EQUAL(Opm::trim_copy(s1), std::string{"X"});

    const auto s2 = PadString<4>{"ABCD"};
    BOOST_CHECK_EQUAL(Opm::trim_copy(s2), std::string{"ABCD"});

    const auto s3 = PadString<4>{""};
    BOOST_CHECK_EQUAL(Opm::trim_copy(s3), std::string{""});
}

BOOST_AUTO_TEST_SUITE_END()
