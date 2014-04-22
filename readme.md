# Safecall -- return to abort() attacks

A small proof of concept using code introspection to make ROP exploitation
really really suck.

## Try it

    # Yes really, it contains some bash and awk to "fix" the assembly.
    bash ./safecall.c

If it doesn't implode, it works on your architecture (meaning it's i386/amd64)

    bash ./safecall.c ./output
    gdb ./output
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

## Further Research

Before trying to get this included in GCC, some other things I'd like to do...

### Function Fingerprints

To further limit the attacker's options for connecting functions not as they were intended,
the second two bytes should be computed from the number of arguments and the sizes of each
argument as well as that of the return value. Variadic functions should contain only their
mandatory parameters plus an extra parameter which is of a unique size. While the caller and
callee may not have exactly the same function declaration, we can reasonably expect them to
be binary compatible.

Should each function *begin* with it's fingerprint? This would allow them to be looked up in
shared libraries.

### _safe_myFunction()

In order to allow safecall to be used in shared libraries, functions will need to be
duplicated so that old/unsafe code can still call them. Emitting an unsafe version of each
function which calls it's safe counterpart will allow legacy code to continue working, albeit
slower. Perhaps modify the linker to patch these stub functions into the GOT?

### Verify performance

Is this going to kill performance? Worse than -fstack-protector ?

### Verify alignment

We should make sure these 4 bytes are always 4 byte aligned. Are function returns always
aligned? Can the compiler be made to emit only aligned calls? Is patching needed?

### Other architectures

How will this port to ARM, MIPS and others?
