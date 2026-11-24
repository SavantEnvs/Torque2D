// Fuzzes TAML xml deserialization: Taml::read(filename) with Taml::XmlFormat — the reader used for
// .taml scenes, module.taml and *.asset.taml files — including object construction and the
// console field setters of whatever classes the document names. TAML's public API reads files, so each
// input is written to a scratch file first.
#include "t2d_boot.h"

#include "platform/platform.h"
#include "persistence/taml/taml.h"
#include "sim/simObject.h"

extern "C" int LLVMFuzzerInitialize(int*, char***)
{
   t2d_boot();
   return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
   t2d_boot();
   const char* path = t2d_write_scratch(".taml", data, size);
   if (path == NULL)
      return 0;
   // The format comes from the scratch file's extension (Taml auto-format), as for game content.
   const unsigned mark = t2d_sim_mark();
   Taml taml;
   taml.setFormatMode(Taml::XmlFormat);
   SimObject* object = taml.read(path);
   if (object != NULL && !object->isProperlyAdded())
      delete object;
   t2d_sim_reset(mark);
   return 0;
}
