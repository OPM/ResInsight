
#include "gtest/gtest.h"

#include "cafAppEnum.h"
#include "cafFilePath.h"
#include "cafPdmFieldTraits.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmPointer.h"

#include <QString>

#include <vector>

//==================================================================================================
// Test enum for AppEnum tests
//==================================================================================================

enum class TraitTestEnum
{
    ALPHA,
    BETA,
    GAMMA
};

namespace caf
{
template <>
void AppEnum<TraitTestEnum>::setUp()
{
    addItem( TraitTestEnum::ALPHA, "ALPHA", "Alpha" );
    addItem( TraitTestEnum::BETA, "BETA", "Beta" );
    addItem( TraitTestEnum::GAMMA, "GAMMA", "Gamma" );
    setDefault( TraitTestEnum::ALPHA );
}
} // namespace caf

//==================================================================================================
// Minimal PdmObjectHandle subclass for PdmPointer tests
//==================================================================================================

class TraitTestObject : public caf::PdmObjectHandle
{
};

//==================================================================================================
// pdmToVariant / pdmFromVariant round-trip tests
//==================================================================================================

TEST( PdmFieldTraits, RoundTrip_Int )
{
    int      original = 42;
    QVariant v        = caf::pdmToVariant( original );
    int      out      = 0;
    caf::pdmFromVariant( v, out );
    EXPECT_EQ( original, out );
}

TEST( PdmFieldTraits, RoundTrip_Double )
{
    double   original = 3.14159265358979;
    QVariant v        = caf::pdmToVariant( original );
    double   out      = 0.0;
    caf::pdmFromVariant( v, out );
    EXPECT_DOUBLE_EQ( original, out );
}

TEST( PdmFieldTraits, RoundTrip_Float )
{
    float    original = 2.71828f;
    QVariant v        = caf::pdmToVariant( original );
    float    out      = 0.0f;
    caf::pdmFromVariant( v, out );
    EXPECT_FLOAT_EQ( original, out );
}

TEST( PdmFieldTraits, RoundTrip_Bool )
{
    QVariant vTrue = caf::pdmToVariant( true );
    bool     out   = false;
    caf::pdmFromVariant( vTrue, out );
    EXPECT_TRUE( out );

    QVariant vFalse = caf::pdmToVariant( false );
    caf::pdmFromVariant( vFalse, out );
    EXPECT_FALSE( out );
}

TEST( PdmFieldTraits, RoundTrip_QString )
{
    QString  original = "Hello PDM";
    QVariant v        = caf::pdmToVariant( original );
    QString  out;
    caf::pdmFromVariant( v, out );
    EXPECT_EQ( original, out );
}

TEST( PdmFieldTraits, RoundTrip_FilePath )
{
    caf::FilePath original( "/some/path/to/file.txt" );
    QVariant      v = caf::pdmToVariant( original );

    // Must be stored as a string (not as FilePath metatype)
    EXPECT_EQ( v.toString(), QString( "/some/path/to/file.txt" ) );

    caf::FilePath out;
    caf::pdmFromVariant( v, out );
    EXPECT_EQ( original.path(), out.path() );
}

TEST( PdmFieldTraits, RoundTrip_AppEnum )
{
    caf::AppEnum<TraitTestEnum> original = TraitTestEnum::BETA;
    QVariant                    v        = caf::pdmToVariant( original );

    // Must be stored as int
    EXPECT_EQ( v.toInt(), static_cast<int>( TraitTestEnum::BETA ) );

    caf::AppEnum<TraitTestEnum> out;
    caf::pdmFromVariant( v, out );
    EXPECT_EQ( static_cast<TraitTestEnum>( out ), TraitTestEnum::BETA );
}

TEST( PdmFieldTraits, RoundTrip_Vector_Int )
{
    std::vector<int> original = { 1, 2, 3, 4, 5 };
    QVariant         v        = caf::pdmToVariant( original );

    EXPECT_TRUE( v.canConvert<QList<QVariant>>() );
    EXPECT_EQ( v.toList().size(), 5 );

    std::vector<int> out;
    caf::pdmFromVariant( v, out );
    EXPECT_EQ( original, out );
}

