# Safecall -- return to abort() attacks

A small proof of concept using code introspection to make ROP exploitation
really really suck.

**NOTE:** This idea is not usable in practice, see issues below.

## Try it

    make
    ./demo

If it doesn't implode, it works on your architecture (meaning it's amd64)

    gdb ./demo
    (gdb) disassemble main
    (gdb) disassemble test
    # check it out

## How it works
Stack overflow attacks depend on being able to overwrite the return pointer which points
to the code which called the current function. When the function returns, instead of
returning back to it's caller, it returns to some evil place determined by the attacker.
The `NX bit` is a great help because nolonger can *data* become executable code but still
the attacker can win using what is known as `Return Oriented Programming`, IE: finding little
snippets of code just before additional return instructions which do what he wants.
Like piecing together a ransom note from letters cut out of magazines, the attacker can
piece together evil code from little pieces of the ends of many functions.

The `Stack Smashing Protector` helps a lot by checking that a cookie has not been overwritten
prior to returning. The logic is that if the cookie resides between the vulnerable buffer
and the return pointer, the attacker can't change the return pointer without overwriting
the cookie first. This cookie must be unpredictable and secret for the duration of the
program's operation, otherwise the attacker will simply overwrite it with the same value.
The flaw with the cookie approach is that if the attacker can find a bug which *reads*
past the end of a buffer and makes the result available to him, he can then prepare an
attack which defeats the cookie.

Safecall is a modification of the calling convention which is used to call the function.
The caller follows the call ASM instruction with 4 predictable bytes, in this case the
first two bytes constitute a *jmp* instruction which skips over the second two, the second
two are never executed. Before returning, a function checks the actual code at the return
pointer and expects to see these 4 predictable bytes. Because the value must exist in
*executable* memory which on modern systems is non-writable, this protection cannot be
foiled by overwriting a value in memory. An attacker attempting to use ROP can only look
for code to piece together from places where these 4 unique bytes appear (because of the
uniqueness of the bytes, that means almost certainly these will be function calls).
Therefor the attacker can make one function return to a function which does not call it but
he cannot assemble individual pieces of assembly by returning to any place in the executable.

## Issues

Thanks to Zach Riggle ( @zachriggle ) for helping tear this idea apart.
While the idea is technically safe, there are so many constraints on it's usage that it
cannot reasonably be implemented.

### Function pointers are impossible
In this implementation, the fingerprint of the function is it's hash, therefore taking
a function pointer and calling it later would not be possible.

### Any unsafe call is an attack point
The initial version of this project had a main() function safe-calling a test() function
which was vulnerable. Unfortunately in this case the example was not safe because the
attacker can overwrite many frames of the stack, not just one, so he could simply make the
test() function return properly to main() function and then make the main function return
to the attacker's place of choice.

This was mitigated by making the example abort() rather than return. In order to put it
in production libc _start() would need to use a safecall.

#### Calling a "safe" function from an "unsafe" one is impossible
One might consider that if there's a safe function, one can simply use a sort of
trampoline function to call it from an unprotected function. Not so fast, if the
trampoline exists in memory then the attacker can return to it and then setup the
next return pointer to point to his code of choice. This seriously harms the usability
of this kind of solution.

### SafeStack has already solved the problem
This solution, while interesting, is in almost no way, better than SafeStack which is
implemented in clang: https://clang.llvm.org/docs/SafeStack.html
