#include "cafPdmCoreColor3f.h"

#include "cafPdmFieldTraits.h"

#include "gtest/gtest.h"

#include <QColor>

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

    QVariant myVariant = caf::toVariant( myColor );

    cvf::Color3f decoded;
    caf::fromVariant( myVariant, decoded );

    EXPECT_FLOAT_EQ( myColor.r(), decoded.r() );
    EXPECT_FLOAT_EQ( myColor.g(), decoded.g() );
    EXPECT_NEAR( myColor.b(), decoded.b(), 0.01 ); // For some reason, 0.18 is not close enough to use EXPECT_FLOAT_EQ
}

TEST( VariantEqualTest, PdmCoreColor3f )
{
    cvf::Color3f a( 0.4f, 0.2f, 0.18f );
    cvf::Color3f b( 0.4f, 0.2f, 0.18f );
    cvf::Color3f c( 0.4f, 0.2f, 0.5f );

    QVariant va = caf::toVariant( a );
    QVariant vb = caf::toVariant( b );
    QVariant vc = caf::toVariant( c );

    EXPECT_TRUE( caf::pdmVariantEqual<cvf::Color3f>( va, vb ) );
    EXPECT_FALSE( caf::pdmVariantEqual<cvf::Color3f>( va, vc ) );
}

// Verifies that caf::toVariant dispatches to the cvf::Color3f overload via ADL,
// producing a QColor variant rather than a raw cvf::Color3f.
TEST( AdlVariantTest, PdmCoreColor3f )
{
    cvf::Color3f original( 0.4f, 0.2f, 0.18f );

    QVariant variant = caf::toVariant( original );

    // The variant must hold a QColor, not a raw cvf::Color3f
    EXPECT_TRUE( variant.canConvert<QColor>() );

    cvf::Color3f decoded;
    caf::fromVariant( variant, decoded );

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
