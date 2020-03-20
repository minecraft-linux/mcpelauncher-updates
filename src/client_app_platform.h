#pragma once

#include <mcpelauncher/app_platform.h>

class Vec2;
class MinecraftGameWrapper;

class ClientAppPlatform : public LauncherAppPlatform {

private:
    static const char* TAG;
public:
    static void initVtable(void* lib);

    bool supportsFilePicking() { return true; }

    bool supportsMSAA() const { return true; }

};