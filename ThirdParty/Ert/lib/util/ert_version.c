#include <ert/util/util.h>
#include <ert/util/ert_version.h>

#define xstr(s) #s
#define str(s) xstr(s)

char* version_get_git_commit() {
    #ifdef GIT_COMMIT
        return str(GIT_COMMIT);
    #else
        return "Unknown git commit hash";
    #endif
}

char* version_get_git_commit_short() {
    #ifdef GIT_COMMIT_SHORT
        return str(GIT_COMMIT_SHORT);
    #else
        return "Unknown git short commit hash";
    #endif
}


char* version_get_build_time() {
    #ifdef COMPILE_TIME_STAMP
        return COMPILE_TIME_STAMP;
    #else
        return "Unknown build time";
    #endif
}

int version_get_major_ert_version() {
  return ERT_VERSION_MAJOR;
}


int version_get_minor_ert_version() {
  return ERT_VERSION_MINOR;
}


const char * version_get_micro_ert_version() {
  return str(ERT_VERSION_MICRO);
}


bool version_is_ert_devel_version() {
  return util_sscanf_int(str(ERT_VERSION_MICRO), NULL);
}
