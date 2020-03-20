#include "client_app_platform.h"

#include <iostream>
#include <minecraft/ImagePickingCallback.h>
#include <minecraft/FilePickerSettings.h>
#include <minecraft/MinecraftGame.h>
#include <minecraft/legacy/AppPlatform.h>
#include <hybris/dlfcn.h>
#include <minecraft/Keyboard.h>
#include <mcpelauncher/minecraft_version.h>

const char* ClientAppPlatform::TAG = "ClientAppPlatform";

void ClientAppPlatform::initVtable(void* lib) {
    void** vt = &((void**) hybris_dlsym(lib, "_ZTV21AppPlatform_android23"))[2];
    void** vta = &((void**) hybris_dlsym(lib, "_ZTV19AppPlatform_android"))[2];

    PatchUtils::VtableReplaceHelper vtr (lib, vt, vta);
    vtr.replace("_ZNK11AppPlatform19supportsFilePickingEv", &ClientAppPlatform::supportsFilePicking);
    vtr.replace("_ZNK11AppPlatform12supportsMSAAEv", &ClientAppPlatform::supportsMSAA);
}