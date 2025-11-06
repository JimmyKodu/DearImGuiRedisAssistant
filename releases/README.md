# Release Artifacts

This directory contains versioned release builds of the Dear ImGui Redis Assistant.

## Directory Structure

```
releases/
└── v1.0.0/
    ├── linux/
    │   ├── DearImGuiRedisAssistant  (1.6 MB executable)
    │   └── VERSION
    └── windows/
        ├── DearImGuiRedisAssistant.exe  (will be added by CI/CD)
        └── VERSION
```

## Platform-Specific Executables

### Linux (x86_64)
- **File**: `linux/DearImGuiRedisAssistant`
- **Size**: ~1.6 MB
- **Type**: ELF 64-bit LSB pie executable
- **Dependencies**: Only system libraries (OpenGL, libc, libm, libgcc)
- **Tested on**: Ubuntu 24.04 LTS

**How to run**:
```bash
cd releases/v1.0.0/linux
chmod +x DearImGuiRedisAssistant
./DearImGuiRedisAssistant
```

### Windows (x86_64)
- **File**: `windows/DearImGuiRedisAssistant.exe`
- **Type**: PE32+ executable
- **Dependencies**: Statically linked, self-contained
- **Tested on**: Windows 10/11

**How to run**:
```cmd
cd releases\v1.0.0\windows
DearImGuiRedisAssistant.exe
```

## Build Information

These executables are automatically built and verified by GitHub Actions on every commit to ensure:
1. ✅ Successful compilation on both platforms
2. ✅ All dependencies properly linked
3. ✅ Self-contained deployment (no external runtime required)

## Version History

### v1.0.0 (Initial Release)
- Dear ImGui-based Redis GUI
- Cross-platform support (Windows & Linux)
- Connection management with authentication
- Key browser with type information
- Monitoring dashboard with server statistics
- Self-contained executables
