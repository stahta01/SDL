#include <stdio.h>

#include "SDL3/SDL_process.h"
#include "SDL3/SDL_test_assert.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_test_log.h"
#include "SDL3/SDL_test_harness.h"
#include "SDL3/SDL_test_fuzzer.h"

int process_testInheritedEnv();
int process_testNewEnv();

int main(void) {
    return process_testInheritedEnv();
}

int process_testInheritedEnv()
{
    const char *process_args[] = {
        "test/childprocess.exe",
        "--print-environment",
        NULL,
    };
    SDL_PropertiesID props;
    SDL_Process *process = NULL;
    Sint64 pid;
    int exit_code;
    char random_env1[64];
    char random_env2[64];
    static const char *const TEST_ENV_KEY1 = "TESTPROCESS_INHERITED_VAR";
    static const char *const TEST_ENV_KEY2 = "TESTPROCESS_OTHER_VAR";
    char *test_env_val1 = NULL;
    char *test_env_val2 = NULL;
    char *buffer = NULL;

    test_env_val1 = SDLTest_RandomAsciiStringOfSize(32);
    SDL_snprintf(random_env1, sizeof(random_env1), "%s=%s", TEST_ENV_KEY1, test_env_val1);
    SDLTest_AssertPass("Setting parent environment variable %s=%s", TEST_ENV_KEY1, test_env_val1);
    SDL_SetEnvironmentVariable(SDL_GetEnvironment(), TEST_ENV_KEY1, test_env_val1, true);

    SDL_UnsetEnvironmentVariable(SDL_GetEnvironment(), TEST_ENV_KEY2);

    props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ARGS_POINTER, (void *)process_args);
    SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDOUT_NUMBER, SDL_PROCESS_STDIO_APP);
    process = SDL_CreateProcessWithProperties(props);
    SDL_DestroyProperties(props);
    SDLTest_AssertCheck(process != NULL, "SDL_CreateProcessWithProperties()");
    if (!process) {
        goto failed;
    }

    test_env_val2 = SDLTest_RandomAsciiStringOfSize(32);
    SDL_snprintf(random_env2, sizeof(random_env2), "%s=%s", TEST_ENV_KEY2, test_env_val2);
    SDLTest_AssertPass("Setting parent environment variable %s=%s", TEST_ENV_KEY2, test_env_val2);
    SDL_SetEnvironmentVariable(SDL_GetEnvironment(),TEST_ENV_KEY2, test_env_val2, true);
    SDLTest_AssertCheck(SDL_strcmp(test_env_val1, test_env_val2) != 0, "Sanity checking the 2 random environment variables are not identical");

    props = SDL_GetProcessProperties(process);
    SDLTest_AssertCheck(props != 0, "SDL_GetProcessProperties()");

    pid = SDL_GetNumberProperty(props, SDL_PROP_PROCESS_PID_NUMBER, 0);
    SDLTest_AssertCheck(pid != 0, "Checking process ID, expected non-zero, got %" SDL_PRIs64, pid);

    exit_code = 0xdeadbeef;
    buffer = (char *)SDL_ReadProcess(process, NULL, &exit_code);
    SDLTest_AssertCheck(buffer != NULL, "SDL_ReadProcess()");
    SDLTest_AssertCheck(exit_code == 0, "Exit code should be 0, is %d", exit_code);

    SDLTest_AssertCheck(SDL_strstr(buffer, random_env1) != NULL, "Environment of child should contain \"%s\"", test_env_val1);
    SDLTest_AssertCheck(SDL_strstr(buffer, random_env2) == NULL, "Environment of child should not contain \"%s\"", test_env_val2);

    SDLTest_AssertPass("About to destroy process");
    SDL_DestroyProcess(process);
    SDL_free(test_env_val1);
    SDL_free(test_env_val2);
    SDL_free(buffer);
    return TEST_COMPLETED;
failed:
    SDL_free(test_env_val1);
    SDL_free(test_env_val2);
    SDL_DestroyProcess(process);
    SDL_free(buffer);
    return TEST_ABORTED;
}

