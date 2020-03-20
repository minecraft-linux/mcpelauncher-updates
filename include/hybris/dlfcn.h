#include <dlfcn.h>

#define hybris_dlsym dlsym
#define hybris_dlopen dlopen
#define hybris_dladdr dladdr
#define hybris_dlclose dlclose
#define hybris_dlerror dlerror