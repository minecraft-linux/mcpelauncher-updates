//
// pch.h
// Header für Standardsystem-Includedateien.
//
// Wird vom Buildsystem zum Generieren des vorkompilierten Headers verwendet. Beachten Sie, dass
// "pch.cpp" nicht erforderlich ist und "pch.h" automatisch in alle CPP-Dateien,
// die zum Projekt gehören, aufgenommen wird.
//

#include <jni.h>
#include <errno.h>

#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>

#include <android/log.h>
#include "android_native_app_glue.h"
