#include "Vk_Assist.h"
#include "../Core/Assert.h"

#include <set>
#include <algorithm>

#ifdef _DEBUG
bool enable_validation_layers = true;
#else
bool enable_validation_layers = false;
#endif

const std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

void Bolt::Vk_Init::Create_Instance(PFN_vkDebugUtilsMessengerCallbackEXT user_callback, VkInstance& output)
{
    ASSERT(output == VK_NULL_HANDLE, "instance already created!");

    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "CORE";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_2;

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data());

    std::vector<const char*> required_extensions;

    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    required_extensions.reserve(glfw_extension_count + enable_validation_layers);

    for (uint32_t i = 0; i < glfw_extension_count; i++)
        required_extensions.push_back(glfw_extensions[i]);

    if (enable_validation_layers)
        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);


    std::vector<const char*> missing_extensions;
    for (const auto extension_name : required_extensions)
    {
        bool extension_missing = true;
        for (const auto& extension : available_extensions)
            if (strcmp(extension_name, extension.extensionName) == 0)
            {
                extension_missing = false;
                break;
            }

        if (extension_missing)
            missing_extensions.push_back(extension_name);
    }

    if (missing_extensions.empty() == false)
    {
        std::string error_message = "Missing Reguired Vulkan Extension(s):\n";
        for (auto missing_extension : missing_extensions)
            error_message += std::string("\t") + missing_extension + "\n";

        ERROR(error_message);
    }

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    Populate_Debug_Messenger_Create_Info(debug_create_info, user_callback);

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();
    create_info.enabledLayerCount = 0;
    create_info.pNext = nullptr;

    if (enable_validation_layers)
    {
        //Check for validation layer support.
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for (const char* layer_name : validation_layers)
        {
            bool layer_missing = true;
            for (const auto& layer_properties : available_layers)
                if (strcmp(layer_name, layer_properties.layerName) == 0)
                {
                    layer_missing = false;
                    break;
                }

            if (layer_missing)
            {
                WARN(std::string("Validation layer: ") + layer_name + " requested, but not available\n Validation layers disabled!");
                enable_validation_layers = false;
                break;
            }
                
        }

        if (enable_validation_layers)
        {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();
            Populate_Debug_Messenger_Create_Info(debug_create_info, user_callback);
            create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
        } 
    }
    

    if (vkCreateInstance(&create_info, nullptr, &output) != VK_SUCCESS)
        ERROR("Failed to create a vulkan instance");
}

void Bolt::Vk_Init::Create_Window_Surface(VkInstance instance, GLFWwindow* window, VkSurfaceKHR& output)
{
    ASSERT(instance, "Vulkan instance is a nullptr!");
    ASSERT(window, "Window instance is a nullptr!");
    ASSERT(output == VK_NULL_HANDLE, "Surface already created!");

    if (glfwCreateWindowSurface(instance, window, nullptr, &output) != VK_SUCCESS)
        ERROR("failed to create window surface!");
}

void Bolt::Vk_Init::Create_Debug_Messanger(VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT user_callback, VkDebugUtilsMessengerEXT& output)
{
    ASSERT(instance, "Vulkan instance is a nullptr!");
    ASSERT(output == VK_NULL_HANDLE, "Debug messanger already created!");

    if (!enable_validation_layers) return;

    VkDebugUtilsMessengerCreateInfoEXT create_info;
    Populate_Debug_Messenger_Create_Info(create_info, user_callback);

    if (Create_Debug_Utils_Messenger(instance, &create_info, nullptr, &output) != VK_SUCCESS)
        ERROR("Failed to set up debug messenger");
}

