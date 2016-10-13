#include <signal.h>

void sigbus_handler(int signal) {
  
}


int main(int argc, char ** argv) {
  signal(SIGBUS , sigbus_handler);
}
