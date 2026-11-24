// Fuzzes the BMP image loader: GBitmap::readMSBmp(Stream&) (graphics/bitmapBmp.cc).
#include "t2d_boot.h"
#include "platform/platform.h"
#include "io/memstream.h"
#include "graphics/gBitmap.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { t2d_boot(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
   t2d_boot();
   // readMSBmp() reads the 14-byte BITMAPFILEHEADER + 40-byte BITMAPINFOHEADER that every BMP
   // starts with without checking the stream reads; on a shorter input those header fields stay
   // UNINITIALIZED stack memory, so the outcome would depend on stack garbage rather than on the
   // input (non-reproducible). Inputs with complete headers reach every check-free path below it.
   if (size < 54)
      return -1;
   MemStream stream((U32)size, (void*)data, true, false);
   GBitmap bitmap;
   bitmap.readMSBmp(stream);
   return 0;
}
