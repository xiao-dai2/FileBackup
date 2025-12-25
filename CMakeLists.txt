# 最低CMake版本要求（支持C++17和现代配置语法）
cmake_minimum_required(VERSION 3.16)

# 项目信息
project(
    BackupTool 
    VERSION 1.0.0 
    LANGUAGES CXX
    DESCRIPTION "C++文件备份软件（支持GTKmm界面+命令行模式，CMake构建）"
)

# 设置C++标准（强制启用，无需依赖系统默认）
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项配置（警告级别+优化）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # 调试模式：开启完整警告，禁用优化
    add_compile_options(-Wall -Wextra -Wpedantic -O0 -g)
else()
    # 发布模式：开启优化，禁用调试信息
    add_compile_options(-O2 -DNDEBUG)
endif()

# -------------------------- 依赖查找 --------------------------
# 1. 查找GTKmm 3.0（UI依赖）
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

# 2. 查找libtar（打包解包依赖）
find_library(LIBTAR NAMES tar REQUIRED)
if(NOT LIBTAR)
    message(FATAL_ERROR "未找到libtar库，请先安装：sudo apt install libtar-dev")
endif()

# 3. 查找Crypto++（加密解密依赖）
find_library(CRYPTOPP NAMES cryptopp REQUIRED)
if(NOT CRYPTOPP)
    message(FATAL_ERROR "未找到Crypto++库，请先安装：sudo apt install libcrypto++-dev")
endif()

# 4. 查找C++ filesystem（文件操作依赖，C++17自带，部分编译器需显式链接）
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION LESS 9.0)
    find_library(STDCPPFS NAMES stdc++fs REQUIRED)
    set(FILESYSTEM_LIB ${STDCPPFS})
else()
    set(FILESYSTEM_LIB "")
endif()

# -------------------------- 源码配置 --------------------------
# 收集所有源文件（递归查找src目录下的.cpp文件）
file(GLOB_RECURSE PROJECT_SOURCES 
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# 收集所有头文件（用于IDE索引，可选）
file(GLOB_RECURSE PROJECT_HEADERS 
    "${PROJECT_SOURCE_DIR}/src/*.h"
)

# 生成可执行文件
add_executable(${PROJECT_NAME} 
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

# -------------------------- 包含目录与链接库 --------------------------
# 包含头文件目录（GTKmm+项目自身头文件）
target_include_directories(${PROJECT_NAME}
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/src          # 项目根头文件目录
        ${PROJECT_SOURCE_DIR}/src/core     # 核心模块头文件
        ${PROJECT_SOURCE_DIR}/src/ui       # UI模块头文件
        ${GTKMM_INCLUDE_DIRS}              # GTKmm头文件目录
)

# 链接依赖库
target_link_directories(${PROJECT_NAME}
    PRIVATE 
        ${GTKMM_LIBRARY_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE 
        ${GTKMM_LIBRARIES}    # GTKmm库
        ${LIBTAR}             # libtar库
        ${CRYPTOPP}           # Crypto++库
        ${FILESYSTEM_LIB}     # C++ filesystem库
        pthread               # 新增：链接线程库，支持Glib线程池
)


# 修正：GTKmm额外编译选项通过compile_options传递（非宏定义）
target_compile_options(${PROJECT_NAME}
    PRIVATE 
        ${GTKMM_CFLAGS_OTHER}
)

# -------------------------- 安装配置（可选） --------------------------
# 将可执行文件安装到/usr/local/bin目录（sudo make install后可全局调用）
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION /usr/local/bin
)

# 安装文档（可选）
install(FILES README.md
    DESTINATION /usr/local/share/doc/${PROJECT_NAME}
)

# -------------------------- 辅助配置 --------------------------
# 让IDE识别项目结构（可选，针对CLion、VS Code等）
source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Source Files" FILES ${PROJECT_SOURCES})
source_group(TREE ${PROJECT_SOURCE_DIR}/src PREFIX "Header Files" FILES ${PROJECT_HEADERS})

# 打印配置信息（方便调试）
message(STATUS "项目名称：${PROJECT_NAME}")
message(STATUS "项目版本：${PROJECT_VERSION}")
message(STATUS "C++标准：${CMAKE_CXX_STANDARD}")
message(STATUS "编译模式：${CMAKE_BUILD_TYPE}")
message(STATUS "可执行文件路径：${CMAKE_BINARY_DIR}/${PROJECT_NAME}")
message(STATUS "GTKmm版本：${GTKMM_VERSION}")
message(STATUS "GTKmm编译选项：${GTKMM_CFLAGS_OTHER}")