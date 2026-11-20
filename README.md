# Two binaries, the same libraries, different results

Libraries B and C both use the inline function `a()` from library A. Binaries D and F both link B and C. D and F print different results for the same call, `b()`, with the same inputs.

Every file is compiled once, with one set of flags: `-O3 -march=haswell -mtune=generic`. B and C are static libraries. D and F are built from the same `main.cpp` and differ only in the order of the two libraries on their link line.

## Build and run

```sh
cmake -S . -B build
cmake --build build
./build/D
./build/F
```

```
b() = 4.544999999999999 (0x1.22e147ae147adp+2)        <- D
b() = 4.5449999999999982 (0x1.22e147ae147acp+2)       <- F
```

## Why

- **B and C each compile their own `a()`.** Call them `a_B()` and `a_C()`. Both come from the same source in `A.hpp`.
- **The compiler builds them differently.** `a()` calls a second function of A, `scaledSum()`, and adds a number. In `B.cpp` nothing else calls `scaledSum()`, so GCC folds it into `a_B()`. Its final multiply then sits next to the addition, and GCC fuses the two into one instruction with one rounding. In `C.cpp` it has another caller, so GCC keeps it separate. In `a_C()` the multiply and the addition stay apart and round twice.
- **Each binary keeps one `a()`, the first its linker meets.** D keeps `a_B()` and F keeps `a_C()`. The linker decides this separately for each binary.
- **`b()` runs whichever `a()` its binary kept.** `b()` itself is the same object code in D and F.

## Inspect the differences

**1. B and C hold different compilations of `a()`.** The second column is the size in bytes. `W` marks a weak symbol: one the linker may replace with another copy.

```sh
nm -S -C build/libB.a build/libC.a | grep ' a('
```

```
0000000000000000 00000000000000ff W a(double const*, double const*, double, double)
0000000000000000 000000000000001e W a(double const*, double const*, double, double)
```

**2. Each binary kept one of them.** D holds the 0xff-byte `a_B()`, F the 0x1e-byte `a_C()`.

```sh
nm -S -C build/D build/F | grep ' a('
```

**3. The two differ in how they round.** `_Z1aPKdS0_dd` is the linker's name for `a()`. `a_B()` in D holds 17 fused multiply-adds (`vfmadd...`), one of which fuses the final multiply with the addition. `a_C()` in F holds none: it calls `scaledSum()`, which multiplies, and then adds with `vaddsd`.

```sh
objdump -d --no-show-raw-insn --disassemble=_Z1aPKdS0_dd build/D | grep -c vfmadd
objdump -d --no-show-raw-insn --disassemble=_Z1aPKdS0_dd build/F
```

**4. Only the library order differs between the two link lines.**

```sh
cat build/CMakeFiles/D.dir/link.txt build/CMakeFiles/F.dir/link.txt
```

```
c++ CMakeFiles/D.dir/main.cpp.o -o D  libB.a libC.a
c++ CMakeFiles/F.dir/main.cpp.o -o F  libC.a libB.a
```

**5. The order comes from `CMakeLists.txt`.** Change `target_link_libraries(F PRIVATE C B)` to `B C` and rebuild: F then prints what D prints.

## What changes the outcome

Measured with GCC 9.4 on x86-64. Use a fresh build folder for each setting.

| Setting | `a_B` and `a_C` | `b()` in D and F |
|---|---|---|
| Default, `-O3 -march=haswell -mtune=generic` | 0xff and 0x1e bytes | differ in the last bit |
| `-DTOY_FLAGS="-O2 -march=haswell -mtune=generic"` | 0x71 and 0x1e bytes | differ in the last bit |
| `-DTOY_FLAGS="-O3 -march=native"` | 0xff and 0x1e bytes | differ in the last bit |
| `-DTOY_FLAGS="-O3"`, no fused multiply-add available | 0x169 and 0x1e bytes | equal |
| `-DCMAKE_CXX_COMPILER=clang++-17` | both 0xb7 bytes | equal |

Without fused multiply-add the two copies still differ in size, but both round twice, so the results agree. Clang folds `scaledSum()` into both copies. The CPU must support fused multiply-add (x86-64 from Haswell on).