void Bolt::Vk_Init::Pick_Physical_Device(VkInstance instance, VkSurfaceKHR surface_example, GPU& output)
{
    ASSERT(output.physical_device == VK_NULL_HANDLE, "Physical device already picked!");

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if (device_count == 0)
        ERROR("Failed to find GPUs with Vulkan support");

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    Swapchain_Support_Details swapchain_support;

    for (const auto& device : devices)
    {
        VkPhysicalDeviceFeatures supported_features;
        vkGetPhysicalDeviceFeatures(device, &supported_features);

        Queue_Family_Indices indices = Find_Queue_Families(device, surface_example);

        bool extensions_supported = Check_Device_Extension_Support(device);

        bool swapchain_adequate = false;
        if (extensions_supported)
        {
            swapchain_support = Query_Swapchain_Support(device, surface_example);
            swapchain_adequate = !swapchain_support.formats.empty() && !swapchain_support.present_modes.empty();
        }

        if (indices.Is_Complete() && extensions_supported && swapchain_adequate && supported_features.samplerAnisotropy)
        {
            output.physical_device = device;
            output.queue_families.graphics = indices.m_graphics_family.value();
            output.queue_families.present = indices.m_present_family.value();
            output.formats.surface = swapchain_support.formats[0];
            for (const auto& available_format : swapchain_support.formats)
                if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    output.formats.surface = available_format;
                    break;
                }

            std::vector<VkFormat> candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

            bool depth_format_found = false;
            for (VkFormat format : candidates)
            {
                VkFormatProperties properties;
                vkGetPhysicalDeviceFormatProperties(device, format, &properties);

                if ((properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                {
                    output.formats.depth = format;
                    depth_format_found = true;
                    break;
                }

            }

            ASSERT(depth_format_found, "failed to find supported format!");

            return;
        }
    }

    ERROR("Failed to find a suitable physical device!");
}

void Bolt::Vk_Init::Create_Logical_Device(GPU gpu, VkDevice& output)
{
    ASSERT(gpu.physical_device, "Physical device is a nullptr!");
    ASSERT(output == VK_NULL_HANDLE, "Logical device already created!");

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = { gpu.queue_families.graphics, gpu.queue_families.present };

    float queuePriority = 1.0f;
    for (uint32_t queue_family : unique_queue_families)
    {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queuePriority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.queueCreateInfoCount = uint32_t(queue_create_infos.size());
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = uint32_t(device_extensions.size());
    create_info.ppEnabledExtensionNames = device_extensions.data();

    if (enable_validation_layers)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    }
    else
        create_info.enabledLayerCount = 0;

    if (vkCreateDevice(gpu.physical_device, &create_info, nullptr, &output) != VK_SUCCESS)
        ERROR("failed to create logical device!");
}

void Bolt::Vk_Init::Retrive_Device_Queues(VkDevice device, Queue_Families queue_families, Device_Queues& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(output.graphics == VK_NULL_HANDLE && output.present == VK_NULL_HANDLE, "Device queues already retrived!");

    vkGetDeviceQueue(device, queue_families.graphics, 0, &output.graphics);
    vkGetDeviceQueue(device, queue_families.present, 0, &output.present);
}

void Bolt::Vk_Init::Create_Render_Pass(VkDevice device, Formats formats, VkRenderPass& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(output == VK_NULL_HANDLE, "Render pass already created!");

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { Vk_Util::Color_Clear_Attach(formats), Vk_Util::Depth_Clear_Attach(formats) };

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &render_pass_info, nullptr, &output) != VK_SUCCESS)
        ERROR("failed to create render pass!");
}


void Bolt::Vk_Init::Create_Render_Pass(VkDevice device, const std::vector<VkAttachmentDescription>& color_attachments, std::optional<VkAttachmentDescription> depth_attachment, VkRenderPass& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(!color_attachments.empty() || depth_attachment.has_value(), "Invalid attachment layout!");
    ASSERT(output == VK_NULL_HANDLE, "Render pass already created!");
    
    std::vector<VkAttachmentReference> color_attachment_references(color_attachments.size());
    color_attachment_references.resize(color_attachment_references.size());

    for (uint32_t i = 0; i < color_attachment_references.size(); i++)
    {
        color_attachment_references[i].attachment = i;
        color_attachment_references[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = (u32)color_attachments.size();
    subpass.pColorAttachments = color_attachment_references.data();

    std::vector<VkAttachmentDescription> attachments(color_attachments);
    if (depth_attachment.has_value())
    {
        VkAttachmentReference depth_attachment_ref{};
        depth_attachment_ref.attachment = (u32)color_attachments.size();
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;
        
        attachments.push_back(depth_attachment.value());
    }
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = (uint32_t)attachments.size();
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &render_pass_info, nullptr, &output) != VK_SUCCESS)
        ERROR("failed to create render pass!");
}


void Bolt::Vk_Init::Create_Descriptor_Set_Layout(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding>& layout_bindings, VkDescriptorSetLayout& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(!layout_bindings.empty(), "Layout bindings vector is empty!");
    ASSERT(output == VK_NULL_HANDLE, "Descriptor set layout already created!");

    VkDescriptorSetLayoutCreateInfo layout_info{};

    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.pBindings = layout_bindings.data();
    layout_info.bindingCount = (uint32_t)layout_bindings.size();

    if (vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &output) != VK_SUCCESS)
        ERROR("failed to create descriptor set layout!");
}