TEST( PdmFieldTraits, RoundTrip_Vector_Double )
{
    std::vector<double> original = { 1.1, 2.2, 3.3 };
    QVariant            v        = caf::pdmToVariant( original );
    std::vector<double> out;
    caf::pdmFromVariant( v, out );
    ASSERT_EQ( original.size(), out.size() );
    for ( size_t i = 0; i < original.size(); ++i )
    {
        EXPECT_DOUBLE_EQ( original[i], out[i] );
    }
}

TEST( PdmFieldTraits, RoundTrip_Vector_Empty )
{
    std::vector<int> original;
    QVariant         v = caf::pdmToVariant( original );

    std::vector<int> out = { 99 }; // pre-fill to confirm it gets cleared
    caf::pdmFromVariant( v, out );
    EXPECT_TRUE( out.empty() );
}

TEST( PdmFieldTraits, RoundTrip_Pair_BoolString )
{
    auto     original = std::make_pair( true, QString( "test" ) );
    QVariant v        = caf::pdmToVariant( original );

    ASSERT_TRUE( v.canConvert<QList<QVariant>>() );
    ASSERT_EQ( v.toList().size(), 2 );

    std::pair<bool, QString> out;
    caf::pdmFromVariant( v, out );
    EXPECT_EQ( original.first, out.first );
    EXPECT_EQ( original.second, out.second );
}

TEST( PdmFieldTraits, RoundTrip_Pair_BoolDouble )
{
    auto     original = std::make_pair( false, 3.14 );
    QVariant v        = caf::pdmToVariant( original );

    std::pair<bool, double> out;
    caf::pdmFromVariant( v, out );
    EXPECT_EQ( original.first, out.first );
    EXPECT_DOUBLE_EQ( original.second, out.second );
}

TEST( PdmFieldTraits, RoundTrip_PdmPointer_Null )
{
    caf::PdmPointer<TraitTestObject> original;
    EXPECT_TRUE( original.isNull() );

    QVariant v = caf::pdmToVariant( original );

    caf::PdmPointer<TraitTestObject> out;
    caf::pdmFromVariant( v, out );
    EXPECT_TRUE( out.isNull() );
}

TEST( PdmFieldTraits, RoundTrip_PdmPointer_NonNull )
{
    auto                             obj = std::make_unique<TraitTestObject>();
    caf::PdmPointer<TraitTestObject> original( obj.get() );

    QVariant v = caf::pdmToVariant( original );

    caf::PdmPointer<TraitTestObject> out;
    caf::pdmFromVariant( v, out );
    EXPECT_EQ( original.rawPtr(), out.rawPtr() );
}

//==================================================================================================
// pdmVariantEqual tests
//==================================================================================================

TEST( PdmFieldTraits, VariantEqual_Int )
{
    QVariant a = caf::pdmToVariant( 42 );
    QVariant b = caf::pdmToVariant( 42 );
    QVariant c = caf::pdmToVariant( 43 );

    EXPECT_TRUE( caf::pdmVariantEqual<int>( a, b ) );
    EXPECT_FALSE( caf::pdmVariantEqual<int>( a, c ) );
}

TEST( PdmFieldTraits, VariantEqual_Double_Exact )
{
    QVariant a = caf::pdmToVariant( 1.0 );
    QVariant b = caf::pdmToVariant( 1.0 );
    EXPECT_TRUE( caf::pdmVariantEqual<double>( a, b ) );
}

TEST( PdmFieldTraits, VariantEqual_Double_WithinEpsilon )
{
    double   base    = 1.0;
    double   epsilon = 1e-8;
    QVariant a       = caf::pdmToVariant( base );
    QVariant b       = caf::pdmToVariant( base + epsilon * 0.5 ); // within epsilon
    QVariant c       = caf::pdmToVariant( base + epsilon * 2.0 ); // outside epsilon

    EXPECT_TRUE( caf::pdmVariantEqual<double>( a, b ) );
    EXPECT_FALSE( caf::pdmVariantEqual<double>( a, c ) );
}

