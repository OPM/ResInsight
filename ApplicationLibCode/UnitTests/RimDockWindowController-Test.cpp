#include "gtest/gtest.h"

#include "RimDockWindowController.h"
#include "RimSummaryPlot.h"

#include "cafPdmDefaultObjectFactory.h"

#include <memory>

//--------------------------------------------------------------------------------------------------
/// handleViewerDeletion() used to dereference viewPdmObject() (m_viewToControl) unconditionally.
/// Calling it before setViewToControl() has ever been called (m_viewToControl is null) must not crash.
//--------------------------------------------------------------------------------------------------
TEST( RimDockWindowControllerTest, HandleViewerDeletionWithoutViewToControlDoesNotCrash )
{
    RimDockWindowController controller;

    EXPECT_NO_FATAL_FAILURE( controller.handleViewerDeletion() );
}

//--------------------------------------------------------------------------------------------------
/// The dock manager identifies dock widgets by the dock window id. Copied views, including plots created from a plot
/// template, must get a new id, while reading a project (or undoing a delete) must keep it.
//--------------------------------------------------------------------------------------------------
TEST( RimDockWindowControllerTest, DockWindowIdIsNotCopied )
{
    RimSummaryPlot plot;
    const QString  dockId = plot.dockWindowName();
    ASSERT_FALSE( dockId.isEmpty() );

    std::unique_ptr<RimSummaryPlot> copy( plot.copyObject<RimSummaryPlot>() );
    ASSERT_TRUE( copy );
    EXPECT_NE( dockId, copy->dockWindowName() );

    const QString xml = plot.writeObjectToXmlString();

    std::unique_ptr<caf::PdmObjectHandle> fromTemplate(
        caf::PdmXmlObjectHandle::readUnknownObjectFromXmlString( xml, caf::PdmDefaultObjectFactory::instance(), true ) );
    auto templatePlot = dynamic_cast<RimSummaryPlot*>( fromTemplate.get() );
    ASSERT_TRUE( templatePlot );
    EXPECT_NE( dockId, templatePlot->dockWindowName() );

    std::unique_ptr<caf::PdmObjectHandle> fromProject(
        caf::PdmXmlObjectHandle::readUnknownObjectFromXmlString( xml, caf::PdmDefaultObjectFactory::instance(), false ) );
    auto projectPlot = dynamic_cast<RimSummaryPlot*>( fromProject.get() );
    ASSERT_TRUE( projectPlot );
    EXPECT_EQ( dockId, projectPlot->dockWindowName() );
}