void Bolt::Vk_Init::Create_Graphics_Pipline_Layout(VkDevice device, const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts, VkPipelineLayout& output, std::optional<VkPushConstantRange> push_constant)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(output == VK_NULL_HANDLE, "Graphics pipeline layout already created!");

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = (uint32_t)descriptor_set_layouts.size();
    pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
    if (push_constant.has_value())
    {
        pipeline_layout_info.pPushConstantRanges = &push_constant.value();
        pipeline_layout_info.pushConstantRangeCount = 1;
    }
    else
    {
        pipeline_layout_info.pushConstantRangeCount = 0; // Optional
        pipeline_layout_info.pPushConstantRanges = nullptr; // Optional
    }


    if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &output) != VK_SUCCESS)
        ERROR("failed to create pipeline layout!");
}

void Bolt::Vk_Init::Create_Graphics_Pipeline(VkDevice device, VkPipelineLayout layout, VkRenderPass render_pass, Shader_Modules shaders, VkPipeline& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(layout, "Pipeline layout is a nullptr!");
    ASSERT(render_pass, "Render pass is a nullptr!");
    ASSERT(shaders.frag, "Shader module (frag) is a nullptr!");
    ASSERT(shaders.vert, "Shader module (vert) is a nullptr!");
    ASSERT(output == VK_NULL_HANDLE, "Graphics pipeline already created!");

    VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = shaders.vert;
    vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = shaders.frag;
    frag_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

    auto binding_description = Vertex::Get_Binding_Description();
    auto attribute_descriptions = Vertex::Get_Attribute_Descriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = uint32_t(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    constexpr bool alpha_blend = true;

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    if (alpha_blend)
    {
        color_blend_attachment.blendEnable = VK_TRUE;
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }
    else
    {
        color_blend_attachment.blendEnable = VK_FALSE;
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    }

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f; // Optional
    color_blending.blendConstants[1] = 0.0f; // Optional
    color_blending.blendConstants[2] = 0.0f; // Optional
    color_blending.blendConstants[3] = 0.0f; // Optional

    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f; // Optional
    depth_stencil.maxDepthBounds = 1.0f; // Optional
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front = {}; // Optional
    depth_stencil.back = {}; // Optional

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;

    pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipeline_info.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &output) != VK_SUCCESS)
        ERROR("failed to create graphics pipeline!");
}

void Bolt::Vk_Init::Create_Command_Pools(VkDevice device, Queue_Families queue_families, Command_Pools& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(output.reset == VK_NULL_HANDLE && output.transient == VK_NULL_HANDLE, "Command pool(s) already created!");

    {
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = queue_families.graphics;

        if (vkCreateCommandPool(device, &pool_info, nullptr, &output.reset) != VK_SUCCESS)
            ERROR("failed to create command pool!");
    }

    {
        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        pool_info.queueFamilyIndex = queue_families.graphics;

        if (vkCreateCommandPool(device, &pool_info, nullptr, &output.transient) != VK_SUCCESS)
            ERROR("failed to create command pool!");
    }

}

void Bolt::Vk_Init::Create_Texture_Sampler(VkDevice device, GPU gpu, VkSampler& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(gpu.physical_device, "Physical device is a nullptr!");
    ASSERT(output == VK_NULL_HANDLE, "Texture sampler already created!");

    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_FALSE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(gpu.physical_device, &properties);

    sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    if (vkCreateSampler(device, &sampler_info, nullptr, &output) != VK_SUCCESS)
        ERROR("failed to create texture sampler!");
}

