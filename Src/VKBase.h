#pragma once
// 用来写Vulkan相关的基本内容。
#include "EasyVKStart.h"
//定义vulkan命名空间，之后会把Vulkan中一些基本对象的封装写在其中
namespace vulkan {
    /*
     * 单例类graphicsBase
     * 作用：管理Vulkan中最基础的对象和行为
     */
    class graphicsBase {

        /*
        * 单例实现
        */
        private:
            //静态变量
            static graphicsBase singleton;
            //--------------------
            graphicsBase() = default;
            graphicsBase(graphicsBase&&) = delete;
            ~graphicsBase() {
                /*待Ch1-4填充*/
            }
        public:
            //静态函数
            //该函数用于访问单例
            static graphicsBase& Base() {
                return singleton;
            }

        /*
         * 使用最新版Vulkan
         */
        private:
            uint32_t apiVersion = VK_API_VERSION_1_0;
        public:
            //Getter
            uint32_t ApiVersion() const {
                return apiVersion;
            }
            VkResult UseLatestApiVersion() {
                // VkInstance instance：要取得的函数指针不依赖Vulkan实例，可为VK_NULL_HANDLE
                // const char* pName：所要取得的函数的名称
                if (vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"))
                    return vkEnumerateInstanceVersion(&apiVersion); // 取得最新Vulkan版本
                return VK_SUCCESS;
            }
        /*
        * Vulkan初始化流程
        * 一、创建Vulkan实例
        * 二、创建debug messenger（若编译选项为DEBUG）
        * 三、创建window surface
        * 四、选择物理设备并创建逻辑设备，取得队列
        * 五、创建交换链
        */



        /*
        * 一、用于创建vulkan实例
        * 流程
        * 1. 在创建Vulkan实例前，用AddInstanceLayer(...)和AddInstanceExtension(...)向对应的vector中添加指向层和扩展名称的指针。
        * 2. 尝试用CreateInstance(...)创建Vulkan实例。
        * 3. 若vkCreateInstance(...)返回VK_ERROR_LAYER_NOT_PRESENT，从InstanceLayers()复制一份instanceLayers，用CheckInstanceLayers(...)检查可用性，
        * 若不可用的仅为非必要的层，创建一份去除该层后的vector，用InstanceLayers(...)复制给instanceLayers
        */
        private:
            //单例类对象是静态的，未设定初始值亦无构造函数的成员会被零初始化
            VkInstance instance;
            std::vector<const char*> instanceLayers;
            std::vector<const char*> instanceExtensions;

            //该函数用于向instanceLayers或instanceExtensions容器中添加字符串指针，并确保不重复
            static void AddLayerOrExtension(std::vector<const char*>& container, const char* name) {
                for (auto& i : container)
                    if (!strcmp(name, i)) //strcmp(...)在字符串匹配时返回0
                        return;           //如果层/扩展的名称已在容器中，直接返回
                container.push_back(name);
            }
        public:
            //Getter
            VkInstance Instance() const {
                return instance;
            }
            const std::vector<const char*>& InstanceLayers() const {
                return instanceLayers;
            }
            const std::vector<const char*>& InstanceExtensions() const {
                return instanceExtensions;
            }
            //以下函数用于创建Vulkan实例前
            void AddInstanceLayer(const char* layerName) {
                AddLayerOrExtension(instanceLayers, layerName);
            }
            void AddInstanceExtension(const char* extensionName) {
                AddLayerOrExtension(instanceExtensions, extensionName);
            }
            //该函数用于创建Vulkan实例
            VkResult CreateInstance(VkInstanceCreateFlags flags = 0) {
                /*待Ch1-3填充*/
                // 仅在编译选项为DEBUG时，在instanceLayers和instanceExtensions尾部加上所需的名称。
                #ifndef NDEBUG
                    // VK_LAYER_KHRONOS_validation：验证层，找出API错误、同步错误、资源生命周期bug
                    AddInstanceLayer("VK_LAYER_KHRONOS_validation");
                    // VK_EXT_DEBUG_UTILS_EXTENSION_NAME：接收验证层的报错消息、给 Vulkan 对象命名、给命令缓冲打标签，方便调试和 RenderDoc 抓帧看信息
                    AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                #endif
                VkApplicationInfo applicationInfo = {
                    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                    .apiVersion = apiVersion
                };
                // 若你需要获取Vulkan实例创建过程中的debug信息：
                // 让VkInstanceCreateInfo::pNext（或其pNext链中的其它结构体的pNext成员）指向一个VkDebugUtilsMessengerCreateInfoEXT结构体，
                // 如此一来相当于创建一个只在Vulkan实例的创建过程中起作用的、临时的debug messenger。
                VkInstanceCreateInfo instanceCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                    .flags = flags,
                    .pApplicationInfo = &applicationInfo,
                    .enabledLayerCount = uint32_t(instanceLayers.size()),
                    .ppEnabledLayerNames = instanceLayers.data(),
                    .enabledExtensionCount = uint32_t(instanceExtensions.size()),
                    .ppEnabledExtensionNames = instanceExtensions.data()
                };
                if (VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance)) {
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to create a vulkan instance!\nError code: {}\n", int32_t(result));
                    return result;
                }
                //成功创建Vulkan实例后，输出Vulkan版本
                std::cout << std::format(
                    "Vulkan API Version: {}.{}.{}\n",
                    VK_API_VERSION_MAJOR(apiVersion),
                    VK_API_VERSION_MINOR(apiVersion),
                    VK_API_VERSION_PATCH(apiVersion));
                #ifndef NDEBUG
                    //创建完Vulkan实例后紧接着创建debug messenger
                    CreateDebugMessenger();
                #endif
                return VK_SUCCESS;
            }
            //以下函数用于创建Vulkan实例失败后
            // 设计思路：将传入的span中不可用的层和扩展设置为nullptr，然后与原本保存的instanceLayers和instanceExtensions比对，
            // 即可确定哪些层和扩展不可用。返回值则指示在获取可用层或扩展列表时是否发生错误。
            // std::span 是 C++20 引入的非持有、轻量视图（view），用来代表一段连续元素的序列。
            // 它只是记录「起始指针 + 元素个数」，用来包装数组如vector连续缓冲区，避免裸指针 + 长度成对传递。
            VkResult CheckInstanceLayers(std::span<const char*> layersToCheck) {
                uint32_t layerCount;
                std::vector<VkLayerProperties> availableLayers;
                // 第一次vkEnumerateInstanceLayerProperties取得layerCount
                if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr)) {
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of instance layers!\n");
                    return result;
                }
                if (layerCount) {
                    availableLayers.resize(layerCount);
                    // 第二次vkEnumerateInstanceLayerProperties取得layerCount个VkLayerProperties
                    if (VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data())) {
                        std::cout << std::format("[ graphicsBase ] ERROR\nFailed to enumerate instance layer properties!\nError code: {}\n", string_VkResult(result));
                        return result;
                    }
                    // 保留layersToCheck里available的layer
                    for (auto& i : layersToCheck) {
                        bool found = false;
                        for (auto& j : availableLayers)
                            if (!strcmp(i, j.layerName)) {
                                found = true;
                                break;
                            }
                        if (!found)
                            i = nullptr;
                    }
                }
                else
                    for (auto& i : layersToCheck)
                        i = nullptr;
                //一切顺利则返回VK_SUCCESS
                return VK_SUCCESS;
            }
            void InstanceLayers(const std::vector<const char*>& layerNames) {
                instanceLayers = layerNames;
            }

            VkResult CheckInstanceExtensions(std::span<const char*> extensionsToCheck, const char* layerName = nullptr) const {
                uint32_t extensionCount;
                std::vector<VkExtensionProperties> availableExtensions;
                if (VkResult result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr)) {
                    layerName ?
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of instance extensions!\nLayer name: {}\n", layerName) :
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of instance extensions!\n");
                    return result;
                }
                if (extensionCount) {
                    availableExtensions.resize(extensionCount);
                    if (VkResult result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, availableExtensions.data())) {
                        std::cout << std::format("[ graphicsBase ] ERROR\nFailed to enumerate instance extension properties!\nError code: {}\n", string_VkResult(result));
                        return result;
                    }
                    // 保留extensionsToCheck里available的extension
                    for (auto& i : extensionsToCheck) {
                        bool found = false;
                        for (auto& j : availableExtensions)
                            if (!strcmp(i, j.extensionName)) {
                                found = true;
                                break;
                            }
                        if (!found)
                            i = nullptr;
                    }
                }
                else
                    for (auto& i : extensionsToCheck)
                        i = nullptr;
                return VK_SUCCESS;
            }
            void InstanceExtensions(const std::vector<const char*>& extensionNames) {
                instanceExtensions = extensionNames;
            }

        /*
        * 二、创建debug messenger
        * 作用：检查初始化流程中的所有其他步骤
        */
        private:
            VkDebugUtilsMessengerEXT debugMessenger;
            //以下函数用于创建debug messenger
            VkResult CreateDebugMessenger() {
                static PFN_vkDebugUtilsMessengerCallbackEXT DebugUtilsMessengerCallback = [](
                    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                    void* pUserData)->VkBool32 {
                        std::cout << std::format("{}\n\n", pCallbackData->pMessage);
                        return VK_FALSE;
                };
                VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                    .messageSeverity =
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                    .messageType =
                        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                    .pfnUserCallback = DebugUtilsMessengerCallback
                };
                PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger =
                        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
                if (vkCreateDebugUtilsMessenger) {
                    VkResult result = vkCreateDebugUtilsMessenger(instance, &debugUtilsMessengerCreateInfo, nullptr, &debugMessenger);
                    if (result)
                        std::cout << std::format("[ graphicsBase ] ERROR\nFailed to create a debug messenger!\nError code: {}\n", string_VkResult(result));
                    return result;
                }
                std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the function pointer of vkCreateDebugUtilsMessengerEXT!\n");
                return VK_RESULT_MAX_ENUM;
            }

        /*
        * 三、创建window surafce
        * 创建glfw窗口和Vulkan实例，用glfwCreateWindowSurface(...)创建window surface
        * 若用WindowsAPI创建窗口，用vkCreateWin32SurfaceKHR(...)创建window surface
        * 如果有多个窗口（应用程序可以有子窗口），当然也会有多个window surface，以及多个交换链，当前教程不考虑
        */
        private:
            VkSurfaceKHR surface;
        public:
            //Getter
            VkSurfaceKHR Surface() const {
                return surface;
            }

            //该函数用于选择物理设备前
            void Surface(VkSurfaceKHR surface) {
                if (!this->surface)
                    this->surface = surface;
            }

        /*
        * 四、创建逻辑设备
        * 流程
        * 1. 获取物理设备列表
        * 2. 检查物理设备是否满足所需的队列族类型，从中选择能满足要求的设备并顺便取得队列族索引
        * 3. 确定所需的设备级别扩展，不检查是否可用
        * 4. 用vkCreateDevice(...)创建逻辑设备，取得队列
        * 5. 取得物理设备属性、物理设备内存属性，以备之后使用
        *
        * 第5步的内容是以创建逻辑设备成功，确定将要使用的物理设备不再变动为前提的。
        * 若对物理设备的性能有要求，也可以在第2步前自行取得各个物理设备的属性，进行甄选。
        *
        * 三大队列：图形、呈现、计算，GPU必定会有一个同时支持图形和呈现的队列族
        * 如果你的程序没有图形界面（比如，仅仅用于对图像做某种处理的控制台程序），那么呈现队列非必须。
        * 如果你不需要GPU计算或间接渲染（将CPU上的一些计算扔到GPU上然后再从计算结果做渲染），那么计算队列非必须。
        * 如果你只打算搞计算（GPU是高度并行的计算设备）而不搞渲染，那么图形队列非必须。
        * 检查硬件是否支持光追管线的方式与检查是否支持图形和计算管线的方式不同，这里暂不作考虑。
        *
        * 程序逻辑
        * 1. 用GetPhysicalDevices()获取可用物理设备的列表
        * 2. 用DeterminePhysicalDevice(...)判断将要使用的物理设备是否满足要求，如满足则将其记录为所用设备。
        * 这一步需要判断物理设备是否支持window surface，正是因此创建window surface一步须在此之前。
        * 3. 用AddDeviceExtension(...)向deviceExtensions中添加指向各扩展名称的指针。
        * 4. 用CreateDevice(...)创建逻辑设备，若创建成功，该函数会取得队列。
        * 若创建逻辑设备失败：
        * 5. 用DeterminePhysicalDevice(...)更换物理设备。到这一步为止仍没有检查扩展，如果你首先选用的是性能较好的物理设备，
        * 且优先看重性能（而非支持的扩展），那么这一步与下一步可互换。
        * 6. 若遍历所有物理设备后仍无法创建逻辑设备且vkCreateDevice(...)返回VK_ERROR_EXTENSION_NOT_PRESENT，
        * 从DeviceExtensions()复制一份deviceExtensions，嵌套循环中用DeterminePhysicalDevice(...)遍历物理设备（会有代码防止反复取得队列族索引），
        * 用CheckDeviceExtensions(...)检查扩展可用性，对于某物理设备，若不可用的仅为非必要的扩展，
        * 创建一份去除该扩展后的vector，用DeviceExtensions(...)复制给deviceExtensions。然后重新尝试创建逻辑设备。
        */
        private:
            VkPhysicalDevice physicalDevice;
            VkPhysicalDeviceProperties physicalDeviceProperties;
            VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
            std::vector<VkPhysicalDevice> availablePhysicalDevices;

            VkDevice device;
            //有效的索引从0开始，因此使用特殊值VK_QUEUE_FAMILY_IGNORED（为UINT32_MAX）为队列族索引的默认值
            uint32_t queueFamilyIndex_graphics = VK_QUEUE_FAMILY_IGNORED;
            uint32_t queueFamilyIndex_presentation = VK_QUEUE_FAMILY_IGNORED;
            uint32_t queueFamilyIndex_compute = VK_QUEUE_FAMILY_IGNORED;
            VkQueue queue_graphics;
            VkQueue queue_presentation;
            VkQueue queue_compute;

            std::vector<const char*> deviceExtensions;

            //该函数被DeterminePhysicalDevice(...)调用，用于检查物理设备是否满足所需的队列族类型，并将对应的队列族索引返回到queueFamilyIndices，执行成功时直接将索引写入相应成员变量
            VkResult GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, bool enableGraphicsQueue, bool enableComputeQueue, uint32_t (&queueFamilyIndices)[3]) {
                uint32_t queueFamilyCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
                if (!queueFamilyCount)
                    return VK_RESULT_MAX_ENUM;
                std::vector<VkQueueFamilyProperties> queueFamilyPropertieses(queueFamilyCount);
                vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyPropertieses.data());
                auto& [ig, ip, ic] = queueFamilyIndices; //分别对应图形、呈现、计算
                ig = ip = ic = VK_QUEUE_FAMILY_IGNORED;
                //遍历所有队列族的索引
                for (uint32_t i = 0; i < queueFamilyCount; i++) {
                    //这三个VkBool32变量指示是否可获取（指应该被获取且能获取）相应队列族索引
                    VkBool32
                        //只在enableGraphicsQueue为true时获取支持图形操作的队列族的索引
                        supportGraphics = enableGraphicsQueue && queueFamilyPropertieses[i].queueFlags & VK_QUEUE_GRAPHICS_BIT,
                        supportPresentation = false,
                        //只在enableComputeQueue为true时获取支持计算的队列族的索引
                        supportCompute = enableComputeQueue && queueFamilyPropertieses[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
                        //只在创建了window surface时获取支持呈现的队列族的索引
                    if (surface)
                        if (VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportPresentation)) {
                            std::cout << std::format("[ graphicsBase ] ERROR\nFailed to determine if the queue family supports presentation!\nError code: {}\n", string_VkResult(result));
                            return result;
                        }
                    //若某队列族同时支持图形操作和计算
                    if (supportGraphics && supportCompute) {
                        //若需要呈现，最好是三个队列族索引全部相同
                        if (supportPresentation) {
                            ig = ip = ic = i;
                            break;
                        }
                        //除非ig和ic都已取得且相同，否则将它们的值覆写为i，以确保两个队列族索引相同
                        if (ig != ic ||
                            ig == VK_QUEUE_FAMILY_IGNORED)
                            ig = ic = i;
                        //如果不需要呈现，那么已经可以break了
                        if (!surface)
                            break;
                    }
                    //若任何一个队列族索引可以被取得但尚未被取得，将其值覆写为i
                    if (supportGraphics &&
                        ig == VK_QUEUE_FAMILY_IGNORED)
                        ig = i;
                    if (supportPresentation &&
                        ip == VK_QUEUE_FAMILY_IGNORED)
                        ip = i;
                    if (supportCompute &&
                        ic == VK_QUEUE_FAMILY_IGNORED)
                        ic = i;
                }
                //若任何需要被取得的队列族索引尚未被取得，则函数执行失败
                if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphicsQueue ||
                    ip == VK_QUEUE_FAMILY_IGNORED && surface ||
                    ic == VK_QUEUE_FAMILY_IGNORED && enableComputeQueue)
                    return VK_RESULT_MAX_ENUM;
                //函数执行成功时，将所取得的队列族索引写入到成员变量
                queueFamilyIndex_graphics = ig;
                queueFamilyIndex_presentation = ip;
                queueFamilyIndex_compute = ic;
                return VK_SUCCESS;
            }
        public:
            //Getter
            VkPhysicalDevice PhysicalDevice() const {
                return physicalDevice;
            }
            const VkPhysicalDeviceProperties& PhysicalDeviceProperties() const {
                return physicalDeviceProperties;
            }
            const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties() const {
                return physicalDeviceMemoryProperties;
            }
            VkPhysicalDevice AvailablePhysicalDevice(uint32_t index) const {
                return availablePhysicalDevices[index];
            }
            uint32_t AvailablePhysicalDeviceCount() const {
                return uint32_t(availablePhysicalDevices.size());
            }

            VkDevice Device() const {
                return device;
            }
            uint32_t QueueFamilyIndex_Graphics() const {
                return queueFamilyIndex_graphics;
            }
            uint32_t QueueFamilyIndex_Presentation() const {
                return queueFamilyIndex_presentation;
            }
            uint32_t QueueFamilyIndex_Compute() const {
                return queueFamilyIndex_compute;
            }
            VkQueue Queue_Graphics() const {
                return queue_graphics;
            }
            VkQueue Queue_Presentation() const {
                return queue_presentation;
            }
            VkQueue Queue_Compute() const {
                return queue_compute;
            }

            const std::vector<const char*>& DeviceExtensions() const {
                return deviceExtensions;
            }

            //该函数用于创建逻辑设备前
            void AddDeviceExtension(const char* extensionName) {
                AddLayerOrExtension(deviceExtensions, extensionName);
            }
            //该函数用于获取物理设备
            VkResult GetPhysicalDevices() {
                uint32_t deviceCount;
                if (VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr)) {
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to get the count of physical devices!\nError code: {}\n", string_VkResult(result));
                    return result;
                }
                if (!deviceCount)
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to find any physical device supports vulkan!\n"),
                    abort();
                availablePhysicalDevices.resize(deviceCount);
                VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, availablePhysicalDevices.data());
                if (result)
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to enumerate physical devices!\nError code: {}\n", string_VkResult(result));
                return result;
            }
            //该函数用于指定所用物理设备并调用GetQueueFamilyIndices(...)取得队列族索引
            VkResult DeterminePhysicalDevice(uint32_t deviceIndex = 0, bool enableGraphicsQueue = true, bool enableComputeQueue = true) {
                //定义一个特殊值用于标记一个队列族索引已被找过但未找到
                static constexpr uint32_t notFound = INT32_MAX; //== VK_QUEUE_FAMILY_IGNORED & INT32_MAX
                //定义队列族索引组合的结构体
                struct queueFamilyIndexCombination {
                    uint32_t graphics = VK_QUEUE_FAMILY_IGNORED;
                    uint32_t presentation = VK_QUEUE_FAMILY_IGNORED;
                    uint32_t compute = VK_QUEUE_FAMILY_IGNORED;
                };
                //queueFamilyIndexCombinations用于为各个物理设备保存一份队列族索引组合
                static std::vector<queueFamilyIndexCombination> queueFamilyIndexCombinations(availablePhysicalDevices.size());
                auto& [ig, ip, ic] = queueFamilyIndexCombinations[deviceIndex];

                //如果有任何队列族索引已被找过但未找到，返回VK_RESULT_MAX_ENUM
                if (ig == notFound && enableGraphicsQueue ||
                    ip == notFound && surface ||
                    ic == notFound && enableComputeQueue)
                    return VK_RESULT_MAX_ENUM;

                //如果有任何队列族索引应被获取但还未被找过
                if (ig == VK_QUEUE_FAMILY_IGNORED && enableGraphicsQueue ||
                    ip == VK_QUEUE_FAMILY_IGNORED && surface ||
                    ic == VK_QUEUE_FAMILY_IGNORED && enableComputeQueue) {
                    uint32_t indices[3];
                    VkResult result = GetQueueFamilyIndices(availablePhysicalDevices[deviceIndex], enableGraphicsQueue, enableComputeQueue, indices);
                    //若GetQueueFamilyIndices(...)返回VK_SUCCESS或VK_RESULT_MAX_ENUM（vkGetPhysicalDeviceSurfaceSupportKHR(...)执行成功但没找齐所需队列族），
                    //说明对所需队列族索引已有结论，保存结果到queueFamilyIndexCombinations[deviceIndex]中相应变量
                    //应被获取的索引若仍为VK_QUEUE_FAMILY_IGNORED，说明未找到相应队列族，VK_QUEUE_FAMILY_IGNORED（~0u）与INT32_MAX做位与得到的数值等于notFound
                    if (result == VK_SUCCESS ||
                        result == VK_RESULT_MAX_ENUM) {
                        if (enableGraphicsQueue)
                            ig = indices[0] & INT32_MAX;
                        if (surface)
                            ip = indices[1] & INT32_MAX;
                        if (enableComputeQueue)
                            ic = indices[2] & INT32_MAX;
                    }
                    //如果GetQueueFamilyIndices(...)执行失败，return
                    if (result)
                        return result;
                }
                //若以上两个if分支皆不执行，则说明所需的队列族索引皆已被获取，从queueFamilyIndexCombinations[deviceIndex]中取得索引
                else {
                    queueFamilyIndex_graphics = enableGraphicsQueue ? ig : VK_QUEUE_FAMILY_IGNORED;
                    queueFamilyIndex_presentation = surface ? ip : VK_QUEUE_FAMILY_IGNORED;
                    queueFamilyIndex_compute = enableComputeQueue ? ic : VK_QUEUE_FAMILY_IGNORED;
                }
                physicalDevice = availablePhysicalDevices[deviceIndex];
                return VK_SUCCESS;
            }
            //该函数用于创建逻辑设备，并取得队列
            VkResult CreateDevice(VkDeviceCreateFlags flags = 0) {
                float queuePriority = 1.f;
                VkDeviceQueueCreateInfo queueCreateInfos[3] = {
                    {
                        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                        .queueCount = 1,
                        .pQueuePriorities = &queuePriority },
                    {
                        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                        .queueCount = 1,
                        .pQueuePriorities = &queuePriority },
                    {
                        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                        .queueCount = 1,
                        .pQueuePriorities = &queuePriority } };
                uint32_t queueCreateInfoCount = 0;
                if (queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED)
                    queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_graphics;
                if (queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED &&
                    queueFamilyIndex_presentation != queueFamilyIndex_graphics)
                    queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_presentation;
                if (queueFamilyIndex_compute != VK_QUEUE_FAMILY_IGNORED &&
                    queueFamilyIndex_compute != queueFamilyIndex_graphics &&
                    queueFamilyIndex_compute != queueFamilyIndex_presentation)
                    queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_compute;
                // 获取物理设备特性
                VkPhysicalDeviceFeatures physicalDeviceFeatures;
                vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

                VkDeviceCreateInfo deviceCreateInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                    .flags = flags,
                    .queueCreateInfoCount = queueCreateInfoCount,
                    .pQueueCreateInfos = queueCreateInfos,
                    .enabledExtensionCount = uint32_t(deviceExtensions.size()),
                    .ppEnabledExtensionNames = deviceExtensions.data(),
                    .pEnabledFeatures = &physicalDeviceFeatures
                };
                if (VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device)) {
                    std::cout << std::format("[ graphicsBase ] ERROR\nFailed to create a vulkan logical device!\nError code: {}\n", string_VkResult(result));
                // 之前在填写队列的创建信息时，指定了每个队列族中只创建一个队列，所以获取队列时，参数queueIndex为0
                if (queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED)
                    vkGetDeviceQueue(device, queueFamilyIndex_graphics, 0, &queue_graphics);
                if (queueFamilyIndex_presentation != VK_QUEUE_FAMILY_IGNORED)
                    vkGetDeviceQueue(device, queueFamilyIndex_presentation, 0, &queue_presentation);
                if (queueFamilyIndex_compute != VK_QUEUE_FAMILY_IGNORED)
                    vkGetDeviceQueue(device, queueFamilyIndex_compute, 0, &queue_compute);
                    return result;
                }
                vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
                vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
                //输出所用的物理设备名称
                std::cout << std::format("Renderer: {}\n", physicalDeviceProperties.deviceName);
                /*待Ch1-4填充*/
                return VK_SUCCESS;
            }
            //以下函数用于创建逻辑设备失败后
            VkResult CheckDeviceExtensions(std::span<const char*> extensionsToCheck, const char* layerName = nullptr) const {
                /*待Ch1-3填充*/
            }
            void DeviceExtensions(const std::vector<const char*>& extensionNames) {
                deviceExtensions = extensionNames;
            }
        /*
        * 五、创建交换链
        * 流程
        * 1. 填写一大堆信息
        * 2. 创建交换链并取得交换链图像，为交换链图像创建image view
        * 重要的一点是要考虑重建交换链
        * VkImage引用一片设备内存（device memory），将该片内存上的数据用作图像
        * VkImageView指定图像的使用方式
        *
        * 程序逻辑
        * 1. 用GetSurfaceFormats()来取得surface的可用格式到availableSurfaceFormats，用SetSurfaceFormat(...)验证并指定图像格式和色彩空间。
        * 2. 用CreateSwapchain(...)创建交换链。
        * 3. 之后运行中，窗口大小改变时（可通过GLFW的回调函数，或WindowsAPI中的message得知），调用RecreateSwapchain()
        */
        private:
            std::vector <VkSurfaceFormatKHR> availableSurfaceFormats;

            VkSwapchainKHR swapchain;
            std::vector <VkImage> swapchainImages;
            std::vector <VkImageView> swapchainImageViews;
            //保存交换链的创建信息以便重建交换链
            VkSwapchainCreateInfoKHR swapchainCreateInfo = {};

            //该函数被CreateSwapchain(...)和RecreateSwapchain()调用
            VkResult CreateSwapchain_Internal() {
                /*待Ch1-4填充*/
            }

        public:
            const VkFormat& AvailableSurfaceFormat(uint32_t index) const {
                return availableSurfaceFormats[index].format;
            }
            const VkColorSpaceKHR& AvailableSurfaceColorSpace(uint32_t index) const {
                return availableSurfaceFormats[index].colorSpace;
            }
            uint32_t AvailableSurfaceFormatCount() const {
                return uint32_t(availableSurfaceFormats.size());
            }

            VkSwapchainKHR Swapchain() const {
                return swapchain;
            }
            VkImage SwapchainImage(uint32_t index) const {
                return swapchainImages[index];
            }
            VkImageView SwapchainImageView(uint32_t index) const {
                return swapchainImageViews[index];
            }
            uint32_t SwapchainImageCount() const {
                return uint32_t(swapchainImages.size());
            }
            const VkSwapchainCreateInfoKHR& SwapchainCreateInfo() const {
                return swapchainCreateInfo;
            }

            VkResult GetSurfaceFormats() {
                /*待Ch1-4填充*/
            }
            // 用于指定HDR这类特殊的色彩空间，在此之前需手动调用GetSurfaceFormats()来取得surface的可用格式。
            // 若因为不需要指定surface格式或色彩空间而没有手动调用GetSurfaceFormats()，CreateSwapchain(...)将调用它。
            VkResult SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat) {
                /*待Ch1-4填充*/
            }
            //该函数用于创建交换链
            VkResult CreateSwapchain(bool limitFrameRate = true, VkSwapchainCreateFlagsKHR flags = 0) {
                /*待Ch1-4填充*/
            }
            //该函数用于重建交换链
            VkResult RecreateSwapchain() {
                /*待Ch1-4填充*/
            }
    };
    inline graphicsBase graphicsBase::singleton;
}
