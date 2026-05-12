
#include "gtest/gtest.h"

#include "cafIconProvider.h"

#include <QIcon>
#include <QImage>
#include <QPixmap>

using namespace caf;

//--------------------------------------------------------------------------------------------------
/// Regression test for the topmost nested-collection icon: assigning a resource-string-only
/// IconProvider on top of a pixmap-based one must fully replace the state. Previously
/// operator= leaked the existing pixmap through copyPixmap(), so icon() kept returning the
/// stale pixmap and the branded resource icon was never visible.
//--------------------------------------------------------------------------------------------------
TEST( IconProviderTest, AssignmentFromDefaultClearsPixmap )
{
    QPixmap pix( 16, 16 );
    pix.fill( Qt::red );

    IconProvider provider( pix );
    ASSERT_TRUE( provider.valid() );

    provider = IconProvider();

    EXPECT_FALSE( provider.valid() );
}

TEST( IconProviderTest, AssignmentFromResourceStringReplacesPixmap )
{
    QPixmap pix( 16, 16 );
    pix.fill( Qt::red );

    IconProvider provider( pix );

    // Assign a resource-string-only provider whose resource does not resolve to any real
    // image. icon() should yield nullptr because nothing is renderable; with the bug the
    // previous pixmap leaked through and icon() returned the red QIcon.
    provider = IconProvider( ":/non-existent-resource.png" );

    EXPECT_TRUE( provider.icon() == nullptr );
}

TEST( IconProviderTest, AssignmentBetweenPixmapsPicksNewPixmap )
{
    QPixmap red( 16, 16 );
    red.fill( Qt::red );

    QPixmap blue( 16, 16 );
    blue.fill( Qt::blue );

    IconProvider provider( red );
    provider = IconProvider( blue );

    auto icon = provider.icon();
    ASSERT_TRUE( icon != nullptr );

    QImage img = icon->pixmap( 16, 16 ).toImage();
    ASSERT_FALSE( img.isNull() );
    EXPECT_EQ( img.pixel( 0, 0 ), qRgb( 0, 0, 255 ) );
}