void Bolt::Vk_Init::Create_Swapchain(VkDevice device, GPU gpu, VkSurfaceKHR surface, GLFWwindow* window, Swapchain_Description& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(gpu.physical_device, "Physical device is a nullptr!");
    ASSERT(surface, "Surface device is a nullptr!");
    ASSERT(window, "Window is a nullptr!");
    ASSERT(output.images.empty(), "Surface image vector contains elements already!");
    ASSERT(output.image_views.empty(), "Surface image view vector contains elements already!");
    ASSERT(output.handle == VK_NULL_HANDLE, "Swapchain already created!");

    const Swapchain_Support_Details& swapchain_support = Query_Swapchain_Support(gpu.physical_device, surface);

    VkSurfaceFormatKHR surface_format = gpu.formats.surface;

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& available_present_mode : swapchain_support.present_modes)
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }

    glm::uvec2 window_dimensions;
    {
        int32_t x;
        int32_t y;
        glfwGetFramebufferSize(window, &x, &y);
        window_dimensions = { (uint32_t)x, (uint32_t)y };
    }

    output.extent = swapchain_support.capabilities.currentExtent;
    if (output.extent.width == std::numeric_limits<uint32_t>::max())
    {
        VkExtent2D actual_extent = {
            static_cast<uint32_t>(window_dimensions.x),
            static_cast<uint32_t>(window_dimensions.y)
        };

        actual_extent.width = std::clamp(actual_extent.width, swapchain_support.capabilities.minImageExtent.width, swapchain_support.capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, swapchain_support.capabilities.minImageExtent.height, swapchain_support.capabilities.maxImageExtent.height);

        output.extent = actual_extent;
    }

    ASSERT((output.extent.width && output.extent.height), "Invalid Window Size!");

    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;

    if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount)
        image_count = swapchain_support.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = output.extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    Queue_Families queue_families = gpu.queue_families;

    uint32_t queue_family_indices[] = { queue_families.graphics, queue_families.present };

    if (queue_families.graphics != queue_families.present)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0; // Optional
        create_info.pQueueFamilyIndices = nullptr; // Optional
    }

    create_info.preTransform = swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &create_info, nullptr, &output.handle) != VK_SUCCESS)
        ERROR("failed to create swap chain!");

    vkGetSwapchainImagesKHR(device, output.handle, &image_count, nullptr);
    output.images.resize(image_count);
    vkGetSwapchainImagesKHR(device, output.handle, &image_count, output.images.data());

    output.image_views.resize(image_count);
    for (size_t i = 0; i < output.image_views.size(); i++)
        Vk_Util::Create_Image_View(device, output.images[i], gpu.formats.surface.format, VK_IMAGE_ASPECT_COLOR_BIT, output.image_views[i]);
}

void Bolt::Vk_Init::Create_Depth_Resources(VkDevice device, GPU gpu, Swapchain_Description swapchain, Command_Pools command_pools, Device_Queues device_queues, Image_Description& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(gpu.physical_device, "Physical device is a nullptr!");
    ASSERT(swapchain.handle, "Swapchain is a nullptr!");
    ASSERT(swapchain.extent.width && swapchain.extent.height, "Swapchain extent is invalid!");
    ASSERT(output.handle == VK_NULL_HANDLE, "Depth image already created!");
    ASSERT(output.memory == VK_NULL_HANDLE, "Depth memory already allocated!");
    ASSERT(output.view == VK_NULL_HANDLE, "Depth view already created!");

    Image_Create_Info image_create_info{};
    image_create_info.format = gpu.formats.depth;
    image_create_info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_create_info.width = swapchain.extent.width;
    image_create_info.height = swapchain.extent.height;
    Vk_Util::Create_Image(device, gpu, image_create_info, output);
    Vk_Util::Create_Image_View(device, gpu.formats.depth, VK_IMAGE_ASPECT_DEPTH_BIT, output);

    Image_Layout_Transition_Info transition_info;
    transition_info.image = output.handle;
    transition_info.format = gpu.formats.depth;
    transition_info.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    transition_info.new_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    Vk_Util::Transition_Image_Layout(device, command_pools, device_queues, transition_info);
}

void Bolt::Vk_Init::Create_Framebuffers(VkDevice device, VkRenderPass render_pass, Image_Description depth_image, Swapchain_Description& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(render_pass, "render_pass is a nullptr!");
    ASSERT(depth_image.handle, "Depth image is a nullptr!");
    ASSERT(depth_image.view, "Depth image view is a nullptr!");
    ASSERT(!output.images.empty(), "Surface image vector is empty!");
    ASSERT(!output.image_views.empty(), "Surface image view vector is empty!");
    ASSERT(output.handle, "Swapchain is a nullptr!");

    output.framebuffers.resize(output.images.size());

    for (size_t i = 0; i < output.framebuffers.size(); i++)
    {
        std::array<VkImageView, 2> attachments = { output.image_views[i], depth_image.view };

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass;
        framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = output.extent.width;
        framebuffer_info.height = output.extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(device, &framebuffer_info, nullptr, &output.framebuffers[i]) != VK_SUCCESS)
            ERROR("failed to create framebuffer!");
    }
}

