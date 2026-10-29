#include "gtest/gtest.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiPropertyView.h"

#include <QLabel>

#include <new>

using namespace caf;

namespace
{
alignas( std::max_align_t ) unsigned char childStorage[4096];
} // namespace

//--------------------------------------------------------------------------------------------------
/// Always allocated at the same address, to reproduce a deleted object being replaced by one at the same address
//--------------------------------------------------------------------------------------------------
class FormLayoutTestChild : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    FormLayoutTestChild()
    {
        CAF_PDM_InitObject( "FormLayoutTestChild" );
        CAF_PDM_InitField( &m_text, "Text", QString( "text" ), "Text" );
    }

    static void* operator new( size_t size )
    {
        EXPECT_LE( size, sizeof( childStorage ) );
        return childStorage;
    }
    static void operator delete( void* ) {}

    caf::PdmField<QString> m_text;
};
CAF_PDM_SOURCE_INIT( FormLayoutTestChild, "FormLayoutTestChild" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class FormLayoutTestParent : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    FormLayoutTestParent()
    {
        CAF_PDM_InitObject( "FormLayoutTestParent" );
        CAF_PDM_InitFieldNoDefault( &m_child, "Child", "Child" );
        m_child = new FormLayoutTestChild;
    }

    void replaceChild()
    {
        delete m_child();
        m_child = new FormLayoutTestChild;
    }

    caf::PdmChildField<FormLayoutTestChild*> m_child;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override
    {
        uiOrdering.add( &m_child->m_text );
        uiOrdering.skipRemainingFields();
    }
};
CAF_PDM_SOURCE_INIT( FormLayoutTestParent, "FormLayoutTestParent" );

//--------------------------------------------------------------------------------------------------
/// A field editor bound to a deleted field must not be reused for a new field at the same address
//--------------------------------------------------------------------------------------------------
TEST( PdmUiFormLayoutObjectEditorTest, FieldReplacedAtSameAddress )
{
    FormLayoutTestParent parent;

    PdmUiPropertyView view;
    view.showProperties( &parent );

    auto oldField = &parent.m_child->m_text;
    ASSERT_EQ( 1u, oldField->uiCapability()->connectedEditors().size() );

    parent.replaceChild();
    ASSERT_EQ( oldField, &parent.m_child->m_text );
    ASSERT_TRUE( parent.m_child->m_text.uiCapability()->connectedEditors().empty() );

    parent.updateConnectedEditors();

    EXPECT_EQ( 1u, parent.m_child->m_text.uiCapability()->connectedEditors().size() );

    view.showProperties( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class FormLayoutTestLabelObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    FormLayoutTestLabelObject()
    {
        CAF_PDM_InitObject( "FormLayoutTestLabelObject" );
        CAF_PDM_InitField( &m_text, "Text", QString( "text" ), "FieldLabel" );
    }

    caf::PdmField<QString> m_text;

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override
    {
        uiOrdering.addNewLabel( "InfoLabel" );
        uiOrdering.add( &m_text );
        uiOrdering.skipRemainingFields();
    }
};
CAF_PDM_SOURCE_INIT( FormLayoutTestLabelObject, "FormLayoutTestLabelObject" );

namespace
{
int visibleLabelCount( const QWidget& root, const QString& text )
{
    int count = 0;
    for ( auto label : root.findChildren<QLabel*>() )
    {
        if ( !label->isHidden() && label->text().startsWith( text ) ) count++;
    }
    return count;
}
} // namespace

//--------------------------------------------------------------------------------------------------
/// Widgets scheduled for deferred deletion must be hidden immediately. Deferred deletes are not processed by nested
/// event loops, and many visible stale siblings overflow the stack in QWidgetPrivate::paintSiblingsRecursive.
//--------------------------------------------------------------------------------------------------
TEST( PdmUiFormLayoutObjectEditorTest, StaleWidgetsAreHiddenWithoutEventLoop )
{
    FormLayoutTestLabelObject first;
    FormLayoutTestLabelObject second;

    PdmUiPropertyView view;
    view.showProperties( &first );

    for ( int i = 0; i < 5; i++ )
    {
        first.updateConnectedEditors();
    }
    EXPECT_EQ( 1, visibleLabelCount( view, "InfoLabel" ) );

    view.showProperties( &second );
    EXPECT_EQ( 1, visibleLabelCount( view, "InfoLabel" ) );
    EXPECT_EQ( 1, visibleLabelCount( view, "FieldLabel" ) );

    view.showProperties( nullptr );
}
