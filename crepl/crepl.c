#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/wait.h>

#define TEMP_C "/tmp/input.c"
#define TEMP_SO "/tmp/input.so"

int expid = 0;

// Compile a function definition and load it
bool compile_and_load_function(const char* function_def) {
    FILE *fp = fopen(TEMP_C, "a");
    long prev_size = 0;
    if (!fp) {
        perror("Failed to open temporary C file");
        return false;
    }else{
        fseek(fp, 0, SEEK_END); // Move to the end of the file
        prev_size = ftell(fp);
        fprintf(fp, "%s\n", function_def);
    }
    fclose(fp);

    //编译
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: execute the compilation command
        execlp("gcc", "gcc", "-shared", "-fPIC", "-Wno-implicit-function-declaration", "-o", TEMP_SO, TEMP_C, (char *)NULL);
        perror("Failed to execute gcc");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Failed to fork");
        return false;
    } else {
        // Parent process: wait for the child to finish
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            truncate(TEMP_C, prev_size); // Revert the temporary C file to its previous state
            fprintf(stderr, "Compilation failed with exit code %d\n", WEXITSTATUS(status));
            return false;
        }
    }
    return true;
}

// Evaluate an expression
bool evaluate_expression(const char* expression, int* result) {
    // wrapper function to evaluate the expression
    char wrapper_code[512];
    char expname[512];
    // getpid() makes the wrapper name unique even when TestKit forks
    // each test case into a separate child process (where expid resets to 0).
    snprintf(expname, sizeof(expname), "__expr_wapper_%d_%d", getpid(), expid);
    snprintf(wrapper_code, sizeof(wrapper_code), "int __expr_wapper_%d_%d() { return %s; }", getpid(), expid++, expression);
    bool res = compile_and_load_function(wrapper_code);
    if(!res) {
        return false;
    }
    // Load the compiled shared object and call the wrapper function
    void* handle = dlopen(TEMP_SO, RTLD_NOW);  // NOW: fail at load time on undefined symbols
    if (!handle) {
        fprintf(stderr, "Failed to load shared object: %s\n", dlerror());
        return false;
    }
    int (*func)() = dlsym(handle, expname);
    if (!func) {
        fprintf(stderr, "Failed to find function: %s\n", dlerror());
        dlclose(handle);
        return false;
    }
    *result = func();
    dlclose(handle);
    return true;
}

int main() {
    char input[256];
    int result;
    FILE *fp = fopen(TEMP_C, "w");
    if(fp) fclose(fp); // Clear the temporary C file at the start

    printf("Welcome to the C REPL!\n");
    while (true) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) {
            break; // Exit on EOF
        }
        input[strcspn(input, "\n")] = '\0'; // Remove newline character

        // Check if the input is a function definition
        if (strncmp(input, "int", 3) == 0) {
            if (compile_and_load_function(input)) {
                printf("Add: %s\n", input);
            } else {
                printf("Failed to compile function.\n");
            }
        } else {
            // Evaluate the expression
            if (evaluate_expression(input, &result)) {
                printf("(%s) == %d\n", input, result);
            } else {
                printf("Failed to evaluate expression.\n");
            }
        }
    }

    return 0;;

}


/*
int gcd(int a, int b) { return b ? gcd(b, a % b) : a; }
*/