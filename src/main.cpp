#include "types.hpp"
#include <cstring>
#include <dlfcn.h>
#include <iostream>
#include <link.h>
#include <sys/mman.h>
#include <vector>
#include <jni.h>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <jni.h>

#ifdef __aarch64__
#define ARCH_FOLDER "arm64-v8a"
#else
#define ARCH_FOLDER "x86_64"
#endif

using dlsym_fn = void *(*)(void *handle, const char *name);

struct cryptstring {
  const char* _ptr;
  cryptstring(const char* ptr) {
    _ptr = ptr;
  }

  const char* c_str() {
    return _ptr;
  }

  operator const char*() {
    return _ptr;
  }
  operator std::filesystem::path() {
    return _ptr;
  }
  operator std::string() {
    return _ptr;
  }
};

#ifndef ENABLE_VALIDATION
JNIEnv *env;
#endif

#ifdef ENABLE_VALIDATION
#include "validation.hpp"
#endif

bool files_identical(const std::filesystem::path& a, const std::filesystem::path& b) {
    std::cout << "start files_identical " << a << " " << b << std::endl;
    std::error_code ec;
    auto sa = std::filesystem::file_size(a, ec);
    if (ec) return false;
    auto sb = std::filesystem::file_size(b, ec);
    if (ec) return false;
    if (sa != sb) return false;

    std::ifstream fa(a, std::ios::binary);
    std::ifstream fb(b, std::ios::binary);
    if (!fa || !fb) return false;

    static const std::size_t BUF_SIZE = 1 << 16;
    std::vector<char> ba(BUF_SIZE), bb(BUF_SIZE);

    while (fa && fb) {
        fa.read(ba.data(), BUF_SIZE);
        fb.read(bb.data(), BUF_SIZE);
        std::streamsize ra = fa.gcount();
        std::streamsize rb = fb.gcount();
        if (ra != rb) return false;
        if (ra == 0) break;
        if (!std::equal(ba.begin(), ba.begin() + ra, bb.begin())) return false;
    }
    std::cout << "end" << a << " " << b << std::endl;
    return true;
}

std::filesystem::path make_backup_path(const std::filesystem::path& original) {
    std::filesystem::path base = original;
    std::filesystem::path bck = base;
    bck += ".bck";
    if (!std::filesystem::exists(bck)) return bck;

    // try .bck1, .bck2, ...
    for (int i = 1; i < 10000; ++i) {
        std::filesystem::path candidate = base;
        candidate += ".bck" + std::to_string(i);
        if (!std::filesystem::exists(candidate)) return candidate;
    }
    // fallback: append timestamp if all else fails
    return base.string() + ".bck.fallback";
}

bool copy_file_portable(const std::filesystem::path& src,
                        const std::filesystem::path& dst)
{
    std::ifstream in(src, std::ios::binary);
    std::ofstream out(dst, std::ios::binary);

    if (!in || !out)
        return false;

    static constexpr std::size_t BUF_SIZE = 1 << 16;
    std::vector<char> buffer(BUF_SIZE);

    while (in) {
        in.read(buffer.data(), buffer.size());
        std::streamsize n = in.gcount();
        if (n > 0)
            out.write(buffer.data(), n);
    }

    return out.good();
}

void copy_with_backup(const std::filesystem::path& src_root, const std::filesystem::path& dst_root) {
    std::cout << "Start copy_with_backup" << src_root << " to " << dst_root << std::endl;
    std::error_code ec;
    for (auto it = std::filesystem::recursive_directory_iterator(src_root, ec); it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
        if (ec) {
            std::cerr << "Traversal error: " << ec.message() << "\n";
            continue;
        }
        const std::filesystem::path src_path = it->path();
        if(src_path.string().find(".so") == std::string::npos) {
          continue;
        }

        std::cout << "f" << src_path << std::endl;
        std::filesystem::path rel = std::filesystem::relative(src_path, src_root, ec);
        if (ec) {
            std::cerr << "Relative path error: " << ec.message() << "\n";
            continue;
        }
        std::filesystem::path dst_path = dst_root / rel;
        std::cout << "d" << dst_path << std::endl;

        if (std::filesystem::is_directory(src_path)) {
            if (!std::filesystem::exists(dst_path)) {
                std::filesystem::create_directories(dst_path);
                std::cout << "Created directory: " << dst_path << "\n";
            }
        } else if (std::filesystem::is_regular_file(src_path)) {
            if (!std::filesystem::exists(dst_path)) {
                std::filesystem::create_directories(dst_path.parent_path());
                std::cout << "Copy new file: " << dst_path << "\n" << std::endl;
                copy_file_portable(src_path, dst_path);
                std::cout << "Copied new file: " << dst_path << "\n";
            } else {
                std::cout << "check identical: " << dst_path << "\n" << std::endl;
                if (files_identical(src_path, dst_path)) {
                    std::cout << "Unchanged, skipped: " << dst_path << "\n";
                } else {
                    std::cout << "Update file: " << dst_path << "\n" << std::endl;
                    std::filesystem::path backup = make_backup_path(dst_path);
                    std::filesystem::create_directories(backup.parent_path());
                    std::filesystem::rename(dst_path, backup);
                    copy_file_portable(src_path, dst_path);
                    std::cout << "Backed up: " << backup << "  -> replaced with: " << dst_path << "\n";
                }
            }
        } else {
            std::cout << "Skipping non-regular file: " << src_path << "\n";
        }
    }
}

