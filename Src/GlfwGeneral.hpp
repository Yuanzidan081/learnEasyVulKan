#pragma once
// 用来写一些GLFW的通用内容及相关的全局变量
#include "VKBase.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib") //链接编译所需的静态库

//窗口的指针，全局变量自动初始化为NULL
GLFWwindow* pWindow;
//显示器信息的指针
GLFWmonitor* pMonitor;
//窗口标题
const char* windowTitle = "learnEasyVK";

/*
 * InitializeWindow: 初始化窗口，初始化成功时返回true，否则返回false。
 * size: 窗口大小
 * fullScreen: 指定是否以全屏初始化窗口
 * isResizable: 指定窗口是否可拉伸，游戏窗口通常是不可任意拉伸的
 * limitFrameRate: 指定是否将帧数限制到不超过屏幕刷新率，在本节先不实现这个参数的作用
 */
bool InitializeWindow(VkExtent2D size, bool fullScreen = false, bool isResizable = true, bool limitFrameRate = true) {
    if (!glfwInit()) {
        std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n");
        return false;
    }
    // 向GLFW说明不需要OpenGL的API
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // 向GLFW指定窗口可否拉伸
    glfwWindowHint(GLFW_RESIZABLE, isResizable);

    // 获取当前显示器信息的指针
    pMonitor = glfwGetPrimaryMonitor();

    // 通常全屏时的图像区域应当跟屏幕分辨率一致
    // 取得显示器当前的视频模式：视频模式可能因为用户的操作而在程序运行过程中发生变更，因此总是在需要时获取，而不将其存储到全局变量。
    // 你不需要手动 delete pMode，GLFW的注释及官方文档中写到：
    // The returned array is allocated and freed by GLFW. You should not free it yourself. It is valid until the specified monitor is disconnected or the library is terminated.
    const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);

    // 创建窗口，根据fullScreen的值决定是否以全屏模式初始化窗口
    // 第四个参数是nullptr为窗口模式
    // 第五个参数可以传入其他窗口的指针和其他窗口共享内容，这里没有，填nullptr
    pWindow = fullScreen ?
        glfwCreateWindow(pMode->width, pMode->height, windowTitle, pMonitor, nullptr) :
        glfwCreateWindow(size.width, size.height, windowTitle, nullptr, nullptr);

    if (!pWindow) {
        std::cout << std::format("[ InitializeWindow ]\nFailed to create a glfw window!\n");
        glfwTerminate();
        return false;
    }
    return true;
}

/*
 * TerminateWindow: 终止窗口时，清理GLFW
 */
void TerminateWindow() {
/* 待后续填充 */
    glfwTerminate();
}


void TitleFps() {
    static double time0 = glfwGetTime();
    static double time1;
    static double dt;
    static int dframe = -1;
    static std::stringstream info;
    time1 = glfwGetTime();
    dframe++;
    if ((dt = time1 - time0) >= 1) {
        info.precision(1);
        info << windowTitle << "    " << std::fixed << dframe / dt << " FPS";
        glfwSetWindowTitle(pWindow, info.str().c_str());
        info.str(""); // 别忘了在设置完窗口标题后清空所用的stringstream
        time0 = time1;
        dframe = 0;
    }
}
