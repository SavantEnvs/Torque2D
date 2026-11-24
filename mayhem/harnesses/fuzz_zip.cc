// Fuzzes the zip archive reader: Zip::ZipArchive::openArchive(Stream*) (end-of-central-directory and
// central-directory parsing) and then opens and reads every entry through openFile() (local file
// headers, extra fields, stored/deflate decompression).
#include "t2d_boot.h"
#include "platform/platform.h"
#include "io/memstream.h"
#include "io/zip/zipArchive.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { t2d_boot(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
   t2d_boot();
   MemStream* stream = new MemStream((U32)size, (void*)data, true, false);
   Zip::ZipArchive* archive = new Zip::ZipArchive;
   if (archive->openArchive(stream, Zip::ZipArchive::Read))
   {
      U8 buf[4096];
      for (U32 i = 0; i < archive->numEntries() && i < 256; ++i)
      {
         const Zip::CentralDir& cd = (*archive)[i];
         if (cd.mFilename == NULL)
            continue;
         Stream* entry = archive->openFile(cd.mFilename, Zip::ZipArchive::Read);
         if (entry == NULL)
            continue;
         U32 total = 0;
         while (entry->getStatus() == Stream::Ok && total < (16u << 20))
         {
            U32 before = entry->getPosition();
            if (!entry->read(sizeof(buf), buf))
               break;
            total += sizeof(buf);
            if (entry->getPosition() == before)
               break;
         }
         archive->closeFile(entry);
      }
   }
   delete archive;
   delete stream;
   return 0;
}