static size_t ___strlcpy_chk(char* dst, const char* src, size_t dst_len, size_t src_len) {
    return strlcpy(dst, src, dst_len);
}

static bool symbolsAdded = false;

static bool androidAndChromeOSIntelArm64(int code, int low, int high) {
  int prefixes[] = { 970000000, 980000000, 1970000000, 1980000000 };
  for(auto prefix : prefixes) {
    if(code >= (prefix + low) && code < (prefix + high)) {
      return true;
    }
  }
  return false;
}

static void add_symbols() {
  if(symbolsAdded) {
    return;
  }
  symbolsAdded = true;
  static auto dlsym_ptr = (dlsym_fn)dlsym(RTLD_NEXT, cryptstring("dlsym"));

  auto dlopen_ptr =
    (void *(*)(const char *, int))dlsym_ptr(RTLD_NEXT, cryptstring("dlopen"));
  auto dlclose_ptr =
    (void(*)(void *))dlsym_ptr(RTLD_NEXT, cryptstring("dlclose"));

  static auto mcpelauncher_mod = dlopen_ptr(cryptstring("libmcpelauncher_mod.so"), RTLD_NOLOAD);
  if(mcpelauncher_mod == nullptr) {
    IF_DEBUG(std::cout << cryptstring("mcpelauncher_mod not found") << std::endl);
    return;
  }

  auto mcpelauncher_relocate =
    (void (*)(void* handle, const char* name, void* hook))dlsym_ptr(mcpelauncher_mod, cryptstring("mcpelauncher_relocate"));
  
  if(!mcpelauncher_relocate) {
    IF_DEBUG(std::cout << cryptstring("mcpelauncher_relocate not found") << std::endl);
    return;
  }
  
  auto libc = dlopen_ptr(cryptstring("libc.so"), RTLD_NOLOAD);
  if(libc == nullptr) {
    IF_DEBUG(std::cout << cryptstring("libc.so not found") << std::endl);
    return;
  }

  IF_DEBUG(std::cout << cryptstring("relocate") << std::endl);

  mcpelauncher_relocate(libc, cryptstring("__strlcpy_chk"), (void*)&___strlcpy_chk);

  auto mcpelauncher_package_version_code =
  (int*)dlsym_ptr(mcpelauncher_mod, cryptstring("mcpelauncher_package_version_code"));
  
  if(!mcpelauncher_package_version_code) {
    IF_DEBUG(std::cout << cryptstring("mcpelauncher_package_version_code not found") << std::endl);
    return;
  }

  // 1.26.45.x uses version code 26045xx. Keep the DRM compatibility module
  // enabled through that maintained release line as well as the prior 1.26.x
  // releases; the replacement PlayFab library is ABI-compatible across it.
  if(androidAndChromeOSIntelArm64(*mcpelauncher_package_version_code, 2113000, 2604999)) {
    Dl_info info;
    dladdr((void*)&___strlcpy_chk, &info);
    dlopen_ptr((std::filesystem::path(info.dli_fname).parent_path() / cryptstring("patches") / cryptstring("libPlayFabMultiplayer.so")).c_str(), RTLD_NOW);
    IF_DEBUG(std::cout << std::filesystem::path(info.dli_fname).parent_path() / cryptstring("patches") / cryptstring("libPlayFabMultiplayer.so") << std::endl);

    if(androidAndChromeOSIntelArm64(*mcpelauncher_package_version_code, 2601000, 2604999)) { 
      std::filesystem::path outPath{std::filesystem::path(info.dli_fname).parent_path() / cryptstring("patches") / "v1.26.0.2/" ARCH_FOLDER};
      std::filesystem::create_directories(outPath);

      auto libcxx = dlopen_ptr("libc++_shared.so", 0);
      
      Dl_info game;
      dladdr(dlsym(libcxx, "_ZTIPs"), &game);
      std::cout << std::filesystem::path(game.dli_fname).parent_path() << std::endl;
      std::cout << "Start copy_with_backup" << std::endl;
      copy_with_backup(outPath, std::filesystem::path(game.dli_fname).parent_path());
      std::cout << "End copy_with_backup" << std::endl;
    }

    auto mcpelauncher_unload_library =
      (void (*)(void*))dlsym_ptr(mcpelauncher_mod, cryptstring("mcpelauncher_unload_library"));

    if(!mcpelauncher_unload_library) {
      IF_DEBUG(std::cout << cryptstring("mcpelauncher_unload_library not found") << std::endl);
      return;
    }
    IF_DEBUG(std::cout << cryptstring("libfmod") << std::endl);

    auto fmod = dlopen_ptr(cryptstring("libfmod.so").c_str(), 0);
    IF_DEBUG(std::cout << cryptstring("libfmod") << (intptr_t)fmod << std::endl);

    dlclose_ptr(fmod);
    mcpelauncher_unload_library(fmod);
  }
  struct __emutlsControl {
      size_t size;
      size_t align;
      uintptr_t index;
      void* value;
  };
  auto libcxx = dlopen_ptr("libc++_shared.so", 0);
  static auto __emutls_get_address = (void*(*)(__emutlsControl* ctrl))dlsym_ptr(libcxx, cryptstring("__emutls_get_address"));
  if(__emutls_get_address) {
    mcpelauncher_relocate(libcxx, cryptstring("__emutls_get_address"), (void*)+[](__emutlsControl* ctrl) {
        double scratch;
        if(modf(log2(ctrl->align), &scratch) != 0.0) {
            std::cout << "Corrupted __emutls_get_address alignment: " << ctrl->align << ", size=" << ctrl->size << ", index=" << ctrl->index << "value:" << ctrl->value << "\n";
            ctrl->size = 64;
            ctrl->align = 8;
            ctrl->index = 0;
            ctrl->value = nullptr;
        }
        return __emutls_get_address(ctrl);
    });
  }
}