TEST( PdmFieldTraits, VariantEqual_Float_WithinEpsilon )
{
    float    base    = 1.0f;
    float    epsilon = 1e-6f;
    QVariant a       = caf::pdmToVariant( base );
    QVariant b       = caf::pdmToVariant( base + epsilon * 0.5f ); // within epsilon
    QVariant c       = caf::pdmToVariant( base + epsilon * 2.0f ); // outside epsilon

    EXPECT_TRUE( caf::pdmVariantEqual<float>( a, b ) );
    EXPECT_FALSE( caf::pdmVariantEqual<float>( a, c ) );
}

TEST( PdmFieldTraits, VariantEqual_FilePath )
{
    QVariant a = caf::pdmToVariant( caf::FilePath( "/path/to/file.txt" ) );
    QVariant b = caf::pdmToVariant( caf::FilePath( "/path/to/file.txt" ) );
    QVariant c = caf::pdmToVariant( caf::FilePath( "/other/path.txt" ) );

    EXPECT_TRUE( caf::pdmVariantEqual<caf::FilePath>( a, b ) );
    EXPECT_FALSE( caf::pdmVariantEqual<caf::FilePath>( a, c ) );
}

TEST( PdmFieldTraits, VariantEqual_AppEnum )
{
    QVariant a = caf::pdmToVariant( caf::AppEnum<TraitTestEnum>( TraitTestEnum::ALPHA ) );
    QVariant b = caf::pdmToVariant( caf::AppEnum<TraitTestEnum>( TraitTestEnum::ALPHA ) );
    QVariant c = caf::pdmToVariant( caf::AppEnum<TraitTestEnum>( TraitTestEnum::GAMMA ) );

    EXPECT_TRUE( caf::pdmVariantEqual<caf::AppEnum<TraitTestEnum>>( a, b ) );
    EXPECT_FALSE( caf::pdmVariantEqual<caf::AppEnum<TraitTestEnum>>( a, c ) );
}

TEST( PdmFieldTraits, VariantEqual_Vector_Equal )
{
    std::vector<int> v1 = { 1, 2, 3 };
    std::vector<int> v2 = { 1, 2, 3 };
    QVariant         a  = caf::pdmToVariant( v1 );
    QVariant         b  = caf::pdmToVariant( v2 );

    EXPECT_TRUE( caf::pdmVariantEqual<std::vector<int>>( a, b ) );
}

TEST( PdmFieldTraits, VariantEqual_Vector_DifferentSize )
{
    std::vector<int> v1 = { 1, 2, 3 };
    std::vector<int> v2 = { 1, 2 };
    QVariant         a  = caf::pdmToVariant( v1 );
    QVariant         b  = caf::pdmToVariant( v2 );

    EXPECT_FALSE( caf::pdmVariantEqual<std::vector<int>>( a, b ) );
}

TEST( PdmFieldTraits, VariantEqual_Vector_DifferentValues )
{
    std::vector<int> v1 = { 1, 2, 3 };
    std::vector<int> v2 = { 1, 2, 99 };
    QVariant         a  = caf::pdmToVariant( v1 );
    QVariant         b  = caf::pdmToVariant( v2 );

    EXPECT_FALSE( caf::pdmVariantEqual<std::vector<int>>( a, b ) );
}

TEST( PdmFieldTraits, VariantEqual_Vector_Double_UsesEpsilon )
{
    double              epsilon = 1e-8;
    std::vector<double> v1      = { 1.0, 2.0 };
    std::vector<double> v2      = { 1.0 + epsilon * 0.5, 2.0 + epsilon * 0.5 }; // within epsilon
    std::vector<double> v3      = { 1.0 + epsilon * 2.0, 2.0 }; // outside epsilon

    QVariant a = caf::pdmToVariant( v1 );
    QVariant b = caf::pdmToVariant( v2 );
    QVariant c = caf::pdmToVariant( v3 );

    EXPECT_TRUE( caf::pdmVariantEqual<std::vector<double>>( a, b ) );
    EXPECT_FALSE( caf::pdmVariantEqual<std::vector<double>>( a, c ) );
}

