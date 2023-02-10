#pragma once

#include "Renderer_Resource_Factory.h"
#include "Render_Submissions.h"

#include "Vk_Descriptor_Allocator.h"
#include "Vk_Assist.h"

namespace Bolt
{
	GLFWwindow* Create_GLFW_Window(glm::u32vec2 dimensions, const char* title);

	class Vk_Renderer : public Resource_Factory
	{
	public:
		Vk_Renderer(GLFWwindow* window);
		~Vk_Renderer();
		
		void Draw_Frame(Render_Submissions& submissions, glm::vec3 clear_color = glm::vec3(0));

		VkExtent2D* Swapchain_Extent() { return &m_swapchain.extent; }

		//External resource Creation functions.
	public: 
		void Load_Texture(const char* file_path, Texture& output_texture) override;
		void Destroy(Texture& texture) override;

		void Load_Shader(const char* frag_file_path, const char* vert_file_path, Raw_Shader& output_shader) override;
		void Destroy(Raw_Shader& shader) override;

		void Create_Material(const Material_Properties& properties, const Texture& texture, Material& output_material, const Shader shader) override;
		void Destroy(Material& material) override;

		void Load_Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Mesh& output_mesh) override;
		void Destroy(Mesh& mesh) override;

		void Create_Render_Pass(Render_Pass& output, Clear_Method color_clear_method, Clear_Method depth_clear_method) override;
		void Destroy(Render_Pass& render_pass) override;

		void Create_Scene_Descriptor(Scene_Descriptor& output) override;
		void Destroy(Scene_Descriptor& scene_descriptor) override;

	private:
		void Create_Default_Resources();
		void Destory_Default_Resources();

	private:
		void Init();
		void Init_Per_Frame_Data();

		void Dest();

		void Upload_Buffer_To_GPU(void* source, VkDeviceSize size, VkBufferUsageFlags usage, Buffer_Description& output);

		VkShaderModule Create_Shader_Module(const std::vector<char>& code);

		void Create_Graphics_Pipeline_Layout(std::vector<VkDescriptorSetLayout> layouts);

		void Recreate_Swapchain();
		
		Raw_Shader* Extract_Raw_Shader(const Shader shader);
	
		//Draw frame utility functions.
		bool Acquire_Image_From_Swapchain(Frame_Data& frame);
		void Submit_Graphics_Queue(const Frame_Data& frame);
		void Present_To_Surface(const Frame_Data& frame);
		void Beging_Secondary_Pass(Bolt::Render_Submissions& submissions, Bolt::Frame_Data& frame, VkCommandBuffer& cmd_buffer, VkFramebuffer& framebuffer, glm::vec3& clear_color);
		void Beging_First_Pass(Bolt::Render_Submissions& submissions, Bolt::Frame_Data& frame, VkCommandBuffer& cmd_buffer, VkFramebuffer& framebuffer, glm::vec3& clear_color);
		void Draw_Render_Pass(Bolt::Pass_Submissions& active_pass, VkCommandBuffer& cmd_buffer, VkFramebuffer& framebuffer, const glm::vec3& clear_color);

		void Draw_Submissions(VkCommandBuffer& cmd_buffer, const Pass_Submissions& submissions);
		void Draw_Model_3D(const std::vector<Render_Object_3D_Model>* render_set, VkCommandBuffer& cmd_buffer);
		void Draw_Billboard(const std::vector<Render_Object_Billboard>* render_set, VkCommandBuffer& cmd_buffer);

		Push_Constant Create_Push_Constant_3D_Model(const glm::mat4& model_transform);
		Push_Constant Create_Push_Constant_Billboard(const glm::vec3& position, const glm::vec2& scale);

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL Debug_Callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data_ptr, void* user_data_ptr);
		static void On_Framebuffer_Resize_Callback(GLFWwindow* window, int width, int height);

	private:
		//glfw handel
		GLFWwindow* m_window;

		//Raw Vulkan handels
		VkInstance m_instance = VK_NULL_HANDLE;
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_debug_messenger = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VkRenderPass m_dummy_render_pass = VK_NULL_HANDLE;
		VkSampler m_sampler = VK_NULL_HANDLE;
		VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;

		//Wrapper objects for vulkan handles.
		GPU m_gpu;
		Device_Queues m_device_queues;
		Command_Pools m_command_pools;
		Swapchain_Description m_swapchain;
		Image_Description m_depth_image;
		
		std::array<Frame_Data, MAX_FRAMES_IN_FLIGHT> m_frame_data;

		u32 m_current_frame = 0;
		bool m_framebuffer_resized = false;
		glm::ivec2 m_framebuffer_size;

		Vk_Descriptor_Allocator m_descriptors;

		//default resources
		Raw_Shader m_default_shader;
		Raw_Shader m_default_billboard_shader;
	};
}