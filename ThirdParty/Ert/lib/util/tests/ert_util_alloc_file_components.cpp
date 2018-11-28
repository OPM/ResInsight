#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ert/util/util.h>
#include <ert/util/test_util.hpp>


bool checkPath(const char * path, const char * directory, const char * base_name, const char * extension) {
    char * dir;
    char * base;
    char * ext;

    util_alloc_file_components(path, &dir, &base, &ext);

    bool success = true;

    success = success && (util_string_equal(dir, directory) || dir == directory);
    success = success && (util_string_equal(base, base_name) || base == base_name);
    success = success && (util_string_equal(ext, extension) || ext == extension);

    free(dir);
    free(base);
    free(ext);

    return success;
}

int main(int argc , char ** argv) {

    test_assert_true(checkPath("/dir/filename.ext", "/dir", "filename", "ext"));
    test_assert_true(checkPath("/dir/subdir/filename.ext", "/dir/subdir", "filename", "ext"));
    test_assert_true(checkPath("/dir/subdir/filename.name.ext", "/dir/subdir", "filename.name", "ext"));
    test_assert_true(checkPath("/dir/subdir/filename", "/dir/subdir", "filename", NULL));
    test_assert_true(checkPath("filename.ext", NULL, "filename", "ext"));
    test_assert_true(checkPath("filename", NULL, "filename", NULL));
    test_assert_true(checkPath(".filename", NULL, ".filename", NULL));
    test_assert_true(checkPath(".filename.ext", NULL, ".filename", "ext"));

    exit(0);
}
