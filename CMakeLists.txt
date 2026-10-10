cmake_minimum_required(VERSION 3.20)
project(learnEasyVK LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 配置属性 → 常规 → 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    "${PROJECT_SOURCE_DIR}/bin/${PROJECT_NAME}_${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>"
)


file(GLOB SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/Src/*.cpp"
    "${PROJECT_SOURCE_DIR}/Src/*.h"
    "${PROJECT_SOURCE_DIR}/Src/*.hpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

# C/C++ → 常规 → 附加包含目录（所有配置、所有平台）
target_include_directories(${PROJECT_NAME} PRIVATE
    "${PROJECT_SOURCE_DIR}/ThirdPart/stb_image"
    "${PROJECT_SOURCE_DIR}/ThirdPart/Vulkan/include"
    "${PROJECT_SOURCE_DIR}/ThirdPart/glm"
    "${PROJECT_SOURCE_DIR}/ThirdPart/GLFW/include"
    "${PROJECT_SOURCE_DIR}/Src"
)

# C/C++ → 语言 → C++ 语言标准
# MSVC 对应“预览 - 最新 C++ 工作草案中的功能 (/std:c++latest)”
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        "/utf-8"
        /std:c++latest
    )
else()
    # 非 MSVC 时可改为项目所需的稳定标准，例如 C++23
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
endif()

# 当前为x64平台

target_link_directories(${PROJECT_NAME} PRIVATE
        "${PROJECT_SOURCE_DIR}/ThirdPart/Vulkan/lib_win64"
        "${PROJECT_SOURCE_DIR}/ThirdPart/GLFW/lib_win64"
    )


# 链接库名
target_link_libraries(${PROJECT_NAME} PRIVATE
    vulkan-1
    glfw3
)
