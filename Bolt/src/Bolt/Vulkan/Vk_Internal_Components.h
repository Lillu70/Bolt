#pragma once
#include <vulkan/vulkan.h>

#include <vector>
#include <array>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace Bolt
{
    struct Queue_Families
    {
        uint32_t graphics = 0;
        uint32_t present = 0;
    };

    struct Format_Description
    {
        VkSurfaceFormatKHR surface;
        VkFormat depth;
    };

    struct GPU
    {
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        Queue_Families queue_families;
        Format_Description formats;
    };

    struct Device_Queues
    {
        VkQueue graphics = VK_NULL_HANDLE;
        VkQueue present = VK_NULL_HANDLE;
    };

    struct Shader_Modules
    {
        VkShaderModule frag = VK_NULL_HANDLE;
        VkShaderModule vert = VK_NULL_HANDLE;
    };

    struct Command_Pools
    {
        VkCommandPool reset = VK_NULL_HANDLE;
        VkCommandPool transient = VK_NULL_HANDLE;
    };

    struct Swapchain_Description
    {
        VkSwapchainKHR handle = VK_NULL_HANDLE;
        VkExtent2D extent = { 0,0 };

        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
        std::vector<VkFramebuffer> framebuffers;
    };

    struct Image_Description
    {
        VkImage handle = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
    };

    struct Buffer_Description
    {
        VkBuffer handle = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
    };

    struct Frame_Syncronization
    {
        VkSemaphore image_available_semaphore = VK_NULL_HANDLE;
        VkSemaphore render_finished_semaphore = VK_NULL_HANDLE;
        VkFence render_fence = VK_NULL_HANDLE;
    };

    struct Frame_Data
    {
        uint32_t swapchain_image_index = std::numeric_limits<uint32_t>::max();
        Frame_Syncronization sync;
        VkCommandBuffer cmd_buffer = VK_NULL_HANDLE;
        Buffer_Description camera_buffer;
        VkDescriptorSet camera_descriptor = VK_NULL_HANDLE;
    };

    struct Push_Constant
    {
        glm::mat4 model_matrix = glm::mat4(1);
        glm::mat4 normal_matrix = glm::mat4(1);
    };

    struct Scene_Uniform_Buffer_Object
    {
        glm::mat4 inverse_view = glm::mat4(1);
        glm::mat4 view_proj = glm::mat4(1);
        alignas(16) glm::vec3 light_direction = glm::vec3(1);
        alignas(16) glm::vec3 light_color = glm::vec3(1);
        alignas(16) glm::vec3 amplient_color = glm::vec3(1);
    };

    struct Material_Uniform_Buffer_Object
    {
        float roughness = 32.f;
    };

    typedef Material_Uniform_Buffer_Object Material_Properties;

    struct Swapchain_Support_Details
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };
    
    struct Queue_Family_Indices
    {
        std::optional<uint32_t> m_graphics_family;
        std::optional<uint32_t> m_present_family;

        bool Is_Complete() const
        {
            return m_graphics_family.has_value() && m_present_family.has_value();
        }
    };

    struct Image_Create_Info
    {
        uint32_t width = 0;
        uint32_t height = 0;
        VkFormat format;
        VkImageTiling tiling;
        VkImageUsageFlags usage;
        VkMemoryPropertyFlags properties;
    };

    struct Image_Layout_Transition_Info
    {
        VkImage image = VK_NULL_HANDLE;
        VkFormat format;
        VkImageLayout old_layout;
        VkImageLayout new_layout;
    };

    struct Vertex
    {
        glm::vec3 m_position;
        glm::vec3 m_normal;
        glm::vec2 m_uv;

        bool operator==(const Vertex& other) const
        {
            return m_position == other.m_position && m_normal == other.m_normal && m_uv == other.m_uv;
        }

        static VkVertexInputBindingDescription Get_Binding_Description()
        {
            VkVertexInputBindingDescription binding_description{};

            binding_description.binding = 0;
            binding_description.stride = sizeof(Vertex);
            binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return binding_description;
        }

        static std::array<VkVertexInputAttributeDescription, 3> Get_Attribute_Descriptions()
        {
            std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};

            uint32_t location = 0;

            attribute_descriptions[0].binding = 0;
            attribute_descriptions[0].location = location++;
            attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[0].offset = offsetof(Vertex, m_position);

            attribute_descriptions[1].binding = 0;
            attribute_descriptions[1].location = location++;
            attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[1].offset = offsetof(Vertex, m_normal);

            attribute_descriptions[2].binding = 0;
            attribute_descriptions[2].location = location++;
            attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attribute_descriptions[2].offset = offsetof(Vertex, m_uv);

            return attribute_descriptions;
        }
    };
}

namespace std
{
    template<> struct hash<Bolt::Vertex>
    {
        size_t operator()(Bolt::Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.m_position) ^
                (hash<glm::vec3>()(vertex.m_normal) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.m_uv) << 1);
        }
    };
}