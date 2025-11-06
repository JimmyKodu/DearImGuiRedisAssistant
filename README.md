# Dear ImGui Redis Assistant

A desktop Redis GUI based on [Dear ImGui](https://github.com/ocornut/imgui) for visualization, management, and monitoring.

## Features

- 🔌 **Connection Management**: Connect to Redis servers with authentication support
- 🔑 **Key Browser**: Browse, view, and manage Redis keys with type information
- 📊 **Monitoring Dashboard**: Real-time server statistics including memory, clients, and commands
- 🖥️ **Cross-Platform**: Self-contained executables for Windows and Linux
- 🚀 **No Dependencies**: Statically linked executables require no external libraries

## Screenshots

The application provides a clean, responsive GUI for Redis management:
- Connection dialog for server configuration
- Key browser with type indicators and TTL display
- Monitoring tab with server statistics
- Value viewer with multiline support

## Building from Source

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler
- OpenGL development libraries

### Linux

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install cmake build-essential libgl1-mesa-dev libglu1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

# Clone with submodules
git clone --recursive https://github.com/JimmyKodu/DearImGuiRedisAssistant.git
cd DearImGuiRedisAssistant

# Build
chmod +x build-linux.sh
./build-linux.sh
```

The executable will be in `releases/v1.0.0/linux/DearImGuiRedisAssistant`

### Windows

```batch
# Clone with submodules
git clone --recursive https://github.com/JimmyKodu/DearImGuiRedisAssistant.git
cd DearImGuiRedisAssistant

# Build (requires Visual Studio 2022)
build-windows.bat
```

The executable will be in `releases\v1.0.0\windows\DearImGuiRedisAssistant.exe`

## Using Pre-built Releases

Each commit includes pre-built executables in the `releases` directory:
- `releases/v1.0.0/linux/` - Linux x64 executable
- `releases/v1.0.0/windows/` - Windows x64 executable

These are self-contained and require no additional dependencies.

## Usage

1. Launch the application
2. Use **Connection → Connect** to configure your Redis server
3. Enter host, port, and password (if required)
4. Browse keys in the **Keys Browser** tab
5. Monitor server statistics in the **Monitor** tab

## Technology Stack

- **[Dear ImGui](https://github.com/ocornut/imgui)**: Immediate mode GUI library
- **[GLFW](https://github.com/glfw/glfw)**: OpenGL window and input handling
- **[hiredis](https://github.com/redis/hiredis)**: Redis C client library
- **OpenGL 3.0**: Graphics rendering

## License

This project uses the following open-source libraries:
- Dear ImGui - MIT License
- GLFW - zlib/libpng License
- hiredis - BSD License

## Contributing

Contributions are welcome! Please ensure:
1. Code compiles on both Windows and Linux
2. Self-contained executables are generated
3. Build scripts work correctly

## CI/CD

GitHub Actions automatically builds executables for both platforms on every commit to main/master branch. The workflow:
1. Builds Linux executable on Ubuntu
2. Builds Windows executable on Windows Server
3. Commits the executables to the `releases` directory with version folders

This ensures every commit has verified, working executables for both platforms.