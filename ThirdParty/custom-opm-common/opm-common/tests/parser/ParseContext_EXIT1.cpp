#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>


void exit1(Opm::InputError::Action action) {
    const char * deckString =
        "RUNSPEC\n"
        "DIMENS\n"
        "  10 10 10 10 /n"
        "\n";

    Opm::ParseContext parseContext;
    Opm::Parser parser;
    Opm::ErrorGuard errors;

    parseContext.update(Opm::ParseContext::PARSE_EXTRA_DATA , action);
    parser.parseString( deckString , parseContext, errors );
}



/*
  This test checks that the application will exit with status 1 - if that is
  requested; since the boost test framework has registered atexit() handlers
  which will unconditionally fail the complete test in the face of an exit(1) -
  this test is implemented without the BOOST testing framework.
*/

void test_exit(Opm::InputError::Action action) {
    pid_t pid = fork();
    if (pid == 0)
        exit1(action);

    int wait_status;
    waitpid(pid, &wait_status, 0);

    if (WIFEXITED(wait_status)) {
        /*
          We *want* the child process to terminate with status exit(1), i.e. if
          the exit status is 0 we fail the complete test with exit(1).
        */
        if (WEXITSTATUS(wait_status) == 0)
            std::exit(EXIT_FAILURE);
    } else
        std::exit(EXIT_FAILURE);
}

int main() {
    test_exit(Opm::InputError::Action::EXIT1);
    test_exit(Opm::InputError::Action::DELAYED_EXIT1);
}
