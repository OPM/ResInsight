#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ert/util/util.h>
#include <ert/util/test_util.hpp>


void checkPath(const char * path, const char * directory, const char * base_name, const char * extension) {
    char * dir;
    char * base;
    char * ext;

    util_alloc_file_components(path, &dir, &base, &ext);

    test_assert_string_equal(dir, directory);
    test_assert_string_equal(base, base_name);
    test_assert_string_equal(ext, extension);

    free(dir);
    free(base);
    free(ext);

}

int main(int argc , char ** argv) {

    checkPath("/dir/filename.ext", "/dir", "filename", "ext");
    checkPath("/dir/subdir/filename.ext", "/dir/subdir", "filename", "ext");
    checkPath("/dir/subdir/filename.name.ext", "/dir/subdir", "filename.name", "ext");
    checkPath("/dir/subdir/filename", "/dir/subdir", "filename", NULL);
    checkPath("filename.ext", NULL, "filename", "ext");
    checkPath("filename", NULL, "filename", NULL);
    checkPath(".filename", NULL, ".filename", NULL);
    checkPath(".filename.ext", NULL, ".filename", "ext");
    checkPath("./SPECASE1", ".", "SPECASE1", NULL);
    checkPath("/absolute/path/to/CASE", "/absolute/path/to", "CASE", NULL);
}
