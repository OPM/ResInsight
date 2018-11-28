#ifndef PATH_UTIL
#define PATH_UTIL


#include <string>
namespace ecl {
  namespace util {
    namespace path {
      /*
        Observe that these functions are purely based on string inspection; i.e.
        the actual filesystem is *never* consulted. Furthermore the functions
        interpret the argument as a *filename* - that implies the (maybe
        surprising) semantics:

            dirname("/tmp")   =>  "/"
            basename("/tmp")  =>  "tmp"

        Although if you actually checked the filesystem you would of course
        discover that /tmp actually is a directory. This is the same behaviour
        as the os.path.dirname() and os.path.basename() functions in the Python
        library.
      */

      std::string dirname(const std::string& fname);
      std::string basename(const std::string& fname);
      std::string extension(const std::string& fname);
    }
  }
}


#endif
