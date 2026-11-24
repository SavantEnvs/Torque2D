// Headless engine boot shared by every harness (see t2d_boot.h).
#include "t2d_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "platform/platform.h"
#include "platformX86UNIX/platformX86UNIX.h"
#include "platformX86UNIX/x86UNIXState.h"
#include "console/console.h"
#include "graphics/TextureManager.h"
#include "sim/simBase.h"

extern x86UNIXPlatformState* x86UNIXState;
extern bool initializeLibraries();
namespace Sim { extern U32 gNextObjectId; }

void t2d_boot()
{
   static bool booted = false;
   if (booted)
      return;
   booted = true;
   x86UNIXState = new x86UNIXPlatformState;
   x86UNIXState->setDedicated(true);
   // No GL context exists headless: tell the texture manager not to upload textures (its own
   // switch; TextureManager::refresh() otherwise AssertISVs on the missing GL texture name when a
   // .uft font or an image registers a texture).
   TextureManager::mDGLRender = false;
   if (!initializeLibraries())
   {
      fprintf(stderr, "t2d_boot: initializeLibraries() failed\n");
      abort();
   }
}

const char* t2d_write_scratch(const char* suffix, const uint8_t* data, size_t size)
{
   static char path[1024];
   const char* tmp = getenv("TMPDIR");
   if (tmp == NULL || *tmp == '\0')
      tmp = "/tmp";
   snprintf(path, sizeof(path), "%s/t2d_fuzz_%d%s", tmp, (int)getpid(), suffix);
   FILE* f = fopen(path, "wb");
   if (f == NULL)
      return NULL;
   if (size && fwrite(data, 1, size, f) != size)
   {
      fclose(f);
      return NULL;
   }
   fclose(f);
   return path;
}

unsigned t2d_sim_mark()
{
   return Sim::gNextObjectId;
}

void t2d_sim_reset(unsigned mark)
{
   for (U32 id = Sim::gNextObjectId; id > mark; --id)
   {
      SimObject* object = Sim::findObject((SimObjectId)(id - 1));
      if (object != NULL)
         object->deleteObject();
   }
   Sim::gNextObjectId = mark;
}
