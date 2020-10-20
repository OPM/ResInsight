
#include "gtest/gtest.h"

#include "cafPdmDataValueField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmXmlObjectHandle.h"
#include "cafPdmXmlObjectHandleMacros.h"

#include <QXmlStreamWriter>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class SimpleObjectWithNumbers : public caf::PdmObjectHandle, public caf::PdmXmlObjectHandle
{
    CAF_PDM_XML_HEADER_INIT;

public:
    SimpleObjectWithNumbers()
        : PdmObjectHandle()
        , PdmXmlObjectHandle( this, false )
    {
        CAF_PDM_XML_InitField( &m_valueA, "ValueA" );
        CAF_PDM_XML_InitField( &m_valueB, "ValueB" );

        CAF_PDM_XML_InitField( &m_floatValueA, "FloatValueA" );
        CAF_PDM_XML_InitField( &m_floatValueB, "FloatValueB" );
    }

    caf::PdmDataValueField<double> m_valueA;
    caf::PdmDataValueField<double> m_valueB;

    caf::PdmDataValueField<float> m_floatValueA;
    caf::PdmDataValueField<float> m_floatValueB;
};
CAF_PDM_XML_SOURCE_INIT( SimpleObjectWithNumbers, "SimpleObjectWithNumbers" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( SerializeNumbers, SimpleObjectWithDoubleValues )
{
    double valueA = 0.123456789;
    double valueB = 123456789 + valueA;

    QString objectAsText;

    {
        SimpleObjectWithNumbers obj1;

        obj1.m_valueA = valueA;
        obj1.m_valueB = valueB;

        objectAsText = obj1.writeObjectToXmlString();
    }

    {
        SimpleObjectWithNumbers obj1;

        obj1.readObjectFromXmlString( objectAsText, caf::PdmDefaultObjectFactory::instance() );

        {
            double epsilon = 1e-7;

            double diffA = fabs( obj1.m_valueA - valueA );
            EXPECT_TRUE( diffA < epsilon );
        }

        {
            double epsilon = 3e-7;

            double diffB = fabs( obj1.m_valueB - valueB );
            EXPECT_TRUE( diffB < epsilon );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( SerializeNumbers, SimpleObjectWithFloatValues )
{
    float valueA = 0.123456789f;
    float valueB = 123456 + valueA;

    QString objectAsText;

    {
        SimpleObjectWithNumbers obj1;

        obj1.m_floatValueA = valueA;
        obj1.m_floatValueB = valueB;

        objectAsText = obj1.writeObjectToXmlString();
    }

    {
        SimpleObjectWithNumbers obj1;

        obj1.readObjectFromXmlString( objectAsText, caf::PdmDefaultObjectFactory::instance() );

        double epsilon = 1e-7;

        double diffA = fabs( obj1.m_floatValueA - valueA );
        EXPECT_TRUE( diffA < epsilon );

        double diffB = fabs( obj1.m_floatValueB - valueB );
        EXPECT_TRUE( diffB < epsilon );
    }
}
