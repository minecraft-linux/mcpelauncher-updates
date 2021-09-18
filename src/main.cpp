#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/mman.h>

// extern "C" void * mcpelauncher_hook(void* sym, void* hook, void** orig) {
//     printf("mcpelauncher_hook: something failed\n");
//     return nullptr;
// }


// extern "C" void mcpelauncher_preinithook2(const char*sym, void*val, void*user, void (*cbk)(void*user, void*orig)) {
//     printf("mcpelauncher_preinithook2: something failed\n");
// }
// extern "C" void mcpelauncher_preinithook(const char*sym, void*val, void **orig) {
//     printf("mcpelauncher_preinithook: something failed\n");
// }


#define HIDE __attribute__((visibility( "hidden" )))
// #define HIDE

HIDE void (*mcpelauncher_preinithook)(const char*sym, void*val, void **orig);

HIDE signed char scrollval = 0;

HIDE char (*Mouse_feed_org)(char, char, short, short, short, short);
HIDE void Mouse_feed(char a, char b, short c, short d, short e, short f) {
    if(a == 4 && b != 0) {
        scrollval = (signed char&)b;
        if(scrollval < 0) {
            Mouse_feed_org(a, b, c, d, e, f);
        }/*  else {
            printf("feed: %d\n", (int)scrollval);
        } */
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
HIDE int (*JNI_OnLoad_) (void*,void*);
HIDE void* pthread_getattr_np_org;

#include <jnivm/vm.h>
#include <jnivm/env.h>
HIDE void* mremap_fake(void *old_address, size_t old_size,
                    size_t new_size, int flags, ...) {
        return MAP_FAILED;
}
HIDE void*_ZNK11AppPlatform12isLANAllowedEv;
HIDE void*__ZNK11AppPlatform12isLANAllowedEv;

#include <playapi/api.h>
#include <playapi/device_info.h>
#include <playapi/login.h>
#include <playapi/file_login_cache.h>
#include <string> 
extern "C" void mod_preinit() {
    auto h = dlopen("libmcpelauncher_mod.so", 0);
    if(!h) {
        return;
    }
    mcpelauncher_preinithook = (decltype(mcpelauncher_preinithook)) dlsym(h, "mcpelauncher_preinithook");
    dlclose(h);
    playapi::device_info device;
    auto cachePath = getenv("GPLAY_TOKEN_CACHE_PATH");
    if(!cachePath) {
        return;
    }
    playapi::file_login_cache cache(cachePath);
    playapi::login_api login_api(device, cache);
    // login_api.set_token(getenv("GPLAY_MAIL"), getenv("GPLAY_TOKEN"));
    login_api.set_token("l@localhost", "a");
    playapi::api api(device);
    playapi::checkin_result checkinResult;
    checkinResult.time = 0;
    auto sid = getenv("GPLAY_CHECKIN_ANDROID_ID");
    if(!sid) {
        return;
    }
    checkinResult.android_id = std::stoull(sid);
    checkinResult.security_token = 0;
    auto devver = getenv("GPLAY_CHECKIN_DEVICE_DATA_VERSION_INFO");
    if(!devver) {
        return;
    }
    checkinResult.device_data_version_info = devver;
    api.set_checkin_data(checkinResult);
    api.set_auth(login_api)->call();
    api.details("com.mojang.minecraftpe")->call([&api](playapi::proto::finsky::response::ResponseWrapper&& resp) {
        auto details = resp.payload().detailsresponse().docv2();
        if(details.details().appdetails().versionstring() == "" || !details.details().appdetails().versioncode()) {
            abort();
        }
        api.delivery("com.mojang.minecraftpe", details.details().appdetails().versioncode(), std::string())->call([](playapi::proto::finsky::response::ResponseWrapper&& resp) {
            auto dd = resp.payload().deliveryresponse().appdeliverydata();
            auto url = (dd.has_gzippeddownloadurl() ? dd.gzippeddownloadurl() : dd.downloadurl());
            printf("Apk Url: %s\n", url.data());
            if(url == "") {
                printf("Invalid Url\n");
                abort();
            }
        }, [](std::exception_ptr e) {
            abort();
        });
    }, [](std::exception_ptr e) {
        abort();
    });
    // mcpelauncher_preinithook("JNI_OnLoad", (void*)+[](void*jvm,void*b) {
    //     printf("JNI_OnLoad: %p %p\n", jvm, JNI_OnLoad_);
    //     auto vm = jnivm::VM::FromJavaVM((JavaVM*)jvm);
    //     printf("jnivm::VM::FromJavaVM: %p\n", vm);
    //     auto c = vm->GetJavaVM();
    //     //->GetClass("java/lang/String");
    //     //printf("GetClass: %s\n", c->getName().data());
    //     printf("GetJavaVM: %p\n", c);
        
    //     if(JNI_OnLoad_)
    //     return JNI_OnLoad_(jvm, b);
    //     return 0;
    // }, (void**)&JNI_OnLoad_);
    
    // mcpelauncher_preinithook("_ZN5Mouse4feedEccssss", (void*)&Mouse_feed, (void**)&Mouse_feed_org);
    // // mcpelauncher_preinithook("android_main", (void*)&Mouse_feed, (void**)&Mouse_feed_org);
    // mcpelauncher_preinithook("_ZN11MouseMapper4tickER15InputEventQueueR23ControllerIDtoClientMap", (void*)&MouseMapper_tick, (void**)&MouseMapper_tick_org);
    // //mcpelauncher_preinithook("_ZN15InputEventQueue28enqueueButtonPressAndReleaseEj11FocusImpacti", (void*)&enqueueButtonPressAndRelease_hook, (void**)&enqueueButtonPressAndRelease);
    mcpelauncher_preinithook("pthread_getattr_np", (void*)+[](pthread_t th, pthread_attr_t* attr) -> int {
        return 1;
    }, &pthread_getattr_np_org);
    mcpelauncher_preinithook("pthread_attr_getstack", (void*)+[](pthread_attr_t* attr, void **stackaddr, size_t *stacksize) -> int {
        return 1;
    }, nullptr);
    mcpelauncher_preinithook("mremap", (void*)&mremap_fake, nullptr);
    //mcpelauncher_preinithook("_ZNK11AppPlatform17supportsScriptingEv", (void*)+[](void* t) -> bool { abort() ;return true; }, nullptr);
    __ZNK11AppPlatform12isLANAllowedEv = (void*)+[](void*** t) -> bool {
        auto mc = dlopen("libminecraftpe.so", 0);
    
        printf("AppPlatform:\n");
        // auto sym = dlsym(mc, "_ZN5Mouse4feedEccssss");
        auto appPlat = (void**)dlsym(mc, "_ZTV11AppPlatform");
        auto raw = &appPlat[2];
        for(int i = 0; raw[i] && raw[i] != (void*)0xffffffffffffffe8; i++) {
            Dl_info data;
            printf("%p (%s)\n", raw[i], dladdr(raw[i], &data) ? data.dli_sname : "(unknown)");
        }
        printf("othervt:\n");
        auto othervt = *t;
        for(int i = 0; othervt[i] && othervt[i] != (void*)0xffffffffffffffe8; i++) {
            Dl_info data;
            printf("%p (%s)\n", othervt[i], dladdr(othervt[i], &data) ? data.dli_sname : "(unknown)");    
        }
        auto _ZNK11AppPlatform19supportsFilePickingEv = (void**)dlsym(mc, "_ZNK11AppPlatform19supportsFilePickingEv");
        auto _ZNK11AppPlatform17supportsScriptingEv = (void**)dlsym(mc, "_ZNK11AppPlatform17supportsScriptingEv");
        auto _ZNK11AppPlatform25getPlatformUIScalingRulesEv = (void**)dlsym(mc, "_ZNK11AppPlatform25getPlatformUIScalingRulesEv");
        auto _ZNK11AppPlatform18supportsWorldShareEv = (void**)dlsym(mc, "_ZNK11AppPlatform18supportsWorldShareEv");
        auto _ZNK11AppPlatform20getLevelSaveIntervalEv = (void**)dlsym(mc, "_ZNK11AppPlatform20getLevelSaveIntervalEv");
        auto _ZNK11AppPlatform10getEditionEv = (void**)dlsym(mc, "_ZNK11AppPlatform10getEditionEv");
        auto _ZNK11AppPlatform27getDefaultNetworkMaxPlayersEv = (void**)dlsym(mc, "_ZNK11AppPlatform27getDefaultNetworkMaxPlayersEv");
        auto _ZNK11AppPlatform23supports3rdPartyServersEv = (void**)dlsym(mc, "_ZNK11AppPlatform23supports3rdPartyServersEv");
        auto _ZNK11AppPlatform29allowsResourcePackDevelopmentEv = (void**)dlsym(mc, "_ZNK11AppPlatform29allowsResourcePackDevelopmentEv");
        auto _ZNK11AppPlatform30supportsAutoSaveOnDBCompactionEv = (void**)dlsym(mc, "_ZNK11AppPlatform30supportsAutoSaveOnDBCompactionEv");
        auto _ZNK11AppPlatform23isAutoCompactionEnabledEv = (void**)dlsym(mc, "_ZNK11AppPlatform23isAutoCompactionEnabledEv");
        auto _ZN11AppPlatform22uiOpenRenderDistScalarEv =  (void**)dlsym(mc, "_ZN11AppPlatform22uiOpenRenderDistScalarEv");
        
        for(int i = 0; raw[i] && raw[i] != (void*)0xffffffffffffffe8; i++) {
            if(raw[i] == _ZNK11AppPlatform19supportsFilePickingEv) {
                othervt[i] = (void*) +[](void*t) -> bool {
                    printf("_ZNK11AppPlatform19supportsFilePickingEv called\n");
                    return true;
                };
                printf("Patched _ZNK11AppPlatform19supportsFilePickingEv\n");
            }
            if(raw[i] == _ZNK11AppPlatform17supportsScriptingEv) {
                othervt[i] = (void*) +[](void*t) -> bool {
                    printf("_ZNK11AppPlatform17supportsScriptingEv called\n");
                    return true;
                };
                printf("Patched _ZNK11AppPlatform17supportsScriptingEv\n");
            }
            if(raw[i] == _ZNK11AppPlatform25getPlatformUIScalingRulesEv) {
                othervt[i] = (void*) +[](void*t) -> int {
                    printf("_ZNK11AppPlatform25getPlatformUIScalingRulesEv called\n");
                    return 0;
                };
                printf("Patched _ZNK11AppPlatform25getPlatformUIScalingRulesEv\n");
            }
            if(raw[i] == _ZNK11AppPlatform18supportsWorldShareEv) {
                othervt[i] = (void*) +[](void*t) -> bool {
                    printf("_ZNK11AppPlatform18supportsWorldShareEv called\n");
                    return true;
                };
                printf("Patched _ZNK11AppPlatform18supportsWorldShareEv\n");
            }
            // if(raw[i] == _ZNK11AppPlatform20getLevelSaveIntervalEv) {
            //     othervt[i] = (void*) +[](void*t) -> int {
            //         printf("_ZNK11AppPlatform20getLevelSaveIntervalEv called\n");
            //         return -1;
            //     };
            //     printf("Patched _ZNK11AppPlatform20getLevelSaveIntervalEv\n");
            // }
            if(raw[i] == _ZNK11AppPlatform10getEditionEv) {
                othervt[i] = (void*) +[](void*t) -> std::string {
                    printf("_ZNK11AppPlatform10getEditionEv called\n");
                    return "win10";
                };
                printf("Patched _ZNK11AppPlatform10getEditionEv\n");
            }
            if(raw[i] == _ZNK11AppPlatform27getDefaultNetworkMaxPlayersEv) {
                othervt[i] = (void*) +[](void*t) -> int {
                    printf("_ZNK11AppPlatform27getDefaultNetworkMaxPlayersEv called\n");
                    return 100;
                };
                printf("Patched _ZNK11AppPlatform27getDefaultNetworkMaxPlayersEv\n");
            }
            // if(raw[i] == _ZNK11AppPlatform23supports3rdPartyServersEv) {
            //     othervt[i] = (void*) +[](void*t) -> bool {
            //         printf("_ZNK11AppPlatform23supports3rdPartyServersEv called\n");
            //         return false;
            //     };
            //     printf("Patched _ZNK11AppPlatform23supports3rdPartyServersEv\n");
            // }
            if(raw[i] == _ZNK11AppPlatform29allowsResourcePackDevelopmentEv) {
                othervt[i] = (void*) +[](void*t) -> bool {
                    printf("_ZNK11AppPlatform29allowsResourcePackDevelopmentEv called\n");
                    return false;
                };
                printf("Patched _ZNK11AppPlatform29allowsResourcePackDevelopmentEv\n");
            }
            if(raw[i] == _ZN11AppPlatform22uiOpenRenderDistScalarEv) {
                othervt[i] = (void*) +[](void*t) -> int {
                    printf("_ZN11AppPlatform22uiOpenRenderDistScalarEv called\n");
                    return 512;
                };
                printf("Patched _ZN11AppPlatform22uiOpenRenderDistScalarEv\n");
            }
            // if(raw[i] == _ZNK11AppPlatform30supportsAutoSaveOnDBCompactionEv) {
            //     othervt[i] = (void*) +[](void*t) -> bool {
            //         printf("_ZNK11AppPlatform30supportsAutoSaveOnDBCompactionEv called\n");
            //         return false;
            //     };
            //     printf("Patched _ZNK11AppPlatform30supportsAutoSaveOnDBCompactionEv\n");
            // }
            // if(raw[i] == _ZNK11AppPlatform23isAutoCompactionEnabledEv) {
            //     othervt[i] = (void*) +[](void*t) -> bool {
            //         printf("_ZNK11AppPlatform23isAutoCompactionEnabledEv called\n");
            //         return false;
            //     };
            //     printf("Patched _ZNK11AppPlatform23isAutoCompactionEnabledEv\n");
            // }
            if(othervt[i] == __ZNK11AppPlatform12isLANAllowedEv) {
                othervt[i] = _ZNK11AppPlatform12isLANAllowedEv;
                printf("Patched __ZNK11AppPlatform12isLANAllowedEv back to org\n");
            }
        }
        
        dlclose(mc);    
        return true;
    };
    mcpelauncher_preinithook("_ZNK11AppPlatform12isLANAllowedEv", __ZNK11AppPlatform12isLANAllowedEv, &_ZNK11AppPlatform12isLANAllowedEv);
    // mcpelauncher_preinithook("_ZNK11AppPlatform28requiresXboxLiveSigninToPlayEv", (void*)+[](void* t) -> bool { return true; }, nullptr);
    
}

extern "C" void mod_init() {

    auto mc = dlopen("libminecraftpe.so", 0);
    
    // // auto sym = dlsym(mc, "_ZN5Mouse4feedEccssss");
    // auto appPlat = (void**)dlsym(mc, "_ZTV11AppPlatform");
    // auto raw = &appPlat[2];
    // if(mcpelauncher_hook(sym, (void*)&Mouse_feed, (void**)&Mouse_feed_org)) {

    // }
    enqueueButtonPressAndRelease = (decltype(enqueueButtonPressAndRelease))dlsym(mc, "_ZN15InputEventQueue28enqueueButtonPressAndReleaseEj11FocusImpacti");
    if(!enqueueButtonPressAndRelease) {
        abort();
    }
    dlclose(mc);
}