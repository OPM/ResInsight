#include "cafPdmCoreVec3d.h"

#include "gtest/gtest.h"

#include "cvfVector3.h"

TEST( SerializeTest, PdmCoreVec3d )
{
    double a = 2.4;
    double b = 12.4;
    double c = 232.778;

    cvf::Vec3d myVector( a, b, c );

    QString textString;
    {
        QTextStream out( &textString );
        out << myVector;

        EXPECT_EQ( 0, textString.compare( "2.4 12.4 232.778" ) );
    }

    {
        cvf::Vec3d  decoded;
        QTextStream out( &textString );
        out >> decoded;

        EXPECT_TRUE( decoded.equals( myVector ) );
    }
}

TEST( VariantTest, PdmCoreVec3d )
{
    double a = 2.4;
    double b = 12.4;
    double c = 232.778;

    cvf::Vec3d myVector( a, b, c );

    QVariant myVariant = caf::toVariant( myVector );

    cvf::Vec3d decoded;
    caf::fromVariant( myVariant, decoded );

    EXPECT_TRUE( decoded.equals( myVector ) );
}

TEST( VariantEqualTest, PdmCoreVec3d )
{
    cvf::Vec3d a( 1.0, 2.0, 3.0 );
    cvf::Vec3d b( 1.0, 2.0, 3.0 );
    cvf::Vec3d c( 1.0, 2.0, 4.0 );

    QVariant va = caf::toVariant( a );
    QVariant vb = caf::toVariant( b );
    QVariant vc = caf::toVariant( c );

    EXPECT_TRUE( caf::pdmVariantEqual<cvf::Vec3d>( va, vb ) );
    EXPECT_FALSE( caf::pdmVariantEqual<cvf::Vec3d>( va, vc ) );
}

TEST( SerializeSeveralTest, PdmCoreVec3d )
{
    double a = 2.4;
    double b = 12.4;
    double c = 232.778;

    cvf::Vec3d myVector( a, b, c );

    QString textString;
    {
        QTextStream out( &textString );
        out << myVector << " " << myVector;
    }
}
