// Fuzzes the AngelCode .fnt text parser: font::BitmapFont::parseFont(Stream&) (bitmapFont/BitmapFont.cc),
// which FontAsset uses to load bitmap fonts.
#include "t2d_boot.h"
#include "platform/platform.h"
#include "io/memstream.h"
#include "bitmapFont/BitmapFont.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { t2d_boot(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
   t2d_boot();
   MemStream stream((U32)size, (void*)data, true, false);
   font::BitmapFont font;
   font.parseFont(stream);
   return 0;
}
