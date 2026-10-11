
#include "gtest/gtest.h"

#include "cafPdmUiEditorHandle.h"

#include <stdexcept>

using namespace caf;

namespace
{
class TestEditor : public caf::PdmUiEditorHandle
{
public:
    int  configureCount        = 0;
    bool throwOnNextConfigure  = false;
    bool updateFromConfigureUi = false;

protected:
    void configureAndUpdateUi( const QString& uiConfigName ) override
    {
        configureCount++;

        if ( throwOnNextConfigure )
        {
            throwOnNextConfigure = false;
            throw std::runtime_error( "configureAndUpdateUi failed" );
        }

        if ( updateFromConfigureUi ) updateUi( uiConfigName );
    }
};
} // namespace

//--------------------------------------------------------------------------------------------------
/// A re-entrant updateUi() from configureAndUpdateUi() used to abort on CAF_ASSERT( !m_isConfiguringUi )
//--------------------------------------------------------------------------------------------------
TEST( PdmUiEditorHandleTest, ReentrantUpdateIsSkipped )
{
    TestEditor editor;
    editor.updateFromConfigureUi = true;

    editor.updateUi( "" );
    EXPECT_EQ( 1, editor.configureCount );

    editor.updateUi();
    EXPECT_EQ( 2, editor.configureCount );
}

//--------------------------------------------------------------------------------------------------
/// An exception from configureAndUpdateUi() used to leave the editor in configuring state, and the next updateUi()
/// aborted on CAF_ASSERT( !m_isConfiguringUi )
//--------------------------------------------------------------------------------------------------
TEST( PdmUiEditorHandleTest, UpdateAfterExceptionInConfigure )
{
    TestEditor editor;
    editor.throwOnNextConfigure = true;

    EXPECT_THROW( editor.updateUi( "" ), std::runtime_error );
    EXPECT_EQ( 1, editor.configureCount );

    editor.updateUi( "" );
    EXPECT_EQ( 2, editor.configureCount );
}
