
#include "gtest/gtest.h"

#include "cafProgressInfo.h"

#include <QCoreApplication>
#include <QEvent>
#include <QObject>

#include <algorithm>

namespace
{
//--------------------------------------------------------------------------------------------------
/// Reproduces the re-entrant event processing that drove the progress dialog into unbounded
/// recursion. Each queued event, when dispatched, updates the progress which in turn pumps the
/// event loop. Without a re-entrancy guard the freshly queued event is dispatched immediately,
/// re-entering this handler and growing the call stack until it overflows.
//--------------------------------------------------------------------------------------------------
class RecursiveProgressTrigger : public QObject
{
public:
    RecursiveProgressTrigger( caf::ProgressInfo& progressInfo, int eventCount )
        : m_progressInfo( progressInfo )
        , m_remainingEvents( eventCount )
    {
    }

    int maxDepth() const { return m_maxDepth; }

    void postNext() { QCoreApplication::postEvent( this, new QEvent( QEvent::User ) ); }

protected:
    void customEvent( QEvent* event ) override
    {
        if ( event->type() != QEvent::User ) return;

        ++m_currentDepth;
        m_maxDepth = std::max( m_maxDepth, m_currentDepth );

        if ( m_remainingEvents > 0 )
        {
            --m_remainingEvents;

            // Keep an event queued so the loop always has something to dispatch, then trigger a
            // progress update. The update pumps the event loop; the re-entrancy guard must keep
            // that nested pump from dispatching the queued event and recursing back into here.
            postNext();
            m_progressInfo.setProgress( static_cast<size_t>( ++m_progressValue ) );
        }

        --m_currentDepth;
    }

private:
    caf::ProgressInfo& m_progressInfo;
    int                m_remainingEvents = 0;
    int                m_currentDepth    = 0;
    int                m_maxDepth        = 0;
    size_t             m_progressValue   = 1;
};

} // namespace

//--------------------------------------------------------------------------------------------------
/// Regression test for the recursive stack overflow in the progress dialog. A large number of
/// queued events each trigger a progress update that pumps the event loop. With the re-entrancy
/// guard in place the nested updates return without pumping, so every event is dispatched at
/// depth 1. Without the guard each dispatched event recurses through setProgress() -> processEvents()
/// and the call stack grows until it overflows.
//--------------------------------------------------------------------------------------------------
TEST( ProgressInfoTest, EventProcessingDoesNotRecurseUnbounded )
{
    const int eventCount = 5000;

    // delayShowingProgress = true keeps the dialog hidden (no real window) while still creating it,
    // so progress updates pump the event loop exactly as they do in production.
    caf::ProgressInfo progressInfo( static_cast<size_t>( eventCount + 2 ), "Recursion test", true, false );

    RecursiveProgressTrigger trigger( progressInfo, eventCount );
    trigger.postNext();

    // Kick off processing through a progress update, mirroring production where a progress update
    // is what pumps the event loop.
    progressInfo.setProgress( 1 );

    EXPECT_EQ( trigger.maxDepth(), 1 );
}
