#include "gtest/gtest.h"

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiPropertyView.h"

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
