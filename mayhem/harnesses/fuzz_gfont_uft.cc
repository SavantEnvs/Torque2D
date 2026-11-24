// Fuzzes the .uft glyph-cache reader: GFont::read(Stream&) (graphics/gFont.cc) — the format the
// resource manager loads for ".uft" fonts (fixed upstream in 337364c07f's area).
#include "t2d_boot.h"
#include "platform/platform.h"
#include "io/memstream.h"
#include "graphics/gFont.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { t2d_boot(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
   t2d_boot();
   MemStream stream((U32)size, (void*)data, true, false);
   GFont* font = new GFont;
   font->read(stream);
   delete font;
   return 0;
}
