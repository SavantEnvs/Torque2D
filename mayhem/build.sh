#!/usr/bin/env bash
# mayhem/build.sh — builds the Torque2D engine as a static library (mayhem/CMakeLists.txt reuses
# upstream's own source lists) twice: sanitized + libFuzzer harnesses, and plain for the unit tests.
set -euo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
: "${SRC:=/mayhem}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS SRC

cd "$SRC"
OUT="$SRC/mayhem/out"
mkdir -p "$OUT"

# 1) Unit-test runner, NORMAL flags (the functional oracle for test.sh).
cmake -S mayhem -B mayhem/build-test -DT2D_FUZZ=OFF -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_C_FLAGS="$COVERAGE_FLAGS" -DCMAKE_CXX_FLAGS="$COVERAGE_FLAGS"
cmake --build mayhem/build-test -j"$MAYHEM_JOBS" --target t2d_unittests
cp mayhem/build-test/t2d_unittests "$OUT/t2d_unittests"

# 2) Engine + vendored libs + harnesses with the sanitizers and DWARF-3, fuzzers and standalones.
#    -O1: the engine is large; keep sanitized code reasonably fast.
#    UBSan's `alignment`, `vptr` and `nonnull-attribute` checks are relaxed (only when sanitizers are on).
#    nonnull-attribute: dQsort() passes qsort a NULL base for an empty vector, so deleting any empty
#    SimSet (SimSet::onRemove, simSet.cc:175) halts. alignment: the chunk allocators
#    (DataChunker, used by the StringTable during boot) hand out 4-byte-aligned nodes, so it fires
#    at startup on every input before any input is read. vptr: the console field registry computes
#    field offsets with `Offset(x, cls)` = &((cls*)1)->x (platform/types.h), and vptr
#    instrumentation dereferences that fake object (SEGV at 0x1 in OpenFolderDialog::initPersistFields
#    during boot). ASan and the rest of UBSan stay halting.
RELAX=""
case "$SANITIZER_FLAGS" in *undefined*|*alignment*) RELAX="-fno-sanitize=alignment,vptr,nonnull-attribute" ;; esac
#    fuzzer-no-link: SanitizerCoverage on the whole engine, so libFuzzer/Mayhem get edge feedback.
#    mayhem/lsan_off.cc (__lsan_is_turned_off, compiled with the sanitizer flags) is linked into every
#    fuzz and -standalone binary: leak detection off at build time, ASan + UBSan stay on.
cmake -S mayhem -B mayhem/build-fuzz -DT2D_FUZZ=ON -DCMAKE_BUILD_TYPE=None \
      -DT2D_LSAN_OFF="$SRC/mayhem/lsan_off.cc" \
      -DCMAKE_C_FLAGS="-O1 -fsanitize=fuzzer-no-link $SANITIZER_FLAGS $RELAX $DEBUG_FLAGS" \
      -DCMAKE_CXX_FLAGS="-O1 -fsanitize=fuzzer-no-link $SANITIZER_FLAGS $RELAX $DEBUG_FLAGS"
cmake --build mayhem/build-fuzz -j"$MAYHEM_JOBS"
for f in mayhem/build-fuzz/fuzz_*; do
  [ -x "$f" ] && cp "$f" "$OUT/"
done
ls -l "$OUT"
