#include <pybind11/pybind11.h>

#include <QCoreApplication>

#include "RiaApplication.h"
#include "../../ApplicationExeCode/RiaMainTools.h"
#include "RiaQuantityInfoTools.h"
#include "cafCmdFeatureManager.h"
#include "RiaRegressionTestRunner.h"



class RiaPythonApplication : public QCoreApplication, public RiaApplication
{
public:
    RiaPythonApplication(int& argc, char** argv)
        : QCoreApplication(argc, argv)
        , RiaApplication()
    {
        installEventFilter(this);
    }

    ApplicationStatus handleArguments(gsl::not_null<cvf::ProgramOptions*> progOpt) override;
    void showFormattedTextInMessageBoxOrConsole(const QString& errMsg) override;

protected:
    void invokeProcessEvents(QEventLoop::ProcessEventsFlags flags = QEventLoop::AllEvents) override;
    void onProjectOpened() override;
    void onProjectOpeningError(const QString& errMsg) override;
    void onProjectClosed() override;
};


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaApplication::ApplicationStatus RiaPythonApplication::handleArguments(gsl::not_null<cvf::ProgramOptions*> progOpt)
{
    return ApplicationStatus::KEEP_GOING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPythonApplication::showFormattedTextInMessageBoxOrConsole(const QString& errMsg)
{
    throw std::logic_error("The method or operation is not implemented.");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPythonApplication::invokeProcessEvents(QEventLoop::ProcessEventsFlags flags /*= QEventLoop::AllEvents */)
{
    throw std::logic_error("The method or operation is not implemented.");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPythonApplication::onProjectOpened()
{
    return;
    // throw std::logic_error( "The method or operation is not implemented." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPythonApplication::onProjectOpeningError(const QString& errMsg)
{
    return;
    // throw std::logic_error( "The method or operation is not implemented." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPythonApplication::onProjectClosed()
{
    return;
    // throw std::logic_error( "The method or operation is not implemented." );
}


class PythonAppSingleton
{
public:

    PythonAppSingleton()
    {
        // Create feature manager before the application object is created
        caf::CmdFeatureManager::createSingleton();
        RiaRegressionTestRunner::createSingleton();
        caf::PdmDefaultObjectFactory::createSingleton();
        
        RiaQuantityInfoTools::initializeSummaryKeywords();


        int argc = 0;
        m_app.reset( new RiaPythonApplication(argc, nullptr));
    }

    static PythonAppSingleton* instance()
    {
        static PythonAppSingleton theInstance;
        return &theInstance;
    }

private:
    std::unique_ptr<RiaPythonApplication> m_app;
};



int add(int a, int b) {

    return a + b;
}



void init_singletons() {

//    RiaPythonApplication::ensureInstanceIsCreated();

}

std::string octave_path() {
    std::string text;
    
    text = "i was here";
    //return text;

    PythonAppSingleton::instance();
    auto app = RiaApplication::instance();
    if (app)
    {
        app->initialize();
        text = app->octavePath().toStdString();
    }

    return text;

}


// Create a Python module named `example` and bind the `add` function.
PYBIND11_MODULE(msjmodule, m) {
    m.doc() = "Example module created using Pybind11";  // Module docstring
    m.def("init_singletons", &init_singletons, "Init application singletons");
    m.def("add", &add, "A function that adds two numbers");
    m.def("octave_path", &octave_path, "Read out Octave Path");

}