void Bolt::Vk_Init::Create_Descriptor_Pool(VkDevice device, VkDescriptorSetLayout layout, const std::vector<VkDescriptorPoolSize>& pool_sizes, uint32_t descriptor_count, VkDescriptorPool& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(layout, "Layout is a nullptr!");
    ASSERT(descriptor_count, "Descriptor count is 0!");
    ASSERT(!pool_sizes.empty(), "Pools sizes is empty!");
    ASSERT(output == VK_NULL_HANDLE, "Descriptor pool already created!");

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = static_cast<uint32_t>(descriptor_count);

    if (vkCreateDescriptorPool(device, &pool_info, nullptr, &output) != VK_SUCCESS)
        ERROR("failed to create descriptor pool!");
}

void Bolt::Vk_Init::Create_Command_Buffers(VkDevice device, Command_Pools pools, uint32_t buffer_count, std::vector<VkCommandBuffer>& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(pools.reset, "Command pool is a nullptr!");
    ASSERT(buffer_count, "Buffer count is 0!");
    ASSERT(output.empty(), "Command buffer vector already contains elements!");

    output.resize(buffer_count);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = pools.reset;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = uint32_t(output.size());

    if (vkAllocateCommandBuffers(device, &alloc_info, output.data()) != VK_SUCCESS)
        ERROR("failed to allocate command buffers!");
}

void Bolt::Vk_Init::Create_Frame_Syncronization_Objects(VkDevice device, Frame_Syncronization& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(output.image_available_semaphore == VK_NULL_HANDLE, "Image available semaphore already created!");
    ASSERT(output.render_finished_semaphore == VK_NULL_HANDLE, "Render finished semaphore already created!");
    ASSERT(output.render_fence == VK_NULL_HANDLE, "Render fence already created!");

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device, &semaphore_info, nullptr, &output.image_available_semaphore) != VK_SUCCESS)
        ERROR("Failed to create semaphore (image_available)!");

    if (vkCreateSemaphore(device, &semaphore_info, nullptr, &output.render_finished_semaphore) != VK_SUCCESS)
        ERROR("Failed to create semaphore (render_finished)!");

    if (vkCreateFence(device, &fence_info, nullptr, &output.render_fence) != VK_SUCCESS)
        ERROR("Failed to create fence!");
}

void Bolt::Vk_Util::Create_Image(VkDevice device, GPU gpu, Image_Create_Info& image_create_info, Image_Description& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(output.handle == VK_NULL_HANDLE, "Image already created!");

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = image_create_info.width;
    image_info.extent.height = image_create_info.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = image_create_info.format;
    image_info.tiling = image_create_info.tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = image_create_info.usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &image_info, nullptr, &output.handle) != VK_SUCCESS)
        ERROR("failed to create image!");

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(device, output.handle, &memory_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = memory_requirements.size;
    if (!Find_Suitable_Memory(gpu, memory_requirements.memoryTypeBits, image_create_info.properties, alloc_info.memoryTypeIndex))
        ERROR_NO_SUITABLE_DEVICE_MEMORY();

    if (vkAllocateMemory(device, &alloc_info, nullptr, &output.memory) != VK_SUCCESS)
        ERROR("failed to allocate image memory!");

    vkBindImageMemory(device, output.handle, output.memory, 0);
}

void Bolt::Vk_Util::Create_Image_View(VkDevice device, VkFormat format, VkImageAspectFlags aspect_flags, Image_Description& output)
{
    Create_Image_View(device, output.handle, format, aspect_flags, output.view);
}

void Bolt::Vk_Util::Create_Image_View(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, VkImageView& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(image, "Image is a nullptr!");
    ASSERT(output == VK_NULL_HANDLE, "Image view already created!");

    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &view_info, nullptr, &output) != VK_SUCCESS)
        ERROR("failed to create texture image view!");
}

void Bolt::Vk_Util::Begin_Single_Time_Commands(VkDevice device, Command_Pools command_pools, VkCommandBuffer& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(command_pools.transient, "Transient command pool a nullptr!");
    ASSERT(output == VK_NULL_HANDLE, "Command buffer aleady allocated!");

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pools.transient;
    alloc_info.commandBufferCount = 1;

    vkAllocateCommandBuffers(device, &alloc_info, &output);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(output, &beginInfo);
}

