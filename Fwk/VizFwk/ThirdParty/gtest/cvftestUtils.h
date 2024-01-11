
namespace cvftest {


//==================================================================================================
//
// Static helper class for unit tests
//
//==================================================================================================
class Utils
{
public:
    static cvf::String getTestDataDir()
    {
        std::string testPath = "";
#if defined(CVF_CEEVIZ_ROOT_SOURCE_DIR)
        testPath = CVF_CEEVIZ_ROOT_SOURCE_DIR "/Tests/TestData/";
#endif

        std::string testPathEnv = getEnvironmentVar("CVF_UTEST_DATA_DIR");
        if (testPathEnv.length() > 0)
        {
            testPath = testPathEnv;
        }

        return testPath;
    }

private:
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


}

