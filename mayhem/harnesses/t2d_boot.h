// Headless (dedicated-mode) boot of the Torque2D engine for fuzz harnesses and the test runner.
#ifndef _T2D_BOOT_H_
#define _T2D_BOOT_H_
#include <stddef.h>
#include <stdint.h>

// Boots Con, Sim, the resource manager, etc. exactly like the platform main() does, but in
// dedicated mode (no SDL window / GL context). Safe to call more than once.
void t2d_boot();

// Writes data to a per-process scratch file under $TMPDIR (default /tmp) whose name ends in
// `suffix` and returns its path (static buffer). Returns NULL on failure.
const char* t2d_write_scratch(const char* suffix, const uint8_t* data, size_t size);

// Per-input isolation for harnesses that create SimObjects: t2d_sim_mark() before the input,
// t2d_sim_reset(mark) after it deletes every object registered since the mark (newest first) and
// rewinds the object-id counter, so names and ids an input registers can't affect the next input.
unsigned t2d_sim_mark();
void t2d_sim_reset(unsigned mark);
#endif
