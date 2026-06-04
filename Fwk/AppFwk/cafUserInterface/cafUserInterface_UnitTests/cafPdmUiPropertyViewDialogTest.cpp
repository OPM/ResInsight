
#include "gtest/gtest.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmUiPropertyViewDialog.h"

#include <QApplication>
#include <QSettings>

using namespace caf;

class GeometryTestObj : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    GeometryTestObj()
    {
        CAF_PDM_InitObject( "GeometryTestObj" );
        CAF_PDM_InitField( &m_value, "Value", 0, "Value" );
    }

    caf::PdmField<int> m_value;
};
CAF_PDM_SOURCE_INIT( GeometryTestObj, "GeometryTestObj" );

//--------------------------------------------------------------------------------------------------
/// Verify that the dialog stores its geometry on close and restores it on the next show.
//--------------------------------------------------------------------------------------------------
TEST( PdmUiPropertyViewDialogTest, GeometryRoundTrip )
{
    // Ensure QSettings has a defined, isolated location for the test.
    QCoreApplication::setOrganizationName( "ResInsightTest" );
    QCoreApplication::setApplicationName( "DialogGeometryTest" );
    QSettings().clear();

    // Geometry persistence is gated behind an experimental feature and disabled by default. Enable it
    // explicitly for this test.
    PdmUiPropertyViewDialog::enableGeometryPersistence( true );

    const QSize userSize( 742, 531 );

    {
        GeometryTestObj obj;

        PdmUiPropertyViewDialog dialog( nullptr, &obj, "Geometry Round Trip", "" );
        dialog.show();
        QApplication::processEvents();

        dialog.resize( userSize );
        QApplication::processEvents();

        // done() is what saves the geometry; reject() routes through done().
        dialog.reject();
    }

    // The settings store should now contain a value for this dialog.
    {
        QSettings settings;
        bool      anyKey = false;
        for ( const QString& key : settings.allKeys() )
        {
            if ( key.contains( "Geometry Round Trip" ) ) anyKey = true;
        }
        EXPECT_TRUE( anyKey ) << "No geometry was written to settings";
    }

    {
        GeometryTestObj obj;

        PdmUiPropertyViewDialog dialog( nullptr, &obj, "Geometry Round Trip", "" );
        dialog.show();
        QApplication::processEvents();

        EXPECT_EQ( dialog.size().width(), userSize.width() );
        EXPECT_EQ( dialog.size().height(), userSize.height() );

        dialog.reject();
    }
}
