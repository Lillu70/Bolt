#pragma once

#include "Vk_Internal_Components.h"

#include <GLFW/glfw3.h>

namespace Bolt
{
    class Vk_Util
    {
    public:
        Vk_Util() = delete;

        static void Create_Image(VkDevice device, GPU gpu, Image_Create_Info& image_create_info, Image_Description& output);
        static void Create_Image_View(VkDevice device, VkFormat format, VkImageAspectFlags aspect_flags, Image_Description& output);
        static void Create_Image_View(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, VkImageView& output);
        static void Begin_Single_Time_Commands(VkDevice device, Command_Pools command_pools, VkCommandBuffer& output);
        static void End_Single_Time_Commands(VkDevice device, Command_Pools command_pools, Device_Queues device_queues, VkCommandBuffer& output);
        static void Transition_Image_Layout(VkDevice device, Command_Pools command_pools, Device_Queues device_queues, Image_Layout_Transition_Info transition_info);
        static bool Find_Suitable_Memory(GPU gpu, uint32_t type_filter, VkMemoryPropertyFlags properties, uint32_t& output);
        static void Create_Buffer(VkDevice device, GPU gpu, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, Buffer_Description& buffer);
        static void Copy_Buffer(VkDevice device, Command_Pools command_pools, Device_Queues device_queues, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
        static void Copy_Buffer_To_Image(VkDevice device, Command_Pools command_pools, Device_Queues device_queues, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        static void Create_Staging_Buffer(VkDevice device, GPU gpu, void* source, VkDeviceSize size, Buffer_Description& output);
        static void Begin_Command_Buffer(VkCommandBuffer& cmd_buffer);
        static void Begin_Render_Pass(VkCommandBuffer cmd_buffer, VkRenderPass render_pass, Swapchain_Description& swapchain, uint32_t image_index, glm::vec3& clear_color);
        static void CMD_Set_View_Port(VkCommandBuffer cmd_buffer, Swapchain_Description& swapchain);
        static void CMD_Set_Scissors(VkCommandBuffer cmd_buffer, Swapchain_Description& swapchain);
    };

    class Vk_Init
    {
    public:
        Vk_Init() = delete;

        //Renderer level resources:
        static void Create_Instance(PFN_vkDebugUtilsMessengerCallbackEXT user_callback, VkInstance& output);
        static void Create_Window_Surface(VkInstance instance, GLFWwindow* window, VkSurfaceKHR& output);
        static void Create_Debug_Messanger(VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT user_callback, VkDebugUtilsMessengerEXT& output);
        static void Pick_Physical_Device(VkInstance instance, VkSurfaceKHR surface_example, GPU& output);
        static void Create_Logical_Device(GPU gpu, VkDevice& output);
        static void Retrive_Device_Queues(VkDevice device, Queue_Families queue_families, Device_Queues& output);
        static void Create_Render_Pass(VkDevice device, Format_Description formats, VkRenderPass& output);
        static void Create_Descriptor_Set_Layout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& layout_bindings, VkDescriptorSetLayout& output);
        static void Create_Graphics_Pipline_Layout(VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts, VkPipelineLayout& output, std::optional<VkPushConstantRange> push_constant = std::optional<VkPushConstantRange>());
        static void Create_Graphics_Pipeline(VkDevice device, VkPipelineLayout layout, VkRenderPass render_pass, Shader_Modules shaders, VkPipeline& output);
        static void Create_Command_Pools(VkDevice device, Queue_Families queue_families, Command_Pools& output);
        static void Create_Texture_Sampler(VkDevice device, GPU gpu, VkSampler& output);

        //Window level resources:
        static void Create_Swapchain(VkDevice device, GPU gpu, VkSurfaceKHR surface, GLFWwindow* window, Swapchain_Description& output);
        static void Create_Depth_Resources(VkDevice device, GPU gpu, Swapchain_Description swapchain, Command_Pools command_pools, Device_Queues device_queues, Image_Description& output);
        static void Create_Framebuffer(VkDevice device, VkRenderPass render_pass, Image_Description depth_image, Swapchain_Description& output);
        static void Create_Descriptor_Pool(VkDevice device, VkDescriptorSetLayout layout, const std::vector<VkDescriptorPoolSize>& pool_sizes, uint32_t descriptor_count, VkDescriptorPool& output);
        static void Create_Command_Buffers(VkDevice device, Command_Pools pools, uint32_t buffer_count, std::vector<VkCommandBuffer>& output);
        static void Create_Frame_Syncronization_Objects(VkDevice device, Frame_Syncronization& output);

    private:
        static void Populate_Debug_Messenger_Create_Info(VkDebugUtilsMessengerCreateInfoEXT& create_info, PFN_vkDebugUtilsMessengerCallbackEXT user_callback);
        static VkResult Create_Debug_Utils_Messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info_ptr, const VkAllocationCallbacks* allocator_ptr, VkDebugUtilsMessengerEXT* debug_messanger_ptr);
        static bool Check_Device_Extension_Support(VkPhysicalDevice device);
        static Swapchain_Support_Details Query_Swapchain_Support(VkPhysicalDevice device, VkSurfaceKHR surface);
        static Queue_Family_Indices Find_Queue_Families(VkPhysicalDevice device, VkSurfaceKHR surface);
    };

    class Vk_Dest final
    {
    public:
        Vk_Dest() = delete;

    public:
        static void Clear_Swapchain(VkDevice device, Swapchain_Description& input);
        static void Clear_Image(VkDevice device, Image_Description& input);
        static void Clear_Buffer(VkDevice device, Buffer_Description& input);
        static void Clear_Shader_Modules(VkDevice device, Shader_Modules& input);
        static void Clear_Frame_Data(VkDevice device, Frame_Data& input);

        static void Destroy_Debug_Utils_Messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator_ptr);
    };
}

