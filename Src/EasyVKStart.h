#pragma once
//可能会用上的C++标准库
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>
#include <span>
#include <memory>
#include <functional>
#include <concepts>
#include <format>
#include <chrono>
#include <numeric>
#include <numbers>

//GLM
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//如果你惯用左手坐标系，在此定义GLM_FORCE_LEFT_HANDED
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

//stb_image.h
#include <stb_image.h>

//Vulkan
#ifdef _WIN32                             //考虑平台是Windows的情况（请自行解决其他平台上的差异）
#define VK_USE_PLATFORM_WIN32_KHR         //在包含vulkan.h前定义该宏，会一并包含vulkan_win32.h和windows.h
#define NOMINMAX                          //定义该宏可避免windows.h中的min和max两个宏与标准库中的函数名冲突
#pragma comment(lib, "vulkan-1.lib")      //链接编译所需的静态存根库
#endif
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h> //用于将枚举项转为对应的字符串，方便输出错误信息等

template<typename T>
class arrayRef {
    T* const pArray = nullptr;
    size_t count = 0;
public:
    arrayRef() = default;
    arrayRef(T& data) :pArray(&data), count(1) {}
    template<size_t elementCount>
    arrayRef(T(&data)[elementCount]) : pArray(data), count(elementCount) {}
    arrayRef(T* pData, size_t elementCount) :pArray(pData), count(elementCount) {}
    arrayRef(const arrayRef<std::remove_const_t<T>>& other) :pArray(other.Pointer()), count(other.Count()) {}
    //Getter
    T* Pointer() const { return pArray; }
    size_t Count() const { return count; }
    //Const Function
    T& operator[](size_t index) const { return pArray[index]; }
    T* begin() const { return pArray; }
    T* end() const { return pArray + count; }
    //Non-const Function
    arrayRef& operator=(const arrayRef&) = delete;
};

/*
 * 以上代码中arrayRef不支持直接从底层内存连续的标准库容器构造，而std::vector又用得尤其多。
 * 直接从符合要求的标准库容器构造的修改
 template<typename R>
 arrayRef(R&& range) requires requires(R r) {
     requires std::ranges::contiguous_range<R>;
     requires std::ranges::sized_range<R>;
     requires std::ranges::borrowed_range<R>;
     requires std::convertible_to<decltype(std::ranges::data(r)), T*>;
     requires sizeof(std::iter_value_t<R>) == sizeof(T);
 } :pArray(std::ranges::data(range)), count(std::ranges::size(range)) {}
 */
// ExecuteOnce宏：用来把函数分割成能被多次执行，以及只执行一次的两个部分
// 这个宏能如同函数般被使用，在其之上的部分可以被多次执行，在其之下的部分只会被执行一次，
// 如果在其之下的部分已经执行过了，那么就直接返回括号中的参数。
#define ExecuteOnce(...) { static bool executed = false; if (executed) return __VA_ARGS__; executed = true; }
