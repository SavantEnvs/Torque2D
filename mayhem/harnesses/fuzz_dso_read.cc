// Fuzzes the compiled-script (.dso) loader: CodeBlock::read(fileName, Stream&) (console/codeBlock.cc)
// — version check, global/function string tables, float tables, code stream and identifier
// patch-up table. Load-only: the bytecode is not executed.
#include "t2d_boot.h"
#include "platform/platform.h"
#include "io/memstream.h"
#include "console/console.h"
#include "console/compiler.h"
#include "console/codeBlock.h"
#include "string/stringTable.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { t2d_boot(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
   t2d_boot();
   MemStream stream((U32)size, (void*)data, true, false);
   CodeBlock* block = new CodeBlock;
   block->read(StringTable->insert("fuzz.cs"), stream);
   delete block;
   return 0;
}
