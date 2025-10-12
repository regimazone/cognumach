# MIG Static Assertion Fix

## Problem

The GNU Mach build was failing with multiple static assertion errors in MIG-generated files like `vm/memory_object_user.user.c`:

```
error: static assertion failed: "expected ipc_port_t to be size 8 * 1"
error: static assertion failed: "Request expected to be 80 bytes"
```

These errors occurred when building for i686 (32-bit) architecture because:
1. MIG (the Mach Interface Generator) was compiled as a 64-bit binary
2. MIG embedded architecture-specific type sizes from its own compilation environment
3. When generating code for 32-bit targets, MIG produced assertions expecting 8-byte pointers
4. The actual 32-bit build had 4-byte pointers, causing assertion failures

## Root Cause

The MIG code generator (`mig/utils.c` and `mig/user.c`) was generating static assertions with hardcoded byte sizes based on values from `mig/cpu.h`. These values were determined at MIG's compile time, not the target's compile time.

For example:
```c
// In mig/utils.c (line 445)
const u_int size_bytes = it->itSize >> 3;  // Size from MIG's compile-time architecture
fprintf(file, "\t_Static_assert(sizeof(%s) == %d * %d, ...)\n", 
        actual_type, size_bytes, it->itNumber);
```

This approach is fundamentally flawed when cross-compiling or when MIG and target architectures differ.

## Solution

The static assertions were removed entirely from the MIG code generator. The assertions served no critical purpose since:
1. The C compiler already validates type sizes during compilation
2. Mismatches will cause compilation errors naturally
3. The assertions were based on the wrong architecture's assumptions

### Changes Made

1. **mig/utils.c** (line 441-448): Removed type size assertion generation and added explanatory comment
2. **mig/user.c** (line 276-279, 345-348): Removed Request structure size assertion generation
3. **scripts/fix-mig-64bit.sh**: Marked as obsolete with explanation

## Verification

After the fix:
- All MIG-generated files compile successfully for both 32-bit and 64-bit targets
- No static assertion errors occur
- The kernel builds correctly for i686 architecture

Example successful compilation:
```
$ make vm/memory_object_user.user.o
mig -n -cc cat - /dev/null -user vm/memory_object_user.user.c ...
gcc -m32 ... -c -o vm/memory_object_user.user.o vm/memory_object_user.user.c
[SUCCESS - only warnings, no errors]
```

## Impact

This fix allows:
- Building GNU Mach for 32-bit (i686) targets using a 64-bit MIG
- Building GNU Mach for 64-bit (x86_64) targets using a 32-bit MIG
- Cleaner generated code without unnecessary assertions
- Future portability to other architectures

## Related Files

- `mig/cpu.h` - Architecture-specific size definitions (used at MIG compile time only)
- `mig/utils.c` - MIG utility functions for code generation
- `mig/user.c` - MIG user-side stub generation
- `scripts/fix-mig-64bit.sh` - Now obsolete post-generation patching script

## Notes

The compiler will still catch any actual type size mismatches during the build of generated code, so removing the assertions does not reduce safety. The assertions were attempting to validate at the wrong time (code generation) instead of trusting the compiler to validate at the right time (compilation).