void Bolt::Vk_Util::End_Single_Time_Commands(VkDevice device, Command_Pools command_pools, Device_Queues device_queues, VkCommandBuffer& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(command_pools.transient, "Transient command pool a nullptr!");
    ASSERT(device_queues.graphics, "Device graphics queue a nullptr!");
    ASSERT(output, "Command buffer NOT allocated!");

    vkEndCommandBuffer(output);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &output;

    vkQueueSubmit(device_queues.graphics, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(device_queues.graphics);

    vkFreeCommandBuffers(device, command_pools.transient, 1, &output);

    output = VK_NULL_HANDLE;
}

void Bolt::Vk_Util::Transition_Image_Layout(VkDevice device, Command_Pools command_pools, Device_Queues device_queues, Image_Layout_Transition_Info transition_info)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(transition_info.image, "Image is a nullptr!");

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    Begin_Single_Time_Commands(device, command_pools, command_buffer);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = transition_info.old_layout;
    barrier.newLayout = transition_info.new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = transition_info.image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (transition_info.new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (transition_info.format == VK_FORMAT_D32_SFLOAT_S8_UINT || transition_info.format == VK_FORMAT_D24_UNORM_S8_UINT)
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;


    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (transition_info.old_layout == VK_IMAGE_LAYOUT_UNDEFINED && transition_info.new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (transition_info.old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && transition_info.new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (transition_info.old_layout == VK_IMAGE_LAYOUT_UNDEFINED && transition_info.new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
        ERROR("unsupported layout transition!");

    vkCmdPipelineBarrier(
        command_buffer,
        source_stage,
        destination_stage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    End_Single_Time_Commands(device, command_pools, device_queues, command_buffer);
}

bool Bolt::Vk_Util::Find_Suitable_Memory(GPU gpu, uint32_t type_filter, VkMemoryPropertyFlags properties, uint32_t& output)
{
    ASSERT(gpu.physical_device, "Physical device is a nullptr!");

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(gpu.physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
        if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            output = i;
            return true;
        }

    return false;
}

void Bolt::Vk_Util::Create_Buffer(VkDevice device, GPU gpu, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, Buffer_Description& buffer)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(gpu.physical_device, "Physical device is a nullptr!");
    ASSERT(buffer.handle == VK_NULL_HANDLE, "Buffer already created!");
    ASSERT(buffer.memory == VK_NULL_HANDLE, "Buffer memory already allocated created!");
    ASSERT(size, "Buffer size is 0!");

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer.handle) != VK_SUCCESS)
        ERROR("failed to create vertex buffer!");

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device, buffer.handle, &memory_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = memory_requirements.size;

    if (!Vk_Util::Find_Suitable_Memory(gpu, memory_requirements.memoryTypeBits, properties, alloc_info.memoryTypeIndex))
        ERROR_NO_SUITABLE_DEVICE_MEMORY();

    if (vkAllocateMemory(device, &alloc_info, nullptr, &buffer.memory) != VK_SUCCESS)
        ERROR("failed to allocate vertex buffer memory!");

    vkBindBufferMemory(device, buffer.handle, buffer.memory, 0);
}

void Bolt::Vk_Util::Copy_Buffer(VkDevice device, Command_Pools command_pools, Device_Queues device_queues, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(src_buffer, "Source buffer is a nullptr!");
    ASSERT(dst_buffer, "Destination buffer is a nullptr!");
    ASSERT(size, "buffer size is 0");

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    Begin_Single_Time_Commands(device, command_pools, command_buffer);

    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0; // Optional
    copy_region.dstOffset = 0; // Optional
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    End_Single_Time_Commands(device, command_pools, device_queues, command_buffer);
}

void Bolt::Vk_Util::Copy_Buffer_To_Image(VkDevice device, Command_Pools command_pools, Device_Queues device_queues, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(buffer, "Source buffer is a nullptr!");
    ASSERT(image, "Image is a nullptr!");
    ASSERT(width, "buffer width is 0");
    ASSERT(height, "buffer width is 0");

    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    Begin_Single_Time_Commands(device, command_pools, command_buffer);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

    vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    End_Single_Time_Commands(device, command_pools, device_queues, command_buffer);
}

void Bolt::Vk_Util::Create_Staging_Buffer(VkDevice device, GPU gpu, void* source, VkDeviceSize size, Buffer_Description& output)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(size, "buffer size is 0");
    ASSERT(source, "Data source is a nullptr!");
    ASSERT(gpu.physical_device, "Physical device is a nullptr!");
    ASSERT(output.handle == VK_NULL_HANDLE, "Buffer is already created!");
    ASSERT(output.memory == VK_NULL_HANDLE, "Buffer is already allocated!");

    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    Vk_Util::Create_Buffer(device, gpu, usage, properties, size, output);
    Vk_Util::Write_To_Buffer(device, output, size, source);
}

void Bolt::Vk_Util::Write_To_Buffer(VkDevice device, Buffer_Description buffer, VkDeviceSize buffer_size, void* data, u32 buffer_offset)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(buffer_size, "Buffer size is 0!");
    ASSERT(data, "Data is a nullptr!");
    ASSERT(buffer.handle != VK_NULL_HANDLE, "Buffer is already created!");
    ASSERT(buffer.memory != VK_NULL_HANDLE, "Buffer is already allocated!");

    void* maped_memory = nullptr;
    vkMapMemory(device, buffer.memory, 0, buffer_size, buffer_offset, &maped_memory);
    memcpy(maped_memory, data, buffer_size);
    vkUnmapMemory(device, buffer.memory);
}

