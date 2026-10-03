#include "gtest/gtest.h"

#include "RimDockWindowController.h"

//--------------------------------------------------------------------------------------------------
/// handleViewerDeletion() used to dereference viewPdmObject() (m_viewToControl) unconditionally.
/// Calling it before setViewToControl() has ever been called (m_viewToControl is null) must not crash.
//--------------------------------------------------------------------------------------------------
TEST( RimDockWindowControllerTest, HandleViewerDeletionWithoutViewToControlDoesNotCrash )
{
    RimDockWindowController controller;

    EXPECT_NO_FATAL_FAILURE( controller.handleViewerDeletion() );
}