TEST( PdmFieldTraits, VariantEqual_Vector_Empty )
{
    std::vector<int> v1;
    std::vector<int> v2;
    QVariant         a = caf::pdmToVariant( v1 );
    QVariant         b = caf::pdmToVariant( v2 );

    EXPECT_TRUE( caf::pdmVariantEqual<std::vector<int>>( a, b ) );
}

TEST( PdmFieldTraits, VariantEqual_Pair_Equal )
{
    auto     p1 = std::make_pair( true, QString( "hello" ) );
    QVariant a  = caf::pdmToVariant( p1 );
    QVariant b  = caf::pdmToVariant( p1 );

    EXPECT_TRUE( ( caf::pdmVariantEqual<std::pair<bool, QString>>( a, b ) ) );
}

TEST( PdmFieldTraits, VariantEqual_Pair_FirstDiffers )
{
    QVariant a = caf::pdmToVariant( std::make_pair( true, QString( "hello" ) ) );
    QVariant b = caf::pdmToVariant( std::make_pair( false, QString( "hello" ) ) );

    EXPECT_FALSE( ( caf::pdmVariantEqual<std::pair<bool, QString>>( a, b ) ) );
}

TEST( PdmFieldTraits, VariantEqual_Pair_SecondDiffers )
{
    QVariant a = caf::pdmToVariant( std::make_pair( true, QString( "hello" ) ) );
    QVariant b = caf::pdmToVariant( std::make_pair( true, QString( "world" ) ) );

    EXPECT_FALSE( ( caf::pdmVariantEqual<std::pair<bool, QString>>( a, b ) ) );
}

TEST( PdmFieldTraits, VariantEqual_Pair_Double_UsesEpsilon )
{
    double   epsilon = 1e-8;
    QVariant a       = caf::pdmToVariant( std::make_pair( true, 1.0 ) );
    QVariant b       = caf::pdmToVariant( std::make_pair( true, 1.0 + epsilon * 0.5 ) ); // within
    QVariant c       = caf::pdmToVariant( std::make_pair( true, 1.0 + epsilon * 2.0 ) ); // outside

    EXPECT_TRUE( ( caf::pdmVariantEqual<std::pair<bool, double>>( a, b ) ) );
    EXPECT_FALSE( ( caf::pdmVariantEqual<std::pair<bool, double>>( a, c ) ) );
}

TEST( PdmFieldTraits, VariantEqual_PdmPointer_BothNull )
{
    caf::PdmPointer<TraitTestObject> p1;
    caf::PdmPointer<TraitTestObject> p2;
    QVariant                         a = caf::pdmToVariant( p1 );
    QVariant                         b = caf::pdmToVariant( p2 );

    EXPECT_TRUE( ( caf::pdmVariantEqual<caf::PdmPointer<TraitTestObject>>( a, b ) ) );
}

TEST( PdmFieldTraits, VariantEqual_PdmPointer_NullVsNonNull )
{
    auto                             obj = std::make_unique<TraitTestObject>();
    caf::PdmPointer<TraitTestObject> p1( obj.get() );
    caf::PdmPointer<TraitTestObject> p2;
    QVariant                         a = caf::pdmToVariant( p1 );
    QVariant                         b = caf::pdmToVariant( p2 );

    EXPECT_FALSE( ( caf::pdmVariantEqual<caf::PdmPointer<TraitTestObject>>( a, b ) ) );
}

TEST( PdmFieldTraits, VariantEqual_PdmPointer_SameObject )
{
    auto                             obj = std::make_unique<TraitTestObject>();
    caf::PdmPointer<TraitTestObject> p1( obj.get() );
    caf::PdmPointer<TraitTestObject> p2( obj.get() );
    QVariant                         a = caf::pdmToVariant( p1 );
    QVariant                         b = caf::pdmToVariant( p2 );

    EXPECT_TRUE( ( caf::pdmVariantEqual<caf::PdmPointer<TraitTestObject>>( a, b ) ) );
}

