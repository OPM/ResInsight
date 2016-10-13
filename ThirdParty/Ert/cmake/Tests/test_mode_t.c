#include <sys/stat.h>

int main(int argc, char ** argv) {
  mode_t new_mode = S_IWGRP;
  chmod( "/tmp" , new_mode );
}
