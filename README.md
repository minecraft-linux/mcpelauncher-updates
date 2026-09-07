# mcpelauncher-updates

ARM64 compatibility support currently covers Minecraft 1.21.130 through the
1.26.45 release line. It supplies the maintained PlayFab replacement used by
DRM-protected Minecraft releases.

Use the packaged release archive for normal launcher installations. It includes
the matching PlayFab dependency. To build it yourself, run `make release_zips`
with an Android NDK installed.

## Non-Android launcher hosts

`MCPELAUNCHER_UPDATES_VALIDATE=OFF` disables the optional Google Play
validation/download worker. `MCPELAUNCHER_UPDATES_STANDALONE=ON` also disables
the Android PairIP JNI bridge, for hosts that invoke `mod_preinit` directly.
These options preserve the local compatibility hooks while avoiding Android-only
validation paths. They are intended for port maintainers, not ordinary Android
launcher installs.

## Disclaimer

- Do not use it for piracy!
- Source Code contains less code than the releases
- Untested
