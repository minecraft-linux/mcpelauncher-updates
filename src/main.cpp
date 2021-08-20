#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

extern "C" void * mcpelauncher_hook(void* sym, void* hook, void** orig) {
    printf("mcpelauncher_hook: something failed\n");
    return nullptr;
}

extern "C" void mcpelauncher_preinithook2(const char*sym, void*val, void*user, void (*cbk)(void*user, void*orig)) {
    printf("mcpelauncher_preinithook2: something failed\n");
}
extern "C" void mcpelauncher_preinithook(const char*sym, void*val, void **orig) {
    printf("mcpelauncher_preinithook: something failed\n");
}

// #define HIDE __attribute__((visibility( "hidden" )))
#define HIDE

HIDE signed char scrollval = 0;

HIDE char (*Mouse_feed_org)(char, char, short, short, short, short);
HIDE void Mouse_feed(char a, char b, short c, short d, short e, short f) {
    if(a == 4 && b != 0) {
        scrollval = (signed char&)b;
        if(scrollval < 0) {
            Mouse_feed_org(a, b, c, d, e, f);
        } else {
            printf("feed: %d\n", (int)scrollval);
        }
    } else {
        Mouse_feed_org(a, b, c, d, e, f);
    }
}
HIDE void (*enqueueButtonPressAndRelease)(void*q, unsigned int, int, int);
HIDE void enqueueButtonPressAndRelease_hook(void*q, unsigned int c, int d, int e) {
    enqueueButtonPressAndRelease(q, c, d, e);
    printf("enqueueButtonPressAndRelease_hook: %d %d %d\n", c, d, e);
    abort();
}
HIDE void (*MouseMapper_tick_org)(void*a,void*b,void*c);
HIDE void MouseMapper_tick(void*a,void*b,void*c) {
    // scrollval = 0;
    MouseMapper_tick_org(a, b, c);
    if (scrollval > 0 && enqueueButtonPressAndRelease) {
        enqueueButtonPressAndRelease(b, 425082297, 0, 0);
        scrollval = 0;
        // enqueueButtonPressAndRelease(b, 1, 0, 425082297);
        //enqueueButtonPressAndRelease(b, 1, 0, 425082297);
    }
}
HIDE int (*JNI_OnLoad) (void*,void*);

extern "C" void mod_preinit() {
    mcpelauncher_preinithook("JNI_OnLoad", (void*)+[](void*jvm,void*b) {
        printf("JNI_OnLoad: %p %p\n", jvm, JNI_OnLoad);
        if(JNI_OnLoad)
        return JNI_OnLoad(jvm, b);
        return 0;
    }, (void**)&JNI_OnLoad);
    
    mcpelauncher_preinithook("_ZN5Mouse4feedEccssss", (void*)&Mouse_feed, (void**)&Mouse_feed_org);
    // mcpelauncher_preinithook("android_main", (void*)&Mouse_feed, (void**)&Mouse_feed_org);
    mcpelauncher_preinithook("_ZN11MouseMapper4tickER15InputEventQueueR23ControllerIDtoClientMap", (void*)&MouseMapper_tick, (void**)&MouseMapper_tick_org);
    //mcpelauncher_preinithook("_ZN15InputEventQueue28enqueueButtonPressAndReleaseEj11FocusImpacti", (void*)&enqueueButtonPressAndRelease_hook, (void**)&enqueueButtonPressAndRelease);
}

extern "C" void mod_init() {
    auto mc = dlopen("libminecraftpe.so", 0);
    // auto sym = dlsym(mc, "_ZN5Mouse4feedEccssss");
    // if(mcpelauncher_hook(sym, (void*)&Mouse_feed, (void**)&Mouse_feed_org)) {

    // }
    enqueueButtonPressAndRelease = (decltype(enqueueButtonPressAndRelease))dlsym(mc, "_ZN15InputEventQueue28enqueueButtonPressAndReleaseEj11FocusImpacti");
    if(!enqueueButtonPressAndRelease) {
        abort();
    }
    dlclose(mc);
}