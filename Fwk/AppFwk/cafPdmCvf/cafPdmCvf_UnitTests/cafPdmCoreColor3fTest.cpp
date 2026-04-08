#include "cafPdmCoreColor3f.h"

#include "cafPdmFieldTraits.h"

#include "gtest/gtest.h"

#include <QColor>

// Helper templates that mirror the call pattern in PdmUiFieldSpecializationForValueSpec<T>.
// The "using caf::pdmToVariant" directive means the cvf::Color3f overload is only reachable
// via ADL (argument namespace), not via the using-declaration alone.
template <typename T>
QVariant adlConvert( const T& value )
{
    using caf::pdmToVariant;
    return pdmToVariant( value );
}

template <typename T>
void adlSetFromVariant( const QVariant& v, T& out )
{
    using caf::pdmFromVariant;
    pdmFromVariant( v, out );
}

TEST( SerializeTest, PdmCoreColor3f )
{
    float        r = 0.4f;
    float        g = 0.2f;
    float        b = 0.18f;
    cvf::Color3f myColor( r, g, b );

    QString textString;
    {
        QTextStream out( &textString );
        out << myColor;

        EXPECT_EQ( 0, textString.compare( "0.4 0.2 0.18" ) );
    }

    {
        cvf::Color3f decoded;
        QTextStream  out( &textString );
        out >> decoded;

        EXPECT_TRUE( decoded == myColor );
    }
}

TEST( VariantTest, PdmCoreColor3f )
{
    float        r = 0.4f;
    float        g = 0.2f;
    float        b = 0.18f;
    cvf::Color3f myColor( r, g, b );

    using caf::pdmToVariant;
    QVariant myVariant = pdmToVariant( myColor );

    cvf::Color3f decoded;
    using caf::pdmFromVariant;
    pdmFromVariant( myVariant, decoded );

    EXPECT_FLOAT_EQ( myColor.r(), decoded.r() );
    EXPECT_FLOAT_EQ( myColor.g(), decoded.g() );
    EXPECT_NEAR( myColor.b(), decoded.b(), 0.01 ); // For some reason, 0.18 is not close enough to use EXPECT_FLOAT_EQ
}

TEST( VariantEqualTest, PdmCoreColor3f )
{
    cvf::Color3f a( 0.4f, 0.2f, 0.18f );
    cvf::Color3f b( 0.4f, 0.2f, 0.18f );
    cvf::Color3f c( 0.4f, 0.2f, 0.5f );

    using caf::pdmToVariant;
    QVariant va = pdmToVariant( a );
    QVariant vb = pdmToVariant( b );
    QVariant vc = pdmToVariant( c );

    EXPECT_TRUE( caf::pdmVariantEqual<cvf::Color3f>( va, vb ) );
    EXPECT_FALSE( caf::pdmVariantEqual<cvf::Color3f>( va, vc ) );
}

// Verifies the ADL round-trip: adlConvert/adlSetFromVariant mirror the template pattern used in
// PdmUiFieldSpecializationForValueSpec<T>, where the cvf::Color3f overload must be found via ADL.
TEST( AdlVariantTest, PdmCoreColor3f )
{
    cvf::Color3f original( 0.4f, 0.2f, 0.18f );

    QVariant variant = adlConvert( original );

    // The variant must hold a QColor, not a raw cvf::Color3f
    EXPECT_TRUE( variant.canConvert<QColor>() );

    cvf::Color3f decoded;
    adlSetFromVariant( variant, decoded );

    EXPECT_FLOAT_EQ( original.r(), decoded.r() );
    EXPECT_FLOAT_EQ( original.g(), decoded.g() );
    EXPECT_NEAR( original.b(), decoded.b(), 0.01 );
}

TEST( SerializeSeveralTest, PdmCoreColor3f )
{
    float        r = 0.4f;
    float        g = 0.2f;
    float        b = 0.18f;
    cvf::Color3f myColor( r, g, b );

    QString textString;
    {
        QTextStream out( &textString );
        out << myColor << " " << myColor;
    }
}