void Bolt::Vk_Util::RCMD_Begin_Command_Buffer(VkCommandBuffer& cmd_buffer)
{
    ASSERT(cmd_buffer, "Command Buffer is a nullptr!");

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(cmd_buffer, &begin_info) != VK_SUCCESS)
        ERROR("Failed to begin recording command buffer!");
}

void Bolt::Vk_Util::RCMD_Set_View_Port(VkCommandBuffer cmd_buffer, Swapchain_Description& swapchain)
{
    ASSERT(cmd_buffer, "Command Buffer is a nullptr!");
    ASSERT(swapchain.handle, "Swapchain is a nullptr");

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = float(swapchain.extent.width);
    viewport.height = float(swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
}

void Bolt::Vk_Util::RCMD_Set_Scissors(VkCommandBuffer cmd_buffer, Swapchain_Description& swapchain)
{
    ASSERT(cmd_buffer, "Command Buffer is a nullptr!");
    ASSERT(swapchain.handle, "Swapchain is a nullptr");

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain.extent;
    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);
}


VkAttachmentDescription Bolt::Vk_Util::Depth_Clear_Attach(Formats format)
{
    VkAttachmentDescription attachment{};
    attachment.format = format.depth;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    return attachment;
}

VkAttachmentDescription Bolt::Vk_Util::Depth_Load_Attach(Formats format)
{
    VkAttachmentDescription attachment{};
    attachment.format = format.depth;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    return attachment;
}

VkAttachmentDescription Bolt::Vk_Util::Color_Clear_Attach(Formats format)
{
    VkAttachmentDescription attachment{};
    attachment.format = format.surface.format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    return attachment;
}

VkAttachmentDescription Bolt::Vk_Util::Color_Load_Attach(Formats format)
{
    VkAttachmentDescription attachment{};
    attachment.format = format.surface.format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    return attachment;
}

