#include <stdio.h>

#include "SDL3/SDL_process.h"
#include "SDL3/SDL_test_assert.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_test_log.h"
#include "SDL3/SDL_test_harness.h"
#include "SDL3/SDL_test_fuzzer.h"
#include "SDL3/SDL_properties.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_test_compare.h"

int process_testStdinToStdout();

int main(void) {
    return process_testStdinToStdout();
}

int process_testStdinToStdout()
{
    const char *process_args[] = {
        "test/childprocess.exe",
        "--stdin-to-stdout",
        NULL,
    };
    SDL_PropertiesID props;
    SDL_Process *process = NULL;
    Sint64 pid;
    SDL_IOStream *process_stdin = NULL;
    SDL_IOStream *process_stdout = NULL;
    SDL_IOStream *process_stderr = NULL;
    size_t text_in_size = 1 * 1024 * 1024;
    char *text_in = NULL;
    size_t total_written;
    size_t total_read;
    bool wait_result;
    int exit_code;
    SDL_IOStream *stdout_stream = NULL;
    char *stdout_stream_buf;
    int iteration_count = 0;

    text_in = SDLTest_RandomAsciiStringOfSize((int)text_in_size);
    /* Make sure text_in does not contain EOF */
    for (;;) {
        char *e = SDL_strstr(text_in, "EOF");
        if (!e) {
            break;
        }
        e[0] = 'N';
    }
    text_in[text_in_size - 3] = 'E';
    text_in[text_in_size - 2] = 'O';
    text_in[text_in_size - 1] = 'F';

    stdout_stream = SDL_IOFromDynamicMem();

    props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ARGS_POINTER, (void *)process_args);
    SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDIN_NUMBER, SDL_PROCESS_STDIO_APP);
    SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDOUT_NUMBER, SDL_PROCESS_STDIO_APP);
    process = SDL_CreateProcessWithProperties(props);
    SDL_DestroyProperties(props);
    SDLTest_AssertCheck(process != NULL, "SDL_CreateProcessWithProperties()");
    if (!process) {
        goto failed;
    }

    props = SDL_GetProcessProperties(process);
    SDLTest_AssertCheck(props != 0, "SDL_GetProcessProperties()");

    pid = SDL_GetNumberProperty(props, SDL_PROP_PROCESS_PID_NUMBER, 0);
    SDLTest_AssertCheck(pid != 0, "Checking process ID, expected non-zero, got %" SDL_PRIs64, pid);

    process_stdin = SDL_GetProcessInput(process);
    SDLTest_AssertCheck(process_stdin != NULL, "SDL_GetPointerProperty(SDL_PROP_PROCESS_STDIN_POINTER) returns a valid IO stream");
    process_stdout = SDL_GetProcessOutput(process);
    SDLTest_AssertCheck(process_stdout != NULL, "SDL_GetPointerProperty(SDL_PROP_PROCESS_STDOUT_POINTER) returns a valid IO stream");
    process_stderr = (SDL_IOStream *)SDL_GetPointerProperty(props, SDL_PROP_PROCESS_STDERR_POINTER, NULL);
    SDLTest_AssertCheck(process_stderr == NULL, "SDL_GetPointerProperty(SDL_PROP_PROCESS_STDERR_POINTER) returns NULL");
    if (!process_stdin || !process_stdout) {
        goto failed;
    }

    printf("%s\n", "Inside process_testStdinToStdout");

    total_written = 0;
    total_read = 0;
    for (;;) {
        int log_this_iteration = 1; // (iteration_count % 32) == 32;
        char local_buffer[16 * 4094];
        size_t amount_read;
        SDL_IOStatus io_status;
        //printf("%s\n total_written %u text_in_size %u", "Inside process_testStdinToStdout, before if (total_written != text_in_size)", total_written, text_in_size);
        if (total_written != text_in_size) {
            size_t amount_written;
            if (log_this_iteration) {
                SDLTest_AssertPass("About to SDL_WriteIO (%dth time)", iteration_count);
            }
            amount_written = SDL_WriteIO(process_stdin, text_in + total_written, text_in_size - total_written);
            if (log_this_iteration) {
                SDLTest_Log("SDL_WriteIO() -> %u (%dth time)", (unsigned)amount_written, iteration_count);
            }
            if (amount_written == 0) {
                io_status = SDL_GetIOStatus(process_stdin);
                if (io_status != SDL_IO_STATUS_NOT_READY) {
                    SDLTest_Log("SDL_GetIOStatus(process_stdin) returns %d, breaking.", io_status);
                    break;
                }
            }
            total_written += amount_written;
            SDL_FlushIO(process_stdin);
        }

        /* FIXME: this needs a rate limit */
        if (log_this_iteration) {
            SDLTest_AssertPass("About to SDL_ReadIO (%dth time)", iteration_count);
        }
        amount_read = SDL_ReadIO(process_stdout, local_buffer, sizeof(local_buffer));
        if (log_this_iteration) {
            SDLTest_Log("SDL_ReadIO() -> %u (%dth time)", (unsigned)amount_read, iteration_count);
        }
        if (amount_read == 0) {
            io_status = SDL_GetIOStatus(process_stdout);
            if (io_status != SDL_IO_STATUS_NOT_READY) {
                SDLTest_Log("SDL_GetIOStatus(process_stdout) returned %d, breaking.", io_status);
                break;
            }
        } else {
            total_read += amount_read;
            SDL_WriteIO(stdout_stream, local_buffer, amount_read);
            stdout_stream_buf = SDL_GetPointerProperty(SDL_GetIOProperties(stdout_stream), SDL_PROP_IOSTREAM_DYNAMIC_MEMORY_POINTER, NULL);
            Sint64 stdout_size = SDL_GetIOSize(stdout_stream);
            if (SDL_strnstr(stdout_stream_buf, "EOF", (size_t)stdout_size)) {
                SDLTest_Log("Found EOF in stdout");
                break;
            }
        }
        SDL_Delay(10);
    }
    SDLTest_Log("Wrote %" SDL_PRIu64 " bytes to process.stdin", (Uint64)total_written);
    SDLTest_Log("Read %" SDL_PRIu64 " bytes from process.stdout",(Uint64)total_read);

    stdout_stream_buf = SDL_GetPointerProperty(SDL_GetIOProperties(stdout_stream), SDL_PROP_IOSTREAM_DYNAMIC_MEMORY_POINTER, NULL);
    SDLTest_CompareMemory(stdout_stream_buf, total_written, text_in, text_in_size);

    exit_code = 0xdeadbeef;
    wait_result = SDL_WaitProcess(process, false, &exit_code);
    SDLTest_AssertCheck(wait_result == false, "Process should not have closed yet");

    SDLTest_AssertPass("About to close stdin");
    /* Closing stdin of `subprocessstdin --stdin-to-stdout` should close the process */
    SDL_CloseIO(process_stdin);

    process_stdin = SDL_GetProcessInput(process);
    SDLTest_AssertCheck(process_stdin == NULL, "SDL_GetPointerProperty(SDL_PROP_PROCESS_STDIN_POINTER) is cleared after close");

    SDLTest_AssertPass("About to wait on process");
    exit_code = 0xdeadbeef;
    wait_result = SDL_WaitProcess(process, true, &exit_code);
    SDLTest_AssertCheck(wait_result == true, "Process should have closed when closing stdin");
    SDLTest_AssertCheck(exit_code == 0, "Exit code should be 0, is %d", exit_code);
    if (!wait_result) {
        bool killed;
        SDL_Log("About to kill process");
        killed = SDL_KillProcess(process, true);
        SDLTest_AssertCheck(killed, "SDL_KillProcess succeeded");
    }
    SDLTest_AssertPass("About to destroy process");
    SDL_DestroyProcess(process);
    SDL_CloseIO(stdout_stream);
    SDL_free(text_in);
    return TEST_COMPLETED;
failed:

    SDL_DestroyProcess(process);
    SDL_CloseIO(stdout_stream);
    SDL_free(text_in);
    return TEST_ABORTED;
}
