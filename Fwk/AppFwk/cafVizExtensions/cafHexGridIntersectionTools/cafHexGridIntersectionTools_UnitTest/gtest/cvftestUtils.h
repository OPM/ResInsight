

namespace cvftest
{
//==================================================================================================
//
//
//==================================================================================================
class Utils
{
public:
    static cvf::String getTestDataDir( const cvf::String& unitTestFolder )
    {
#ifdef WIN32
        std::string exe = std::string( testing::internal::GetArgvs()[0] );
#else
        std::string dir = std::string( testing::internal::FilePath::GetCurrentDir().ToString() );
        std::string exe = dir + std::string( "/" ) + std::string( testing::internal::GetArgvs()[0] );
#endif
        std::string testPath = exe.substr( 0, exe.find( unitTestFolder.toStdString() ) ) + std::string( "TestData/" );

        return testPath;
    }

    static cvf::String getGLSLDir( const cvf::String& unitTestFolder )
    {
#ifdef WIN32
        std::string exe = std::string( testing::internal::GetArgvs()[0] );
#else
        std::string dir = std::string( testing::internal::FilePath::GetCurrentDir().ToString() );
        std::string exe = dir + std::string( "/" ) + std::string( testing::internal::GetArgvs()[0] );
#endif
        std::string glslPath = exe.substr( 0, exe.find( unitTestFolder.toStdString() ) ) +
                               std::string( "../LibRender/glsl/" );

        return glslPath;
    }
};

} // namespace cvftest
