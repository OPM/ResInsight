#include "gtest/gtest.h"

#include "cafPdmChildField.h"
#include "cafPdmDataValueField.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmSettings.h"
#include "cafPdmXmlObjectHandle.h"
#include "cafPdmXmlObjectHandleMacros.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

class SettingsLeaf : public caf::PdmObjectHandle, public caf::PdmXmlObjectHandle
{
    CAF_PDM_XML_HEADER_INIT;

public:
    SettingsLeaf()
        : PdmObjectHandle()
        , PdmXmlObjectHandle( this, false )
    {
        CAF_PDM_XML_InitField( &m_value, "Value" );
    }

    caf::PdmDataValueField<int> m_value;
};
CAF_PDM_XML_SOURCE_INIT( SettingsLeaf, "SettingsLeaf" );

class SettingsMiddle : public caf::PdmObjectHandle, public caf::PdmXmlObjectHandle
{
    CAF_PDM_XML_HEADER_INIT;

public:
    SettingsMiddle()
        : PdmObjectHandle()
        , PdmXmlObjectHandle( this, false )
    {
        CAF_PDM_XML_InitField( &m_leaf, "Leaf" );
        m_leaf = new SettingsLeaf;
    }

    ~SettingsMiddle() { delete m_leaf(); }

    caf::PdmChildField<SettingsLeaf*> m_leaf;
};
CAF_PDM_XML_SOURCE_INIT( SettingsMiddle, "SettingsMiddle" );

class SettingsRoot : public caf::PdmObjectHandle, public caf::PdmXmlObjectHandle
{
    CAF_PDM_XML_HEADER_INIT;

public:
    SettingsRoot()
        : PdmObjectHandle()
        , PdmXmlObjectHandle( this, false )
    {
        CAF_PDM_XML_InitField( &m_middle, "Middle" );
        m_middle = new SettingsMiddle;
    }

    ~SettingsRoot() { delete m_middle(); }

    caf::PdmChildField<SettingsMiddle*> m_middle;
};
CAF_PDM_XML_SOURCE_INIT( SettingsRoot, "SettingsRoot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( PdmSettings, NestedChildrenRoundTrip )
{
    QSettings::setDefaultFormat( QSettings::IniFormat );
    QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, QDir::tempPath() );
    QCoreApplication::setOrganizationName( "cafPdmSettingsTest" );
    QCoreApplication::setApplicationName( "cafPdmSettingsTest" );
    QSettings().clear();

    {
        SettingsRoot root;
        root.m_middle()->m_leaf()->m_value = 42;
        caf::PdmSettings::writeFieldsToApplicationStore( &root );
    }

    // Written with the same nested context as used when reading
    EXPECT_TRUE( QSettings().contains( "SettingsMiddle/SettingsLeaf/Value" ) );

    SettingsRoot restored;
    restored.m_middle()->m_leaf()->m_value = 0;
    caf::PdmSettings::readFieldsFromApplicationStore( &restored );
    EXPECT_EQ( 42, restored.m_middle()->m_leaf()->m_value() );

    QSettings().clear();
}
