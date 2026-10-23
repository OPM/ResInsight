#include "gtest/gtest.h"

#include "cafPdmField.h"
#include "cafPdmLogging.h"
#include "cafPdmObject.h"
#include "cafPdmUiAttributeTools.h"
#include "cafPdmUiFieldHandle.h"
#include "cafPdmUiItem.h"

#include <QSize>

#include <functional>

using namespace caf;

class AttributeToolsTestObj : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    AttributeToolsTestObj()
    {
        CAF_PDM_InitObject( "AttributeToolsTestObj" );
        CAF_PDM_InitField( &m_field, "Field", 0, "Field" );
    }

    caf::PdmField<int> m_field;
};
CAF_PDM_SOURCE_INIT( AttributeToolsTestObj, "AttributeToolsTestObj" );

TEST( PdmUiAttributeToolsTest, StringAttribute )
{
    AttributeToolsTestObj obj;
    PdmUiItem*            uiItem = obj.m_field.uiCapability();

    const QString key = "format";

    EXPECT_FALSE( PdmUiAttributeTools::stringAttribute( uiItem, key ).has_value() );

    uiItem->setAttribute( key, QString( "yyyy-MM-dd" ) );
    EXPECT_EQ( "yyyy-MM-dd", PdmUiAttributeTools::stringAttribute( uiItem, key ).value().toStdString() );

    uiItem->setAttribute( key, "dd MMM yyyy" );
    EXPECT_EQ( "dd MMM yyyy", PdmUiAttributeTools::stringAttribute( uiItem, key ).value().toStdString() );

    uiItem->setAttribute( key, QSize( 1, 2 ) );
    EXPECT_FALSE( PdmUiAttributeTools::stringAttribute( uiItem, key ).has_value() );
}

TEST( PdmUiAttributeToolsTest, StringAttributeFromCallback )
{
    AttributeToolsTestObj obj;
    PdmUiItem*            uiItem = obj.m_field.uiCapability();

    const QString key = "format";

    QString current = "first";
    uiItem->setAttribute<std::function<QString()>>( key, [&current]() { return current; } );

    EXPECT_EQ( "first", PdmUiAttributeTools::stringAttribute( uiItem, key ).value().toStdString() );

    // The callback is evaluated on every call
    current = "second";
    EXPECT_EQ( "second", PdmUiAttributeTools::stringAttribute( uiItem, key ).value().toStdString() );

    uiItem->setAttribute<std::function<QString()>>( key, std::function<QString()>() );
    EXPECT_FALSE( PdmUiAttributeTools::stringAttribute( uiItem, key ).has_value() );
}

TEST( PdmUiAttributeToolsTest, StringAttributeUsesConfigName )
{
    AttributeToolsTestObj obj;
    PdmUiItem*            uiItem = obj.m_field.uiCapability();

    const QString key = "format";

    uiItem->setAttribute( key, "default" );
    uiItem->setAttribute( key, "special", "SpecialConfig" );

    EXPECT_EQ( "default", PdmUiAttributeTools::stringAttribute( uiItem, key ).value().toStdString() );
    EXPECT_EQ( "special", PdmUiAttributeTools::stringAttribute( uiItem, key, "SpecialConfig" ).value().toStdString() );
}

namespace
{
class CapturingLogger : public caf::PdmLogger
{
public:
    int  level() const override { return static_cast<int>( caf::PdmLogLevel::PDM_LL_DEBUG ); }
    void setLevel( int ) override {}
    void error( const QString& ) override {}
    void warning( const QString& message ) override { warnings.push_back( message ); }
    void info( const QString& ) override {}
    void debug( const QString& ) override {}

    std::vector<QString> warnings;
};
} // namespace

TEST( PdmUiAttributeToolsTest, ValidateAttributes )
{
    AttributeToolsTestObj obj;
    PdmUiItem*            uiItem = obj.m_field.uiCapability();

    auto logger = std::make_shared<CapturingLogger>();
    caf::PdmLogging::registerLogger( logger );

    const std::set<QString> supported = { "known" };

    uiItem->setAttribute( "known", 1 );
    uiItem->validateAttributes( "TestEditor", supported );
    EXPECT_TRUE( logger->warnings.empty() );

    uiItem->setAttribute( "unknown", 1 );
    uiItem->validateAttributes( "TestEditor", supported );
    ASSERT_EQ( 1u, logger->warnings.size() );
    EXPECT_TRUE( logger->warnings.front().contains( "unknown" ) );

    // Attributes only set for another config are not validated
    logger->warnings.clear();
    uiItem->removeAttribute( "unknown" );
    uiItem->setAttribute( "otherUnknown", 1, "OtherConfig" );
    uiItem->validateAttributes( "TestEditor", supported );
    EXPECT_TRUE( logger->warnings.empty() );

    // Table view cell attributes are consumed by the table editor and never reported
    uiItem->setAttribute( "tableViewPushButtonText", "Edit" );
    uiItem->validateAttributes( "TestEditor", supported );
    EXPECT_TRUE( logger->warnings.empty() );

    caf::PdmLogging::unregisterLogger( logger );
}
