
#include "MainWindow.h"

#include "cafCmdFeatureManager.h"
#include "cafFactory.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmLogging.h"
#include "cafPdmUiFieldEditorHandle.h"
#include "cafUiAppearanceSettings.h"

#include <QApplication>

#include <iostream>

class ConsoleLogger : public caf::PdmLogger
{
public:
    int  level() const override { return static_cast<int>( caf::PdmLogLevel::PDM_LL_DEBUG ); }
    void setLevel( int logLevel ) override {}

    void error( const QString& message ) override { std::cout << "ERROR: " << message.toStdString() << std::endl; }
    void warning( const QString& message ) override { std::cout << "WARNING: " << message.toStdString() << std::endl; }
    void info( const QString& message ) override { std::cout << "INFO: " << message.toStdString() << std::endl; }
    void debug( const QString& message ) override { std::cout << "DEBUG: " << message.toStdString() << std::endl; }
};

int main( int argc, char* argv[] )
{
    // https://www.w3.org/wiki/CSS/Properties/color/keywords
    caf::UiAppearanceSettings::instance()->setAutoValueEditorColor( "moccasin" );

    caf::PdmLogging::registerLogger( std::make_shared<ConsoleLogger>() );

    auto appExitCode = 0;
    {
        QApplication app( argc, argv );

        caf::CmdFeatureManager::createSingleton();

        MainWindow window;
        window.setWindowTitle( "Ceetron Application Framework Test Application" );
        window.resize( 1000, 810 );
        window.show();

        appExitCode = app.exec();
    }

    caf::PdmLogging::clearAllLoggers();
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