void Bolt::Vk_Util::RCMD_Begin_Render_Pass(VkCommandBuffer cmd_buffer, VkRenderPass render_pass, VkFramebuffer framebuffer, VkExtent2D extent, glm::vec3 clear_color)
{
    ASSERT(cmd_buffer, "Command Buffer is a nullptr!");
    ASSERT(render_pass, "Render Pass is a nullptr");
    ASSERT(framebuffer, "Framebuffer is a nullptr");
    ASSERT(extent.height != 0 && extent.width != 0, "Invalid extent!");

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = { { clear_color.r, clear_color.g, clear_color.b, 1.0f } };
    clear_values[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = framebuffer;
    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = extent;
    render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();;

    vkCmdBeginRenderPass(cmd_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

void Bolt::Vk_Init::Populate_Debug_Messenger_Create_Info(VkDebugUtilsMessengerCreateInfoEXT& create_info, PFN_vkDebugUtilsMessengerCallbackEXT user_callback)
{
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = user_callback;
    create_info.pUserData = nullptr;
}

VkResult Bolt::Vk_Init::Create_Debug_Utils_Messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* create_info_ptr, const VkAllocationCallbacks* allocator_ptr, VkDebugUtilsMessengerEXT* debug_messanger_ptr)
{
    ASSERT(instance, "Vulkan instance is a nullptr!");

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        return func(instance, create_info_ptr, allocator_ptr, debug_messanger_ptr);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

bool Bolt::Vk_Init::Check_Device_Extension_Support(VkPhysicalDevice device)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    for (auto& device_extension : device_extensions)
    {
        bool required_extension_available = false;
        for (const auto& extension : available_extensions)
            if (strcmp(device_extension, extension.extensionName) == 0)
            {
                required_extension_available = true;
                break;
            }

        if (!required_extension_available)
            return false;
    }

    return true;
}

Bolt::Queue_Family_Indices Bolt::Vk_Init::Find_Queue_Families(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    Queue_Family_Indices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    for (uint32_t i = 0; i < uint32_t(queue_families.size()); i++)
    {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.m_graphics_family = i;

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support)
            indices.m_present_family = i;

        if (indices.Is_Complete())
            break;
    }

    return indices;
}

Bolt::Swapchain_Support_Details Bolt::Vk_Init::Query_Swapchain_Support(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    Swapchain_Support_Details details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

    if (format_count > 0)
    {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

    if (present_mode_count > 0)
    {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}

void Bolt::Vk_Dest::Clear_Swapchain(VkDevice device, Swapchain_Description& input)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(input.handle, "Swapchain is a nullptr!");
    ASSERT(!input.images.empty(), "Swapchain images vector is empty!");
    ASSERT(!input.image_views.empty(), "Swapchain framebuffers vector is empty!");
    ASSERT(!input.framebuffers.empty(), "Swapchain framebuffers vector is empty!");

    vkDeviceWaitIdle(device);

    for (auto framebuffer : input.framebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);

    for (auto image_view : input.image_views)
        vkDestroyImageView(device, image_view, nullptr);

    vkDestroySwapchainKHR(device, input.handle, nullptr);

    input.extent = { 0,0 };
    input.framebuffers.clear();
    input.images.clear();
    input.image_views.clear();
    input.handle = VK_NULL_HANDLE;
}

void Bolt::Vk_Dest::Clear_Image(VkDevice device, Image_Description& input)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(input.view, "Image is a nullptr!");
    ASSERT(input.handle, "Image view is a nullptr!");
    ASSERT(input.memory, "Image memory is a nullptr!");

    vkDeviceWaitIdle(device);

    vkDestroyImageView(device, input.view, nullptr);
    vkDestroyImage(device, input.handle, nullptr);
    vkFreeMemory(device, input.memory, nullptr);

    input.view = VK_NULL_HANDLE;
    input.handle = VK_NULL_HANDLE;
    input.memory = VK_NULL_HANDLE;
}

void Bolt::Vk_Dest::Clear_Buffer(VkDevice device, Buffer_Description& input)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(input.handle, "Image view is a nullptr!");
    ASSERT(input.memory, "Image memory is a nullptr!");

    vkDeviceWaitIdle(device);

    vkDestroyBuffer(device, input.handle, nullptr);
    vkFreeMemory(device, input.memory, nullptr);

    input.handle = VK_NULL_HANDLE;
    input.memory = VK_NULL_HANDLE;
}

void Bolt::Vk_Dest::Clear_Shader_Modules(VkDevice device, Shader_Modules& input)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(input.frag, "Fragment shader is a nullptr!");
    ASSERT(input.vert, "Vertex shader is a nullptr!");

    vkDeviceWaitIdle(device);

    vkDestroyShaderModule(device, input.frag, nullptr);
    vkDestroyShaderModule(device, input.vert, nullptr);

    input.frag = VK_NULL_HANDLE;
    input.vert = VK_NULL_HANDLE;
}

void Bolt::Vk_Dest::Clear_Frame_Data(VkDevice device, Frame_Data& input)
{
    ASSERT(device, "Logical device is a nullptr!");
    ASSERT(input.sync.image_available_semaphore, "Image available semaphore is a nullptr!");
    ASSERT(input.sync.render_finished_semaphore, "Render finished semaphore is a nullptr!");
    ASSERT(input.sync.render_fence, "Render fence is a nullptr!");

    vkDeviceWaitIdle(device);

    vkDestroySemaphore(device, input.sync.image_available_semaphore, nullptr);
    vkDestroySemaphore(device, input.sync.render_finished_semaphore, nullptr);
    vkDestroyFence(device, input.sync.render_fence, nullptr);

    input = {};
}


void Bolt::Vk_Dest::Destroy_Debug_Utils_Messenger(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks* allocator_ptr)
{
    ASSERT(instance, "Vulkan instance is a nullptr!");

    if (!enable_validation_layers) return;

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, debug_messenger, allocator_ptr);
}