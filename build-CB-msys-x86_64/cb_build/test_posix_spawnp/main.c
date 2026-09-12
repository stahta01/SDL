#include <spawn.h>
#include <stdio.h>
#include <sys/wait.h>

#include "SDL3/SDL_process.h"
#include "SDL3/SDL_test_assert.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_test_log.h"
#include "SDL3/SDL_test_harness.h"

int process_testArguments();

int main(void) {
//    posix_spawnattr_t attr;
//    posix_spawn_file_actions_t fa;
//    pid_t pid;
//    char **envp = NULL;
//    char * const childargv[] = {"../../test/testprocess", "test/childprocess.exe", NULL};
//
//    if (posix_spawnattr_init(&attr) != 0) {
//        //SDL_SetError("posix_spawnattr_init failed: %s", strerror(errno));
//        goto posix_spawn_fail_none;
//    }
//
//    if (posix_spawn_file_actions_init(&fa) != 0) {
//        //SDL_SetError("posix_spawn_file_actions_init failed: %s", strerror(errno));
//        goto posix_spawn_fail_attr;
//    }
//
//    int i = 0;
//    while(childargv[i] != NULL){
//        printf("childargv %s\n", childargv[i++]);
//    }
//    printf("\n");
//
//    if (posix_spawnp(&pid, childargv[0], &fa, &attr,
//                     childargv, envp) == 0) {
//        int status;
//        waitpid(pid, &status, 0);
//    } else {
//        goto posix_spawn_fail_all;
//    }
//
//    posix_spawn_file_actions_destroy(&fa);
//    return 0;
//
//    posix_spawn_fail_all:
//    posix_spawn_fail_attr:
//    posix_spawn_fail_none:
//
//    return 1;
    return process_testArguments();
}

int process_testArguments()
{
    const char *process_args[] = {
        "test/childprocess.exe",
        "--print-arguments",
        "--",
        "",
        "  ",
        "a b c",
        "a\tb\tc\t\v\r\n",
        "\"a b\" c",
        "'a' 'b' 'c'",
        "%d%%%s",
        "\\t\\c",
        "evil\\",
        "a\\b\"c\\",
        "\"\\^&|<>%", /* characters with a special meaning */
        NULL
    };
    SDL_Process *process = NULL;
    char *buffer;
    int exit_code;
    int i;
    size_t total_read = 0;

    process = SDL_CreateProcess(process_args, true);
    SDLTest_AssertCheck(process != NULL, "SDL_CreateProcess()");
    if (!process) {
        goto failed;
    }

    exit_code = 0xdeadbeef;
    buffer = (char *)SDL_ReadProcess(process, &total_read, &exit_code);
    SDLTest_AssertCheck(buffer != NULL, "SDL_ReadProcess()");
    SDLTest_AssertCheck(exit_code == 0, "Exit code should be 0, is %d", exit_code);
    if (!buffer) {
        goto failed;
    }
    SDLTest_LogEscapedString("stdout of process: ", buffer, total_read);

    for (i = 3; process_args[i]; i++) {
        char line[64];
        SDL_snprintf(line, sizeof(line), "|%d=%s|", i - 3, process_args[i]);
        SDLTest_AssertCheck(!!SDL_strstr(buffer, line), "Check %s is in output", line);
    }
    SDL_free(buffer);

    SDLTest_AssertPass("About to destroy process");
    SDL_DestroyProcess(process);
    return TEST_COMPLETED;
failed:
    SDL_DestroyProcess(process);
    return TEST_ABORTED;
}
