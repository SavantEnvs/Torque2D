// Runs upstream's GoogleTest unit suite (engine/source/testing/tests/*.cc, compiled into the engine
// library) on a headless engine — the same tests `runAllUnitTests()` runs inside the engine, but
// without needing a window. Run from the repository root: the tests read fixtures under toybox/.
#include "t2d_boot.h"
#include "platform/platform.h"
#include "testing/unitTesting.h"

int main(int argc, char** argv)
{
   t2d_boot();
   testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
