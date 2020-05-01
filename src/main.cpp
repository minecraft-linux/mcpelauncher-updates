#include <mcpelauncher/app_platform.h>
#include <minecraft/symbols.h>
#include <dlfcn.h>
#include <log.h>
#include "client_app_platform.h"
#ifdef __i386__
#include "texel_aa_patch.h"
#endif

extern "C" void mod_init() {
    auto mc = dlopen("libminecraftpe.so", RTLD_LAZY);
    Log::info("MINECRAFT FOUND AT ", "%d", (intptr_t)mc);
    // mcpe::string::empty = (mcpe::string*) dlsym(mc, "_ZN4Util12EMPTY_STRINGE");
    Log::info("MOD", "Symbol init");
    minecraft_symbols_init(mc);
    Log::info("MOD", "Patch vtable");
    LauncherAppPlatform::initVtable(mc);
    ClientAppPlatform::initVtable(mc);
    Log::info("MOD", "Vtable patched");
#ifdef __i386__
    Log::info("TexelAAPatch", "install");
    TexelAAPatch::install(mc);
    Log::info("TexelAAPatch", "patched");
#endif
}