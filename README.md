# CRAII

Probably you are familiar with C++ RAII. Thanks to some gcc-extensions RAII can also be used
in C (up to some degree). This allows to implement a lock guard (like `std::lock_guard`)
or auto-closing files.

Some examples (too see a complete list, please see `main.c`):

## Lock Guard

A lock guard allows to do a lock from the point where it is
defined until the end of the block. The macro `M_LOCK_GUARD`
can be used for `pthread_mutex_t` and the macro `S_LOCK_GUARD`
for `pthread_spinlock_t`.

```C
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

...

int f(int x) 
{
    M_LOCK_GUARD(&lock);

    if (x == 0) {
        return g(x);
    }

    if (x == 1) {
        return h(x);
    }

    return 0;
}
```

This code is equivalent to:

```C
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

...

int f(int x) 
{
    pthread_mutex_lock(&lock);

    if (x == 0) {
        int res = g(x);
        pthread_mutex_unlock(&lock);
        return res;
    }

    if (x == 1) {
        int res = h(x);
        pthread_mutex_unlock(&lock);
        return res;
    }

    pthread_mutex_unlock(&lock);
    return 0;
}
```

# Auto Free

Sometimes its needed to allocate some buffer and just free it at the end
of the block. This can be simplified by using the `AUTO_FREE`-macro:

```C
int f(int x)
{
    AUTO_FREE(char *) str = malloc(100);

    if (x == 0) {
        get_name(str);
        printf("name=%s\n", str);
        return 0;
    }

    if (x == 1) {
        get_location(str);
        printf("location=%s\n", str);
        return 2;
    }

    get_country(str);
    printf("country=%s\n", str);
    return 12;
}
```

This code is equivalent to:

```C
int f(int x)
{
    char *str = malloc(100);

    if (x == 0) {
        get_name(str);
        printf("name=%s\n", str);
        if (str != NULL) {
            free(str);
        }
        return 0;
    }

    if (x == 1) {
        get_location(str);
        printf("location=%s\n", str);
        if (str != NULL) {
            free(str);
        }
        return 2;
    }

    get_country(str);
    printf("country=%s\n", str);
    if (str != NULL) {
        free(str);
    }
    return 12;
}
```

## Auto Close

The `AUTO_CLOSE` macro is very similar to the `AUTO_FREE` macro. It can
be used to automatically close files when the end of the block is reached.


```C
int my_func(const char *file, int x)
{
    AUTO_CLOSE(FILE *) fh = fopen(file, "w");

    if (x == 0) {
        fprintf(fh, "xyz");
        return 1;
    }

    if (x == 2) {
        fprintf(fh, "abc");
        return 12;
    }

    fprintf(fh, "hey");
    return x;
}
```

This code is equivalent to:

```C
int my_func(const char *file, int x)
{
    FILE * fh = fopen(file, "w");

    if (x == 0) {
        fprintf(fh, "xyz");
        if (fh != NULL) {
            fclose(fh);
        }
        return 1;
    }

    if (x == 2) {
        fprintf(fh, "abc");
        if (fh != NULL) {
            fclose(fh);
        }
        return 12;
    }

    fprintf(fh, "hey");
    if (fh != NULL) {
        fclose(fh);
    }
    return x;
}
```

## General Cleanup Macros

All the previous shown macros are based on some general cleanup macros. They
can be used for custom types and use-cases.

### Cleanup Variable

Cleaning up variables can make code way easier to read. E.g., memory pools
sometimes can be easier used by using this macro.

```C
struct my_struct *my_struct_create();
void my_struct_destroy(struct my_struct *my_struct);
int my_struct_do_something(struct my_struct *my_struct);

int f(int x)
{
    CLEANUP_VAR(my_struct_destroy) struct my_struct *ms = my_struct_create();

    if (x == 0) {
        return 2;
    }

    if (x == 2) {
        return my_struct_do_something(ms);
    }

    return 0;
}
```

This code is equivalent to:

```C
struct my_struct *my_struct_create();
void my_struct_destroy(struct my_struct *my_struct);
int my_struct_do_something(struct my_struct *my_struct);

int f(int x)
{
    struct my_struct *ms = my_struct_create();

    if (x == 0) {
        my_struct_destroy(ms);
        return 2;
    }

    if (x == 2) {
        int res = my_struct_do_something(ms);
        my_struct_destroy(ms);
        return res;
    }

    my_struct_destroy(ms);
    return 0;
}
```

### Cleanup Value

This macro works almost like `CLEANUP_VAR`, but it is bound to a value instead of
a variable. This macro can actually be applied multiple times to the same value.

```C
void cleanup_int1(int *x);
void cleanup_int2(int *x);

int f(int a)
{
    CLEANUP_VAL(&a, cleanup_int1);

    if (a > 0) {
        int x = a + 1;
        CLEANUP_VAL(&x, cleanup_int2);
        return x + 1;
    }

    CLEANUP_VAL(&a, cleanup_int2);
    return a + 1;
}
```

This code is equivalent to:

```C
void cleanup_int1(int *x);
void cleanup_int2(int *x);

int f(int a)
{
    if (a > 0) {
        int x = a + 1;
        int res = x + 1;
        cleanup_int2(&x);
        cleanup_int1(&a);
        return res;
    }

    int res = a + 1;
    cleanup_int2(&a);
    cleanup_int1(&a);
    return res;
}
```

### Cleanup Function

This is probably not very often needed, but it could be helpful:
A function without a parameter is executed whenever the current
block is left.

```C
void do_something();

int f(int a)
{
    CLEANUP_FUNC(do_something);

    if (a < 0) {
        return 12;
    }

    if (a == 3) {
        return 1;
    }

    return -1;
}
```

This code is equivalent to:

```C
void do_something();

int f(int a)
{
    if (a < 0) {
        do_something();
        return 12;
    }

    if (a == 3) {
        do_something();
        return 1;
    }

    do_something();
    return -1;
}
```

## Order of Execution

The order of the cleanup function execution is inverse to the definitions.

Example:
```C
CLEANUP_FUNC(f1);
CLEANUP_FUNC(f2);
CLEANUP_FUNC(f3);
```

This codes first executes `f3()`, then `f2()` and finally `f1()`.
This rule applies to all macros.