TEST( PdmFieldTraits, VariantEqual_PdmPointer_DifferentObjects )
{
    auto                             obj1 = std::make_unique<TraitTestObject>();
    auto                             obj2 = std::make_unique<TraitTestObject>();
    caf::PdmPointer<TraitTestObject> p1( obj1.get() );
    caf::PdmPointer<TraitTestObject> p2( obj2.get() );
    QVariant                         a = caf::pdmToVariant( p1 );
    QVariant                         b = caf::pdmToVariant( p2 );

    EXPECT_FALSE( ( caf::pdmVariantEqual<caf::PdmPointer<TraitTestObject>>( a, b ) ) );
}

// ------------------------------------------------------------------------------------------------
// std::optional round-trip tests
// ------------------------------------------------------------------------------------------------

TEST( PdmFieldTraits, RoundTrip_Optional_Double_HasValue )
{
    std::optional<double> orig = 3.14;
    QVariant              v    = caf::pdmToVariant( orig );

    std::optional<double> decoded;
    caf::pdmFromVariant( v, decoded );

    ASSERT_TRUE( decoded.has_value() );
    EXPECT_DOUBLE_EQ( *orig, *decoded );
}

TEST( PdmFieldTraits, RoundTrip_Optional_Double_Nullopt )
{
    std::optional<double> orig;
    QVariant              v = caf::pdmToVariant( orig );

    std::optional<double> decoded = 99.0;
    caf::pdmFromVariant( v, decoded );

    EXPECT_FALSE( decoded.has_value() );
}

TEST( PdmFieldTraits, RoundTrip_Optional_QString_HasValue )
{
    std::optional<QString> orig = QString( "hello" );
    QVariant               v    = caf::pdmToVariant( orig );

    std::optional<QString> decoded;
    caf::pdmFromVariant( v, decoded );

    ASSERT_TRUE( decoded.has_value() );
    EXPECT_EQ( *orig, *decoded );
}

// ------------------------------------------------------------------------------------------------
// std::optional equality tests
// ------------------------------------------------------------------------------------------------

TEST( PdmFieldTraits, VariantEqual_Optional_BothNullopt )
{
    std::optional<double> a;
    std::optional<double> b;
    QVariant              va = caf::pdmToVariant( a );
    QVariant              vb = caf::pdmToVariant( b );

    EXPECT_TRUE( ( caf::pdmVariantEqual<std::optional<double>>( va, vb ) ) );
}

TEST( PdmFieldTraits, VariantEqual_Optional_NulloptVsValue )
{
    std::optional<double> a;
    std::optional<double> b  = 1.0;
    QVariant              va = caf::pdmToVariant( a );
    QVariant              vb = caf::pdmToVariant( b );

    EXPECT_FALSE( ( caf::pdmVariantEqual<std::optional<double>>( va, vb ) ) );
    EXPECT_FALSE( ( caf::pdmVariantEqual<std::optional<double>>( vb, va ) ) );
}

TEST( PdmFieldTraits, VariantEqual_Optional_SameValue )
{
    std::optional<double> a  = 2.5;
    std::optional<double> b  = 2.5;
    QVariant              va = caf::pdmToVariant( a );
    QVariant              vb = caf::pdmToVariant( b );

    EXPECT_TRUE( ( caf::pdmVariantEqual<std::optional<double>>( va, vb ) ) );
}

TEST( PdmFieldTraits, VariantEqual_Optional_DifferentValues )
{
    std::optional<double> a  = 2.5;
    std::optional<double> b  = 3.5;
    QVariant              va = caf::pdmToVariant( a );
    QVariant              vb = caf::pdmToVariant( b );

    EXPECT_FALSE( ( caf::pdmVariantEqual<std::optional<double>>( va, vb ) ) );
}
