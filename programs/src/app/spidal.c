#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "../../config/spidal.h"
#include "../../include/check.h"

#ifndef NO_RPI
    #ifndef NO_STYLE_CHANGES
        #define SPIDAL_UI_PATH PATH "bin/app/spidalUi"
    #else
        #define SPIDAL_UI_PATH PATH "bin/app/noStyleChangesUi"
    #endif
    #define SPIDAL_BACK_PATH PATH "bin/app/spidalBack"
#else
    #define SPIDAL_UI_PATH PATH "bin/app/noRPIUi"
    #define SPIDAL_BACK_PATH PATH "bin/app/noRPIBack"
#endif



pid_t spidalBackPid;
pid_t spidalUiPid;

int main() {
    int pipefd[2];
    pid_t deadPid;
    char arg[2];

    // Communication pipe
    CHECK(pipe(pipefd), "pipe()"); 

    CHECK(spidalUiPid = fork(), "fork() - Spidal UI");
    if (spidalUiPid == 0) {
        // Spidal UI process

        CHECK(close(pipefd[1]), "close()"); // Close writting end

        sprintf(arg, "%d", pipefd[0]);
        CHECK(execl(SPIDAL_UI_PATH, "spidalUI", arg, (char *)0), "execl() - Spidal UI");

        // Should not be here
        exit(EXIT_FAILURE);
    }

    CHECK(spidalBackPid = fork(), "fork() - Spidal Back");
    if (spidalBackPid == 0) {
        // Spidal Back process

        CHECK(close(pipefd[0]), "close()"); // Close reading end
        
        //CHECK(dup2(pipefd[1], STDOUT_FILENO), "dup2()"); // Send stdout to the pipe
        //CHECK(dup2(pipefd[1], STDERR_FILENO), "dup2()"); // Send stderr to the pipe
        //CHECK(close(pipefd[1]), "close()"); // Descriptor no longer needed

        sprintf(arg, "%d", pipefd[1]);
        CHECK(execl(SPIDAL_BACK_PATH, "spidalBack", arg, (char *)0), "execl() - Spidal back");

        // Should not be here
        exit(EXIT_FAILURE);
    }

    // Close pipe in parent process
    CHECK(close(pipefd[0]), "close()");
    CHECK(close(pipefd[1]), "close()");

    deadPid = wait(NULL);
    CHECK(deadPid, "check()");
    if (deadPid == spidalUiPid) {
        printf("End of UI process\nTerminating app\n");
        kill(spidalBackPid, SIGKILL);
        CHECK(wait(NULL), "check()");
    } else {
        printf("End of back process\nTerminating app\n");
        kill(spidalUiPid, SIGKILL);
        CHECK(wait(NULL), "check()");
    }

    // Redirect back stdout
    //CHECK(dup2(STDOUT_FILENO, pipefd[1]), "dup2()");
    // CHECK(close(pipefd[0]), "close()");
    // CHECK(close(pipefd[1]), "close()");

    return EXIT_SUCCESS;
}