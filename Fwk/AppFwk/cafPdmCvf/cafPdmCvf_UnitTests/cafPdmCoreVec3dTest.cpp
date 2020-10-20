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

    QVariant myVariant = caf::PdmValueFieldSpecialization<cvf::Vec3d>::convert( myVector );

    cvf::Vec3d decoded;
    caf::PdmValueFieldSpecialization<cvf::Vec3d>::setFromVariant( myVariant, decoded );

    EXPECT_TRUE( decoded.equals( myVector ) );
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