extern "C" void mod_init();

__attribute__((visibility("default"))) jint JNI_OnLoad(JavaVM *vm, void *reserved) {
#ifdef MCPELAUNCHER_UPDATES_STANDALONE
  // On non-Android hosts the launcher invokes mod_preinit directly. Re-entering
  // PairIP through the Android JNI bridge is unnecessary and can trip its
  // platform-integrity signal path before the compatibility hooks are applied.
  return JNI_VERSION_1_6;
#else
  JNIEnv* env = nullptr;
  vm->GetEnv((void**)&env, JNI_VERSION_1_6);
#ifdef ENABLE_VALIDATION
  ::env = env;
  validate(+[]() {
    add_symbols();
  });
#endif
  static auto dlsym_ptr = (dlsym_fn)dlsym(RTLD_NEXT, cryptstring("dlsym"));

  auto dlopen_ptr =
    (void *(*)(const char *, int))dlsym_ptr(RTLD_NEXT, cryptstring("dlopen"));
  auto mcpelauncher_mod = dlopen_ptr(cryptstring("libmcpelauncher_mod.so"), RTLD_NOLOAD);
  if(mcpelauncher_mod == nullptr) {
    IF_DEBUG(std::cout << cryptstring("mcpelauncher_mod not found") << std::endl);
    return 0;
  }

  auto jnivm_register_method =
    (bool (*)(JNIEnv* env, jclass cl, int type, const char* name, const char* signature, jvalue (*cbk)(JNIEnv* env, jobject thiz, jvalue* values)))dlsym_ptr(mcpelauncher_mod, cryptstring("jnivm_register_method"));

  jclass vmRunner = env->FindClass("com/pairip/VMRunner");

  jnivm_register_method(env, vmRunner, 3, cryptstring("invoke").c_str(), cryptstring("(Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;").c_str(),
    +[](JNIEnv* env, jobject thiz, jvalue* values) -> jvalue {
#ifdef ENABLE_VALIDATION
      ::env = env;
#endif
      mod_init();
      jvalue ret{};
      return ret;
    }
  );

  auto pairipcore = dlopen_ptr(cryptstring("libpairipcore.so"), 0);
  if (pairipcore == nullptr) {
    IF_DEBUG(std::cerr << "Error opening pairipcore: " << dlerror()
                       << std::endl);
    return 0;
  }
  auto JNI_OnLoad_ptr =
      (jint (*)(JavaVM *vm, void *reserved))dlsym_ptr(pairipcore, cryptstring("JNI_OnLoad"));
  if (JNI_OnLoad_ptr == nullptr) {
    IF_DEBUG(std::cerr << "Error finding JNI_OnLoad in pairipcore: " << dlerror()
                       << std::endl);
  }
  JNI_OnLoad_ptr(vm, reserved);
  return 0;
#endif
}

__attribute__((visibility("default"))) extern "C" void mod_preinit() {
  add_symbols();
}

__attribute__((visibility("default"))) extern "C" void mod_init() {
#ifdef ENABLE_VALIDATION
  validate(+[]() {});
#else
  add_symbols();
#endif
}
