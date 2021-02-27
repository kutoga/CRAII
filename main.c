#include <stdio.h>
#include "craii.h"

/*
 * Variable cleanup.
 */

static void example1_cleanup(int *value)
{
    printf("Cleanup example 1: %d\n", *value);
}

int example1(int p)
{
    /*
     * Call a function as soon as the function goes out-of-scope.
     * The function is bound to the variable and get the variable
     * as an argument.
     */
    int x CLEANUP_VAR(example1_cleanup) = 1;

    if (p == 0) {
        // Here is example1_cleanup(&x) called
        return 0;
    }

    // Here is example1_cleanup(&x) called
    return x;
}

/*
 * Value cleanup.
 */

static void example2_cleanup(int **value)
{
    printf("Cleanup example 2: %d\n", **value);
}

static void example2_cleanup2(int **value)
{
    printf("Cleanup example 2 2: %d\n", **value);
}

int example2(int p)
{
    /*
     * Call a function as soon as the function goes out-of-scope.
     * The function is bound to a value. Note: Multiple CLEANUP_VAL(...)
     * might be defined for the same value.
     */
    int x = 1;
    CLEANUP_VAL(&x, example2_cleanup);

    if (p == 1) {
        CLEANUP_VAL(&x, example2_cleanup2);
        // Here is example2_cleanup2(&x) called
        // Here is example2_cleanup(&x) called
        return 0;
    }

    // Here is example2_cleanup(&x) called
    return x;
}

/*
 * Cleanup function.
 */

static void example3_cleanup()
{
    printf("Cleanup example 3\n");
}

int example3(int p)
{
    /*
     * Call a function as soon as the function goes out-of-scope.
     */
    int x = 1;
    CLEANUP_FUNC(example3_cleanup);

    if (p == 0) {
        // Here is example3_cleanup() called
        return 0;
    }

    // Here is example3_cleanup() called
    return x;
}

/*
 * High level functions.
 */

int example4(int p)
{
    {
        AUTO_CLOSE FILE *f = fopen("my_file.txt", "r");
        (void)f; // NOTE: f has the type FILE *

        if (p == 0) {
            // "if (f != NULL) fclose(f)" is called
            return 0;
        }

        // "if (f != NULL) fclose(f)" is called
    }

    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    {
        M_LOCK_GUARD(&lock); // S_LOCK_GUARD can be used for pthread_spinlock_t
        if (p == 1) {
            // lock is unlocked
            return 2;
        }
        // lock is unlocked
    }

    return 0;
}

int main()
{
    example1(0);
    example2(1);
    example3(2);
    example4(3);

    return 0;
}
