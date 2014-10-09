#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char ** argv) {
  struct tm ts;
  ts.tm_sec    = 0;
  ts.tm_min    = 0;
  ts.tm_hour   = 0;
  ts.tm_mday   = 1;
  ts.tm_mon    = 0;
  ts.tm_year   = 0;
  ts.tm_isdst  = -1;
  {
    time_t t = mktime( &ts );
    if (t == -1) 
      exit(1);
    else
      exit(0);
  }
}
