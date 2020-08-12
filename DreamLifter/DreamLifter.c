// DreamLifter.c: load the Type-C controller library and kick start it

#include <DreamLifter.h>
#include <MinUmdfLoaderInterface.h>

int main(int argc, char* argv[])
{
    // What it needs to do:
    // 1. Load TyC.dll via LoadLibrary
    // 2. Find WUDF entry point
    // 3. Initialize some structs
    // 4. Kicks in

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    
    return 0;
}
