
namespace cvftest {



//==================================================================================================
//
// Static helper class for unit tests
//
//==================================================================================================
class Utils
{
public:
    static bool doesEnvironmentVarExist(const char* name)
    {        
        const char* env = testing::internal::posix::GetEnv(name);
        return env ? true : false;
    }

    static std::string getEnvironmentVar(const char* name)
    {        
        const char* env = testing::internal::posix::GetEnv(name);
        if (env)
        {
            return env;
        }
        else
        {
            return std::string();
        }
    }

    static std::string getMyExecutablePath()
    {
#ifdef WIN32
        std::string exe = std::string(testing::internal::GetArgvs()[0]);
#else
        std::string dir = std::string(testing::internal::FilePath::GetCurrentDir().ToString());
        std::string exe = dir + std::string("/") + std::string(testing::internal::GetArgvs()[0]);
#endif
        return exe;
    }
};



//==================================================================================================
//
// Helper for making a test data directory available inside unit tests
//
// The data directories can be specified via the environment variables CVF_UTEST_DATA_DIR and
// CVF_UTEST_EXTRA_DATA_DIR. Expects a the path to contain a trailing slash.
//   Linux:  export CVF_UTEST_DATA_DIR="../../../Tests/TestData/"
//
// The main data dir may be populated with default values if the environment variable isn't set.
// The DefaultDataDir enum determines which default will be used.
//==================================================================================================
class TestDataDir
{
public:
    // What value to use if the test data dir environment variable isn't set?
    enum DefaultDataDir
    {
        EMPTY_STRING,                       // Empty string
        DEFAULT_DEFINE,                     // Try and set from the CVF_UTEST_DEFAULT_DATA_DIR define (must of course be defined, typical usage is via CMake)
        DEFAULT_DEFINE_THEN_VIZ_FRAMEWORK   // First try the define above, then fall back to the default test data directory for VizFramework
    };

public:
    /// Static initialize function that sets up the singleton and initializes 
    /// the values of the data directories based on value of the specified enum
    //--------------------------------------------------------------------------------------------------
    static void initializeInstance(DefaultDataDir defaultDataDir, bool verbose)
    {
        TestDataDir* theInstance = internalInstance();
        theInstance->initialize(defaultDataDir, verbose);
    }

    /// Get pointer to singleton intance, must call initializeInstance() before use
    //--------------------------------------------------------------------------------------------------
    static const TestDataDir* instance()
    {
        const TestDataDir* theInstance = internalInstance();
        if (!theInstance->m_isInitialized)
        {
            // Not sure if it is wise to use the gtest internal macros here, but haven't found another solution yet
            GTEST_MESSAGE_("TestDataDir::initializeInstance() must be called first!", ::testing::TestPartResult::kFatalFailure);
        }
        return theInstance;
    }

    /// Returns the main data directory
    //--------------------------------------------------------------------------------------------------
    std::string dataDir() const
    {
        return m_dataDir;
    }

    /// Returns the extra data directory
    //--------------------------------------------------------------------------------------------------
    std::string extraDataDir() const
    {
        return m_extraDataDir;
    }

private:
    TestDataDir()
        : m_isInitialized(false)
    {
    }

    // Initializes the data members
    void initialize(DefaultDataDir defaultDataDir, bool verbose)
    {
        std::string valSrcStr = "empty";

        if (Utils::doesEnvironmentVarExist("CVF_UTEST_DATA_DIR"))
        {
            m_dataDir = Utils::getEnvironmentVar("CVF_UTEST_DATA_DIR");
            valSrcStr = "environmentVar";
        }
        else
        {
            if (defaultDataDir != EMPTY_STRING)
            {
                m_dataDir = getValueOfDefaultDefine();
                if (!m_dataDir.empty())
                {
                    valSrcStr = "define";
                }
                else if (defaultDataDir == DEFAULT_DEFINE_THEN_VIZ_FRAMEWORK)
                {
                    m_dataDir = getVizFrameworkDefault();
                    if (!m_dataDir.empty())
                    {
                        valSrcStr = "vizFramework";
                    }
                }
            }
        }

        m_extraDataDir = Utils::getEnvironmentVar("CVF_UTEST_EXTRA_DATA_DIR");

        if (verbose)
        {
            printf("\n");
            printf("dataDir : \"%s\"  [src=%s]\n", m_dataDir.c_str(), valSrcStr.c_str());
            printf("extraDir: \"%s\"\n", m_extraDataDir.c_str());
        }

        m_isInitialized = true;
    }

    // Extract value set via compile time define
    static std::string getValueOfDefaultDefine()
    {
        std::string defDataDir;
#ifdef CVF_UTEST_DEFAULT_DATA_DIR
        defDataDir = CVF_UTEST_DEFAULT_DATA_DIR;
#endif        
        return defDataDir;
    }

    // Determine VizFramework default based on executable path and our fixed dir structure
    static std::string getVizFrameworkDefault()
    {
        std::string exe = Utils::getMyExecutablePath();
        std::string dataDir;
#ifdef WIN32
        dataDir = exe.substr(0, exe.find("VizFwk\\")) + std::string("VizFwk\\Tests\\TestData\\");
#else
        dataDir = exe.substr(0, exe.find("VizFwk/")) + std::string("VizFwk/Tests/TestData/");
#endif
        return dataDir;
    }

    static TestDataDir* internalInstance()
    {
        static TestDataDir sl_theInstance;
        return &sl_theInstance;
    }

private:
    bool                m_isInitialized;
    std::string         m_dataDir;          // The primary data directory
    std::string         m_extraDataDir;     // Optional extra data dir, settable only via environment variable
};


}

