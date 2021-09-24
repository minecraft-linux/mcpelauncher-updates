#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <playapi/api.h>
#include <playapi/device_info.h>
#include <playapi/login.h>
#include <playapi/file_login_cache.h>
#include <string> 
#include <jnivm/vm.h>
#include <jnivm/env.h>

#ifdef NDEBUG
#define printf(...)
#endif

void (*mcpelauncher_preinithook)(const char*sym, void*val, void **orig);

signed char scrollval = 0;

char (*Mouse_feed_org)(char, char, short, short, short, short);
void Mouse_feed(char a, char b, short c, short d, short e, short f) {
    if(a == 4 && b != 0) {
        scrollval = (signed char&)b;
        if(scrollval < 0) {
            Mouse_feed_org(a, b, c, d, e, f);
        }
    } else {
        Mouse_feed_org(a, b, c, d, e, f);
    }
}
void (*enqueueButtonPressAndRelease)(void*q, unsigned int, int, int);
void enqueueButtonPressAndRelease_hook(void*q, unsigned int c, int d, int e) {
    enqueueButtonPressAndRelease(q, c, d, e);
    printf("enqueueButtonPressAndRelease_hook: %d %d %d\n", c, d, e);
    abort();
}
void (*MouseMapper_tick_org)(void*a,void*b,void*c);
void MouseMapper_tick(void*a,void*b,void*c) {
    MouseMapper_tick_org(a, b, c);
    if (scrollval > 0 && enqueueButtonPressAndRelease) {
        enqueueButtonPressAndRelease(b, 425082297, 0, 0);
        scrollval = 0;
    }
}
int (*JNI_OnLoad_) (void*,void*);
void* pthread_getattr_np_org;

void* mremap_fake(void *old_address, size_t old_size,
                    size_t new_size, int flags, ...) {
        return MAP_FAILED;
}
void*_ZNK11AppPlatform12isLANAllowedEv;
void*__ZNK11AppPlatform12isLANAllowedEv;

extern "C" void __attribute__ ((visibility ("default"))) mod_preinit() {
    auto h = dlopen("libmcpelauncher_mod.so", 0);
    if(!h) {
        return;
    }
    mcpelauncher_preinithook = (decltype(mcpelauncher_preinithook)) dlsym(h, "mcpelauncher_preinithook");
    dlclose(h);
    try {
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
                } else {
#if defined(__arm__) || defined(__aarch64__)
                    mcpelauncher_preinithook("_ZN5Mouse4feedEccssss", (void*)&Mouse_feed, (void**)&Mouse_feed_org);
                    mcpelauncher_preinithook("_ZN11MouseMapper4tickER15InputEventQueueR23ControllerIDtoClientMap", (void*)&MouseMapper_tick, (void**)&MouseMapper_tick_org);
#endif

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

                        auto appPlat = (void**)dlsym(mc, "_ZTV11AppPlatform");
                        auto raw = &appPlat[2];
                        auto othervt = *t;
#ifndef NDEBUG
                        printf("AppPlatform:\n");
                        for(int i = 0; raw[i] && raw[i] != (void*)0xffffffffffffffe8; i++) {
                            Dl_info data;
                            printf("%p (%s)\n", raw[i], dladdr(raw[i], &data) ? data.dli_sname : "(unknown)");
                        }
                        printf("othervt:\n");
                        for(int i = 0; othervt[i] && othervt[i] != (void*)0xffffffffffffffe8; i++) {
                            Dl_info data;
                            printf("%p (%s)\n", othervt[i], dladdr(othervt[i], &data) ? data.dli_sname : "(unknown)");    
                        }
#endif
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
                            if(othervt[i] == __ZNK11AppPlatform12isLANAllowedEv) {
                                othervt[i] = _ZNK11AppPlatform12isLANAllowedEv;
                                printf("Patched __ZNK11AppPlatform12isLANAllowedEv back to org\n");
                            }
                        }

                        dlclose(mc);    
                        return true;
                    };
                    mcpelauncher_preinithook("_ZNK11AppPlatform12isLANAllowedEv", __ZNK11AppPlatform12isLANAllowedEv, &_ZNK11AppPlatform12isLANAllowedEv);
                }
            }, [](std::exception_ptr e) {
            });
        }, [](std::exception_ptr e) {
        });
    } catch(...) {
    }
}

extern "C" __attribute__ ((visibility ("default"))) void mod_init() {
    auto mc = dlopen("libminecraftpe.so", 0);
    enqueueButtonPressAndRelease = (decltype(enqueueButtonPressAndRelease))dlsym(mc, "_ZN15InputEventQueue28enqueueButtonPressAndReleaseEj11FocusImpacti");
    dlclose(mc);
}
