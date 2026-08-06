#include "gtest/gtest.h"

#include "RimEclipseResultDefinition.h"

#include "Riu3dSelectionManager.h"
#include "RiuPlotUpdater.h"

#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class TestPlotUpdater : public RiuPlotUpdater
{
public:
    void storeSelection( const RiuEclipseSelectionItem* selectionItem ) { storeDelayedInformation( selectionItem ); }

    int queryCount() const { return m_queryCount; }

protected:
    void     clearPlot() override {}
    QWidget* plotPanel() override { return nullptr; }

    bool queryDataAndUpdatePlot( const RimEclipseResultDefinition* eclipseResDef, size_t, size_t, size_t ) override
    {
        m_queryCount++;
        return true;
    }

private:
    int m_queryCount = 0;
};

//--------------------------------------------------------------------------------------------------
/// The result definition can be deleted between the selection change and the delayed update
//--------------------------------------------------------------------------------------------------
TEST( RiuPlotUpdaterTest, DelayedUpdateAfterResultDefinitionIsDeleted )
{
    auto* resultDefinition = new RimEclipseResultDefinition;

    RiuEclipseSelectionItem selectionItem( nullptr,
                                           resultDefinition,
                                           0,
                                           0,
                                           0,
                                           0,
                                           cvf::Color3f( 1.0f, 0.0f, 0.0f ),
                                           cvf::StructGridInterface::NO_FACE,
                                           cvf::Vec3d::ZERO );

    TestPlotUpdater plotUpdater;
    plotUpdater.storeSelection( &selectionItem );

    delete resultDefinition;

    plotUpdater.doDelayedUpdate();

    EXPECT_EQ( 0, plotUpdater.queryCount() );
}
