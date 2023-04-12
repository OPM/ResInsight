
#include "MainWindow.h"

#include "cafCmdFeatureManager.h"
#include "cafFactory.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafUiAppearanceSettings.h"

#include <QApplication>

int main( int argc, char* argv[] )
{
    // https://www.w3.org/wiki/CSS/Properties/color/keywords
    caf::UiAppearanceSettings::instance()->setAutoValueEditorColor( "moccasin" );

    auto appExitCode = 0;
    {
        QApplication app( argc, argv );

        MainWindow window;
        window.setWindowTitle( "Ceetron Application Framework Test Application" );
        window.resize( 1000, 810 );
        window.show();

        appExitCode = app.exec();
    }

    caf::CmdFeatureManager::deleteSingleton();
    caf::PdmDefaultObjectFactory::deleteSingleton();

    {
        auto factory = caf::Factory<caf::PdmUiFieldEditorHandle, QString>::instance();
        factory->deleteCreatorObjects();
    }
    {
        auto factory = caf::Factory<caf::CmdFeature, std::string>::instance();
        factory->deleteCreatorObjects();
    }

    return appExitCode;
}
