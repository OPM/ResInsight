
#include "gtest/gtest.h"

#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiItem.h"
#include "cafUpdateEditorsScheduler.h"

#include <functional>
#include <memory>

using namespace caf;

namespace
{
class CountingEditor : public caf::PdmUiEditorHandle
{
public:
    int                   configureCount = 0;
    std::function<void()> onConfigure;

    void bind( PdmUiItem* item ) { bindToPdmItem( item ); }

protected:
    void configureAndUpdateUi( const QString& ) override
    {
        configureCount++;
        if ( onConfigure ) onConfigure();
    }
};
} // namespace

//--------------------------------------------------------------------------------------------------
/// A scheduled item deleted before the scheduled update used to be accessed after deletion
//--------------------------------------------------------------------------------------------------
TEST( UpdateEditorsSchedulerTest, DeletedItemIsNotUpdated )
{
    CountingEditor survivingEditor;
    CountingEditor deletedEditor;

    auto survivingItem = std::make_unique<PdmUiItem>();
    auto deletedItem   = std::make_unique<PdmUiItem>();
    survivingEditor.bind( survivingItem.get() );
    deletedEditor.bind( deletedItem.get() );

    survivingItem->scheduleUpdateConnectedEditors();
    deletedItem->scheduleUpdateConnectedEditors();
    deletedItem.reset();

    UpdateEditorsScheduler::instance()->performScheduledUpdates();

    EXPECT_EQ( 1, survivingEditor.configureCount );
    EXPECT_EQ( 0, deletedEditor.configureCount );
}

//--------------------------------------------------------------------------------------------------
/// Updating the editors of one item can delete another scheduled item, which must then be skipped
//--------------------------------------------------------------------------------------------------
TEST( UpdateEditorsSchedulerTest, ItemDeletedDuringUpdateIsSkipped )
{
    CountingEditor firstEditor;
    CountingEditor secondEditor;

    auto firstItem  = std::make_unique<PdmUiItem>();
    auto secondItem = std::make_unique<PdmUiItem>();
    firstEditor.bind( firstItem.get() );
    secondEditor.bind( secondItem.get() );

    // The processing order depends on the item addresses, so each update deletes the other item
    firstEditor.onConfigure  = [&secondItem]() { secondItem.reset(); };
    secondEditor.onConfigure = [&firstItem]() { firstItem.reset(); };

    firstItem->scheduleUpdateConnectedEditors();
    secondItem->scheduleUpdateConnectedEditors();

    UpdateEditorsScheduler::instance()->performScheduledUpdates();

    EXPECT_EQ( 1, firstEditor.configureCount + secondEditor.configureCount );
}

//--------------------------------------------------------------------------------------------------
/// An update scheduled while updating editors is performed by the next scheduled update
//--------------------------------------------------------------------------------------------------
TEST( UpdateEditorsSchedulerTest, UpdateScheduledDuringUpdateIsKept )
{
    CountingEditor editor;

    auto item = std::make_unique<PdmUiItem>();
    editor.bind( item.get() );

    editor.onConfigure = [&item, &editor]()
    {
        if ( editor.configureCount == 1 ) item->scheduleUpdateConnectedEditors();
    };

    item->scheduleUpdateConnectedEditors();

    UpdateEditorsScheduler::instance()->performScheduledUpdates();
    EXPECT_EQ( 1, editor.configureCount );

    UpdateEditorsScheduler::instance()->performScheduledUpdates();
    EXPECT_EQ( 2, editor.configureCount );
}
