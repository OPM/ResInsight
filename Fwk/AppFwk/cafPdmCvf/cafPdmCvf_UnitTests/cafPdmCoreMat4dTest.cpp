#include "cafPdmCoreMat4d.h"

#include "gtest/gtest.h"

#include "cvfMatrix4.h"

cvf::Mat4d createMatrix()
{
    double m00 = 0.00;
    double m01 = 0.01;
    double m02 = 0.02;
    double m03 = 0.03;
    double m10 = 0.10;
    double m11 = 0.11;
    double m12 = 0.12;
    double m13 = 0.13;
    double m20 = 0.20;
    double m21 = 0.21;
    double m22 = 0.22;
    double m23 = 0.23;
    double m30 = 0.30;
    double m31 = 0.31;
    double m32 = 0.32;
    double m33 = 0.33;

    cvf::Mat4d myVector( m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33 );

    return myVector;
}

TEST( SerializeTest, PdmCoreMat4d )
{
    cvf::Mat4d myMatrix = createMatrix();

    QString textString;
    {
        QTextStream out( &textString );
        out << myMatrix;

        EXPECT_EQ( 0, textString.compare( "0 0.01 0.02 0.03 0.1 0.11 0.12 0.13 0.2 0.21 0.22 0.23 0.3 0.31 0.32 0.33" ) );
    }

    {
        cvf::Mat4d  decoded;
        QTextStream out( &textString );
        out >> decoded;

        EXPECT_TRUE( decoded.equals( myMatrix ) );
    }
}

TEST( VariantTest, PdmCoreMat4d )
{
    cvf::Mat4d myMatrix = createMatrix();

    QVariant myVariant = caf::PdmValueFieldSpecialization<cvf::Mat4d>::convert( myMatrix );

    cvf::Mat4d decoded;
    caf::PdmValueFieldSpecialization<cvf::Mat4d>::setFromVariant( myVariant, decoded );

    EXPECT_TRUE( decoded.equals( myMatrix ) );
}

TEST( SerializeSeveralTest, PdmCoreMat4d )
{
    cvf::Mat4d myMatrix = createMatrix();

    QString textString;
    {
        QTextStream out( &textString );
        out << myMatrix << " " << myMatrix;
    }
}
