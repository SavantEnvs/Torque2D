// Fuzzes the TorqueScript front end: CodeBlock::compile() runs the lexer (CMDscan), the parser
// (cmdgram), AST construction and the bytecode compiler on the input script and writes the
// resulting .dso to a scratch file. Compile-only: nothing is executed, so the fuzzed script can't
// call quit()/file/network console functions and every input is independent.
#include "t2d_boot.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "platform/platform.h"
#include "console/console.h"
#include "console/compiler.h"
#include "console/codeBlock.h"
#include "string/stringTable.h"

extern "C" int LLVMFuzzerInitialize(int*, char***) { t2d_boot(); return 0; }

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
   t2d_boot();
   char* script = (char*)malloc(size + 1);
   memcpy(script, data, size);
   script[size] = '\0';
   char dso[1024];
   const char* tmp = getenv("TMPDIR");
   snprintf(dso, sizeof(dso), "%s/t2d_fuzz_%d.cs.dso", (tmp && *tmp) ? tmp : "/tmp", (int)getpid());
   CodeBlock* block = new CodeBlock;
   block->compile(dso, StringTable->insert("fuzz.cs"), script);
   delete block;
   free(script);
   return 0;
}
