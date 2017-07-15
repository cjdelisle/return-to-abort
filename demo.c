#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

void __stack_chk_fail();

static void returnCheck(int code) {
    unsigned int* realP = ((unsigned int*)__builtin_return_address(1) );
    unsigned int real = realP[0];
    unsigned int test = (code<<16 | 0x02eb);
    if (test != real) {
        printf("code mismatch, expected %04x got %04x at location %016lx\n",
            test, real, (unsigned long) realP);
        __stack_chk_fail();
    }
}

void win() {
    execve("/bin/sh", 0, 0);
}

int test()
{
    printf("Hello World!\n");
    SAFE_RETURN(0);
}

int vulnerable(int len)
{
    char stackbuffer[64];
    read(0, stackbuffer, len);
    SAFE_RETURN(0);
}

int main(int argc, char const *argv[])
{
    test();
    if (argc > 1) {
        vulnerable(atoi(argv[1]));
    }

    // Because main() is not safe-called (it would require changing libc)
    // There is no safe way to return from here.
    // The attacker could easily have made overflowed the vulnerable function to
    // return to main() and then wrote the return pointer from main() to return to win().
    printf("Success, intentionally aborting because libc doesn't safecall us...\n");
    abort();
}