int process_testNewEnv()
{
    const char *process_args[] = {
        "test/childprocess.exe",
        "--print-environment",
        NULL,
    };
    SDL_Environment *process_env;
    SDL_PropertiesID props;
    SDL_Process *process = NULL;
    Sint64 pid;
    int exit_code;
    char random_env1[64];
    char random_env2[64];
    static const char *const TEST_ENV_KEY1 = "TESTPROCESS_INHERITED_VAR";
    static const char *const TEST_ENV_KEY2 = "TESTPROCESS_OTHER_VAR";
    char *test_env_val1 = NULL;
    char *test_env_val2 = NULL;
    char *buffer = NULL;
    size_t total_read = 0;

    test_env_val1 = SDLTest_RandomAsciiStringOfSize(32);
    SDL_snprintf(random_env1, sizeof(random_env1), "%s=%s", TEST_ENV_KEY1, test_env_val1);
    SDLTest_AssertPass("Unsetting parent environment variable %s", TEST_ENV_KEY1);
    SDL_UnsetEnvironmentVariable(SDL_GetEnvironment(), TEST_ENV_KEY1);

    process_env = SDL_CreateEnvironment(true);
    SDL_SetEnvironmentVariable(process_env, "PATH", SDL_GetEnvironmentVariable(SDL_GetEnvironment(), "PATH"), true);
    SDL_SetEnvironmentVariable(process_env, "LD_LIBRARY_PATH", SDL_GetEnvironmentVariable(SDL_GetEnvironment(), "LD_LIBRARY_PATH"), true);
    SDL_SetEnvironmentVariable(process_env, "DYLD_LIBRARY_PATH", SDL_GetEnvironmentVariable(SDL_GetEnvironment(), "DYLD_LIBRARY_PATH"), true);
    SDL_SetEnvironmentVariable(process_env, TEST_ENV_KEY1, test_env_val1, true);

    test_env_val2 = SDLTest_RandomAsciiStringOfSize(32);
    SDL_snprintf(random_env2, sizeof(random_env2), "%s=%s", TEST_ENV_KEY2, test_env_val1);
    SDLTest_AssertPass("Setting parent environment variable %s=%s", TEST_ENV_KEY2, test_env_val2);
    SDL_SetEnvironmentVariable(SDL_GetEnvironment(), TEST_ENV_KEY2, test_env_val2, true);
    SDLTest_AssertCheck(SDL_strcmp(test_env_val1, test_env_val2) != 0, "Sanity checking the 2 random environment variables are not identical");

    props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ARGS_POINTER, (void *)process_args);
    SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ENVIRONMENT_POINTER, process_env);
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

    exit_code = 0xdeadbeef;
    buffer = (char *)SDL_ReadProcess(process, &total_read, &exit_code);
    SDLTest_AssertCheck(buffer != NULL, "SDL_ReadProcess()");
    SDLTest_AssertCheck(exit_code == 0, "Exit code should be 0, is %d", exit_code);
    SDLTest_LogEscapedString("Text read from subprocess: ", buffer, total_read);

    SDLTest_AssertCheck(SDL_strstr(buffer, random_env1) != NULL, "Environment of child should contain \"%s\"", random_env1);
    SDLTest_AssertCheck(SDL_strstr(buffer, random_env2) == NULL, "Environment of child should not contain \"%s\"", random_env1);

    SDLTest_AssertPass("About to destroy process");
    SDL_DestroyProcess(process);
    SDL_DestroyEnvironment(process_env);
    SDL_free(test_env_val1);
    SDL_free(test_env_val2);
    SDL_free(buffer);
    return TEST_COMPLETED;

failed:
    SDL_DestroyProcess(process);
    SDL_DestroyEnvironment(process_env);
    SDL_free(test_env_val1);
    SDL_free(test_env_val2);
    SDL_free(buffer);
    return TEST_ABORTED;
}

