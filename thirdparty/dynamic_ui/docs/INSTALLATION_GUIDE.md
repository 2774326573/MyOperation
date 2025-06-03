# 📦 安装指南 / Installation Guide

<div align="center">

![Installation](https://img.shields.io/badge/Guide-Installation-blue.svg)
![Platforms](https://img.shields.io/badge/Platforms-Windows%20|%20macOS%20|%20Linux-success.svg)

**详细的跨平台安装指导和故障排除**  
*Detailed cross-platform installation guide and troubleshooting*

</div>

---

## 📋 目录 / Table of Contents

- [🔧 系统要求](#-系统要求--system-requirements)
- [🪟 Windows安装](#-windows安装--windows-installation)
- [🍎 macOS安装](#-macos安装--macos-installation)
- [🐧 Linux安装](#-linux安装--linux-installation)
- [📱 移动平台](#-移动平台--mobile-platforms)
- [🛠️ 开发环境配置](#️-开发环境配置--development-setup)
- [🚨 故障排除](#-故障排除--troubleshooting)
- [✅ 验证安装](#-验证安装--verify-installation)

---

## 🔧 系统要求 / System Requirements

### 📊 最低配置 / Minimum Requirements

| 组件 | 最低版本 | 推荐版本 | 说明 |
|------|----------|----------|------|
| **Qt** | 6.0 | 6.5+ | 核心依赖 |
| **CMake** | 3.16 | 3.25+ | 构建系统 |
| **C++编译器** | C++17 | C++20 | 语言标准 |
| **内存** | 4GB | 8GB+ | 编译需要 |
| **磁盘空间** | 2GB | 5GB+ | 包含Qt和工具链 |

### 🎯 支持的编译器 / Supported Compilers

#### Windows
- ✅ **Visual Studio 2019** (v16.0+)
- ✅ **Visual Studio 2022** (推荐)
- ✅ **Clang** (13.0+)
- ✅ **MinGW-w64** (8.0+)

#### macOS
- ✅ **Xcode** (12.0+)
- ✅ **Clang** (13.0+)
- ✅ **GCC** (9.0+)

#### Linux
- ✅ **GCC** (9.0+)
- ✅ **Clang** (13.0+)

---

## 🪟 Windows安装 / Windows Installation

### 📥 方法1: 使用Visual Studio (推荐新手)

#### 1️⃣ 安装Visual Studio

```powershell
# 下载Visual Studio Community 2022（免费）
# https://visualstudio.microsoft.com/downloads/

# 必须包含的组件：
# - C++ CMake tools for Visual Studio
# - C++ core features
# - MSVC v143 compiler toolset
```

#### 2️⃣ 安装Qt

```powershell
# 方法A: 使用Qt在线安装器（推荐）
# 1. 下载 https://www.qt.io/download-qt-installer
# 2. 安装时选择：
#    - Qt 6.5.3 或更新版本
#    - MSVC 2022 64-bit
#    - Qt Creator（可选，推荐）
#    - CMake（如果VS没有）

# 方法B: 使用vcpkg包管理器
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install qt6[core,widgets] --triplet=x64-windows
```

#### 3️⃣ 配置环境变量

```powershell
# 设置Qt安装路径（根据实际安装路径调整）
setx Qt6_DIR "C:\Qt\6.5.3\msvc2022_64"
setx PATH "%PATH%;C:\Qt\6.5.3\msvc2022_64\bin"

# 验证安装
qmake --version
cmake --version
```

#### 4️⃣ 克隆和构建项目

```powershell
# 使用Git Bash或PowerShell
git clone https://github.com/your-repo/dynamic-ui.git
cd dynamic-ui

# 创建构建目录
mkdir build
cd build

# 配置项目（使用CMake）
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DQt6_DIR="C:/Qt/6.5.3/msvc2022_64/lib/cmake/Qt6"

# 构建项目
cmake --build . --config Release
```

### 🛠️ 方法2: 使用命令行工具（适合有经验的开发者）

#### 1️⃣ 安装构建工具

```powershell
# 使用Chocolatey（推荐）
# 首先安装Chocolatey: https://chocolatey.org/install

choco install cmake
choco install git
choco install visualstudio2022buildtools
choco install visualstudio2022-workload-vctools
```

#### 2️⃣ 使用vcpkg管理依赖

```powershell
# 安装vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat

# 安装Qt6
.\vcpkg install qt6[core,widgets] --triplet=x64-windows

# 集成到全局
.\vcpkg integrate install
```

#### 3️⃣ 构建项目

```powershell
git clone https://github.com/your-repo/dynamic-ui.git
cd dynamic-ui
mkdir build && cd build

# 使用vcpkg工具链
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=x64-windows

cmake --build . --config Release
```

---

## 🍎 macOS安装 / macOS Installation

### 📥 方法1: 使用Homebrew (推荐)

#### 1️⃣ 安装Xcode和Homebrew

```bash
# 安装Xcode命令行工具
xcode-select --install

# 安装Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

#### 2️⃣ 安装依赖

```bash
# 安装Qt6和CMake
brew install qt6 cmake git

# 将Qt6添加到PATH
echo 'export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"' >> ~/.zshrc
echo 'export Qt6_DIR="/opt/homebrew/opt/qt@6/lib/cmake/Qt6"' >> ~/.zshrc
source ~/.zshrc

# 验证安装
qmake --version
cmake --version
```

#### 3️⃣ 构建项目

```bash
# 克隆项目
git clone https://github.com/your-repo/dynamic-ui.git
cd dynamic-ui

# 构建
mkdir build && cd build
cmake .. -DQt6_DIR="/opt/homebrew/opt/qt@6/lib/cmake/Qt6"
make -j$(sysctl -n hw.ncpu)
```

### 🛠️ 方法2: 使用Qt官方安装器

#### 1️⃣ 下载和安装Qt

```bash
# 1. 从 https://www.qt.io/download-qt-installer 下载安装器
# 2. 安装时选择：
#    - Qt 6.5.3 或更新版本
#    - macOS
#    - Qt Creator（可选）

# 3. 设置环境变量
export Qt6_DIR="/Users/$USER/Qt/6.5.3/macos/lib/cmake/Qt6"
export PATH="/Users/$USER/Qt/6.5.3/macos/bin:$PATH"
```

#### 2️⃣ 安装CMake

```bash
# 使用Homebrew
brew install cmake

# 或者下载CMake.app并添加到PATH
# https://cmake.org/download/
```

---

## 🐧 Linux安装 / Linux Installation

### 📥 Ubuntu/Debian系统

#### 1️⃣ 安装依赖包

```bash
# 更新包列表
sudo apt update

# 安装构建工具
sudo apt install -y \
    build-essential \
    cmake \
    git \
    pkg-config

# 安装Qt6开发包
sudo apt install -y \
    qt6-base-dev \
    qt6-base-dev-tools \
    libqt6core6 \
    libqt6widgets6

# 如果需要额外的Qt6模块
sudo apt install -y \
    qt6-declarative-dev \
    qt6-multimedia-dev \
    qt6-svg-dev
```

#### 2️⃣ 验证安装

```bash
# 检查Qt版本
qmake6 --version
# 或者
/usr/lib/qt6/bin/qmake --version

# 检查CMake版本
cmake --version
```

#### 3️⃣ 构建项目

```bash
# 克隆项目
git clone https://github.com/your-repo/dynamic-ui.git
cd dynamic-ui

# 构建
mkdir build && cd build

# 配置（可能需要指定Qt6路径）
cmake .. -DQt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6
# 或者
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib/qt6

# 编译
make -j$(nproc)
```

### 📥 CentOS/RHEL/Fedora系统

#### 1️⃣ Fedora安装

```bash
# 安装开发工具
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git

# 安装Qt6
sudo dnf install qt6-qtbase-devel qt6-qttools-devel
```

#### 2️⃣ CentOS 8+安装

```bash
# 启用EPEL和PowerTools
sudo dnf install epel-release
sudo dnf config-manager --set-enabled powertools

# 安装构建工具
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake3 git

# 可能需要从源码编译Qt6（CentOS官方仓库可能没有Qt6）
```

### 📥 Arch Linux安装

```bash
# 安装依赖
sudo pacman -S base-devel cmake git qt6-base qt6-tools

# 构建项目
git clone https://github.com/your-repo/dynamic-ui.git
cd dynamic-ui
mkdir build && cd build
cmake ..
make -j$(nproc)
```

---

## 📱 移动平台 / Mobile Platforms

### 📱 Android

#### 1️⃣ 安装Android开发环境

```bash
# 1. 安装Android Studio
# 2. 安装Android SDK和NDK
# 3. 使用Qt Creator配置Android套件

# 在Qt Creator中配置：
# - Android SDK路径: /path/to/Android/Sdk
# - Android NDK路径: /path/to/Android/Sdk/ndk/version
# - Java JDK路径: /path/to/jdk
```

#### 2️⃣ 构建Android应用

```bash
# 使用Qt Creator的Android套件构建
# 或者使用命令行：

mkdir build-android && cd build-android

# 配置Android构建
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=/path/to/android-ndk/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-23 \
    -DQt6_DIR=/path/to/qt6/android_arm64_v8a/lib/cmake/Qt6

make
```

### 🍎 iOS

#### 1️⃣ 准备iOS开发环境

```bash
# 1. 安装Xcode（从App Store）
# 2. 安装Qt for iOS
# 3. 在Qt Creator中配置iOS套件
```

#### 2️⃣ 构建iOS应用

```bash
mkdir build-ios && cd build-ios

# 配置iOS构建
cmake .. \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DQt6_DIR=/path/to/qt6/ios/lib/cmake/Qt6

make
```

---

## 🛠️ 开发环境配置 / Development Setup

### 🎯 Qt Creator配置

#### 1️⃣ 安装Qt Creator

```bash
# 通常随Qt安装器一起安装
# 或者单独下载：https://www.qt.io/product/development-tools
```

#### 2️⃣ 配置套件(Kits)

```
1. 打开Qt Creator
2. 转到 Tools -> Options -> Kits
3. 添加新套件：
   - 名称: Dynamic UI Development
   - Device type: Desktop
   - Compiler: 选择对应的C++编译器
   - Qt version: 选择安装的Qt6版本
   - CMake Tool: 选择CMake可执行文件
```

### 🔧 Visual Studio Code配置

#### 1️⃣ 安装扩展

```json
// .vscode/extensions.json
{
    "recommendations": [
        "ms-vscode.cpptools",
        "ms-vscode.cmake-tools",
        "twxs.cmake",
        "formulahendry.code-runner"
    ]
}
```

#### 2️⃣ 配置CMake

```json
// .vscode/settings.json
{
    "cmake.configureArgs": [
        "-DQt6_DIR=/path/to/qt6/lib/cmake/Qt6"
    ],
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

---

## 🚨 故障排除 / Troubleshooting

### ❌ 常见编译错误

#### 1️⃣ "Qt6 not found"

**错误信息:**
```
CMake Error: Could not find a package configuration file provided by "Qt6"
```

**解决方案:**
```bash
# 方法1: 指定Qt6路径
cmake .. -DQt6_DIR=/path/to/qt6/lib/cmake/Qt6

# 方法2: 设置CMAKE_PREFIX_PATH
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6

# 方法3: 设置环境变量
export Qt6_DIR=/path/to/qt6/lib/cmake/Qt6
```

#### 2️⃣ "C++ compiler not found"

**Windows:**
```powershell
# 安装Visual Studio Build Tools
choco install visualstudio2022buildtools
choco install visualstudio2022-workload-vctools
```

**Linux:**
```bash
# Ubuntu/Debian
sudo apt install build-essential

# CentOS/RHEL
sudo dnf groupinstall "Development Tools"
```

**macOS:**
```bash
# 安装Xcode命令行工具
xcode-select --install
```

#### 3️⃣ "CMake version too old"

```bash
# Ubuntu/Debian
sudo apt remove cmake
sudo apt install cmake

# 或者从源码安装最新版本
wget https://github.com/Kitware/CMake/releases/download/v3.27.0/cmake-3.27.0.tar.gz
tar -xzf cmake-3.27.0.tar.gz
cd cmake-3.27.0
./bootstrap && make && sudo make install
```

### 🔧 运行时错误

#### 1️⃣ "Qt libraries not found"

**Linux:**
```bash
# 添加Qt库路径到LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/path/to/qt6/lib:$LD_LIBRARY_PATH

# 或者使用ldconfig
echo "/path/to/qt6/lib" | sudo tee /etc/ld.so.conf.d/qt6.conf
sudo ldconfig
```

**macOS:**
```bash
# 设置DYLD_LIBRARY_PATH
export DYLD_LIBRARY_PATH=/path/to/qt6/lib:$DYLD_LIBRARY_PATH
```

**Windows:**
```powershell
# 添加Qt的bin目录到PATH
set PATH=C:\Qt\6.5.3\msvc2022_64\bin;%PATH%
```

#### 2️⃣ "Plugin not found"

```cpp
// 在main函数中添加插件路径
QApplication app(argc, argv);
app.addLibraryPath("/path/to/qt6/plugins");
```

### 🐛 调试技巧

#### 1️⃣ 启用详细输出

```bash
# CMake详细输出
cmake .. --verbose

# Make详细输出
make VERBOSE=1

# 或者使用ninja
cmake .. -G Ninja
ninja -v
```

#### 2️⃣ 检查依赖

```bash
# Linux: 检查库依赖
ldd ./your_application

# macOS: 检查库依赖
otool -L ./your_application

# Windows: 使用Dependency Walker
# 下载：https://www.dependencywalker.com/
```

---

## ✅ 验证安装 / Verify Installation

### 🧪 快速测试

创建并运行这个简单的测试程序：

```cpp
// test_installation.cpp
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

// 如果Dynamic UI可用，包含头文件
#ifdef DYNAMIC_UI_AVAILABLE
#include "DynamicUIBuilder.h"
#endif

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QWidget window;
    window.setWindowTitle("Dynamic UI 安装测试 / Installation Test");
    window.resize(400, 200);
    
    QVBoxLayout* layout = new QVBoxLayout(&window);
    
    // 基础Qt测试
    QLabel* qtLabel = new QLabel("✅ Qt6 安装成功 / Qt6 installed successfully");
    qtLabel->setStyleSheet("color: green; font-weight: bold;");
    layout->addWidget(qtLabel);
    
    QPushButton* testButton = new QPushButton("🧪 测试按钮 / Test Button");
    QObject::connect(testButton, &QPushButton::clicked, [&]() {
        qDebug() << "Qt6 按钮点击测试成功 / Qt6 button click test successful";
    });
    layout->addWidget(testButton);
    
#ifdef DYNAMIC_UI_AVAILABLE
    // Dynamic UI测试
    QLabel* duiLabel = new QLabel("✅ Dynamic UI 可用 / Dynamic UI available");
    duiLabel->setStyleSheet("color: blue; font-weight: bold;");
    layout->addWidget(duiLabel);
    
    // 这里可以添加Dynamic UI的测试代码
    DynamicUIBuilder builder;
    // ... 测试代码
#else
    QLabel* duiLabel = new QLabel("ℹ️ Dynamic UI 未包含在此测试中 / Dynamic UI not included in this test");
    duiLabel->setStyleSheet("color: orange;");
    layout->addWidget(duiLabel);
#endif
    
    window.show();
    
    qDebug() << "🎉 测试应用启动成功 / Test application started successfully";
    qDebug() << "Qt 版本 / Qt Version:" << QT_VERSION_STR;
    
    return app.exec();
}
```

### 🏗️ 测试CMakeLists.txt

```cmake
# CMakeLists.txt for installation test
cmake_minimum_required(VERSION 3.16)
project(DynamicUIInstallationTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# 创建可执行文件
add_executable(test_installation test_installation.cpp)

# 链接Qt6
target_link_libraries(test_installation Qt6::Core Qt6::Widgets)

# 如果Dynamic UI可用，尝试链接
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../include/DynamicUIBuilder.h")
    target_include_directories(test_installation PRIVATE ../include)
    target_compile_definitions(test_installation PRIVATE DYNAMIC_UI_AVAILABLE)
    # 如果有库文件，在这里链接
endif()

# 输出信息
message(STATUS "Qt6 版本 / Qt6 Version: ${Qt6_VERSION}")
message(STATUS "编译器 / Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "构建类型 / Build Type: ${CMAKE_BUILD_TYPE}")
```

### 🚀 运行测试

```bash
# 创建测试目录
mkdir installation_test && cd installation_test

# 复制测试文件（test_installation.cpp 和 CMakeLists.txt）
# ...

# 构建测试
mkdir build && cd build
cmake ..
make  # 或者在Windows上使用 cmake --build .

# 运行测试
./test_installation  # Linux/macOS
# 或者在Windows上: test_installation.exe
```

### 📊 成功指标

安装成功的标志：
- ✅ CMake能找到Qt6
- ✅ 编译无错误
- ✅ 应用程序能正常启动
- ✅ 窗口正常显示
- ✅ 按钮点击有响应
- ✅ 控制台输出正常

---

## 🔄 升级和维护

### 📈 升级Qt版本

```bash
# 1. 备份当前项目
# 2. 下载新版本Qt
# 3. 更新CMakeLists.txt中的Qt版本要求
# 4. 重新构建项目

# 检查兼容性
qmake --version
cmake --version
```

### 🧹 清理构建

```bash
# 清理构建目录
rm -rf build/
mkdir build && cd build

# 重新配置和构建
cmake ..
make clean && make
```

---

## 📞 获取帮助 / Getting Help

如果安装过程中遇到问题，可以通过以下方式获取帮助：

- 📖 **查看文档**: [docs/](../docs/)
- 🐛 **报告问题**: [GitHub Issues](https://github.com/your-repo/dynamic-ui/issues)
- 💬 **社区讨论**: [GitHub Discussions](https://github.com/your-repo/dynamic-ui/discussions)
- 📧 **邮件支持**: dynamic-ui@example.com

---

<div align="center">

**🎉 安装完成！现在可以开始使用Dynamic UI Builder了！**  
*Installation complete! You can now start using Dynamic UI Builder!*

[📖 用户指南 / User Guide](USER_GUIDE.md) | [⚡ 快速参考 / Quick Reference](QUICK_REFERENCE.md)

</div> 