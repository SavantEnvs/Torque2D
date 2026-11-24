// Fuzzes the PNG image loader: GBitmap::readPNG(Stream&) (graphics/bitmapPng.cc over vendored lpng).
#include "t2d_boot.h"
#include "platform/platform.h"
#include "io/memstream.h"
#include "graphics/gBitmap.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { t2d_boot(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
   t2d_boot();
   MemStream stream((U32)size, (void*)data, true, false);
   GBitmap bitmap;
   bitmap.readPNG(stream);
   return 0;
}
