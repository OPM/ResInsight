
#include "MainWindow.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmDefaultObjectFactory.h"

#include "cafFactory.h"
#include "cafPdmUiFieldEditorHandle.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    auto appExitCode = 0;
    {
        QApplication app(argc, argv);

        MainWindow window;
        window.setWindowTitle("Ceetron Application Framework Test Application");
        window.resize(1000, 810);
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
