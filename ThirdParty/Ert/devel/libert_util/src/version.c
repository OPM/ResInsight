#define DOT "."

#ifdef ERT_VERSION_MAJOR
#ifdef ERT_VERSION_MINOR
#ifdef ERT_VERSION_MICRO
    #define ERT_VERSION ERT_VERSION_MAJOR DOT ERT_VERSION_MINOR DOT ERT_VERSION_MICRO
#endif
#endif
#endif


char* version_get_git_commit() {
    #ifdef GIT_COMMIT
        return GIT_COMMIT;
    #else
        return "Unknown git commit hash";
    #endif
}

char* version_get_git_commit_short() {
    #ifdef GIT_COMMIT_SHORT
        return GIT_COMMIT_SHORT;
    #else
        return "Unknown git short commit hash";
    #endif
}

char* version_get_ert_version() {
    #ifdef ERT_VERSION
        return ERT_VERSION;
    #else
        return "Unknown version";
    #endif
}

char* version_get_build_time() {
    #ifdef COMPILE_TIME_STAMP
        return COMPILE_TIME_STAMP;
    #else
        return "Unknown build time";
    #endif
}
