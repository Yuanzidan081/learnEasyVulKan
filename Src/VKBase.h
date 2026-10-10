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
                /*待Ch1-3填充*/
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
            }
            //以下函数用于创建Vulkan实例失败后
            // 设计思路：将传入的span中不可用的层和扩展设置为nullptr，然后与原本保存的instanceLayers和instanceExtensions比对，
            // 即可确定哪些层和扩展不可用。返回值则指示在获取可用层或扩展列表时是否发生错误。
            // std::span 是 C++20 引入的非持有、轻量视图（view），用来代表一段连续元素的序列。
            // 它只是记录「起始指针 + 元素个数」，用来包装数组如vector连续缓冲区，避免裸指针 + 长度成对传递。
            VkResult CheckInstanceLayers(std::span<const char*> layersToCheck) {
                /*待Ch1-3填充*/
            }
            void InstanceLayers(const std::vector<const char*>& layerNames) {
                instanceLayers = layerNames;
            }

            VkResult CheckInstanceExtensions(std::span<const char*> extensionsToCheck, const char* layerName = nullptr) const {
                /*待Ch1-3填充*/
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
                /*待Ch1-3填充*/
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
                /*待Ch1-3填充*/
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
                /*待Ch1-3填充*/
            }
            //该函数用于指定所用物理设备并调用GetQueueFamilyIndices(...)取得队列族索引
            VkResult DeterminePhysicalDevice(uint32_t deviceIndex = 0, bool enableGraphicsQueue, bool enableComputeQueue = true) {
                /*待Ch1-3填充*/
            }
            //该函数用于创建逻辑设备，并取得队列
            VkResult CreateDevice(VkDeviceCreateFlags flags = 0) {
                /*待Ch1-3填充*/
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
