#include "Vk_Renderer.h"
#include "../Core/Assert.h"

#include <glm/gtc/matrix_inverse.hpp>

Bolt::Vk_Renderer::Vk_Renderer(GLFWwindow* window) : m_window(window)
{
	Init();
}

Bolt::Vk_Renderer::~Vk_Renderer()
{
	m_descriptors.Dest();
	Dest();
}

void Bolt::Vk_Renderer::Init()
{
	ASSERT(m_window, "Window is a nullptr!");
	ASSERT(glfwGetWindowUserPointer(m_window) == nullptr, "Window maybe already bound to another renderer!");

	//Finish seting up the glfw window.
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, On_Framebuffer_Resize_Callback);
	glfwGetFramebufferSize(m_window, &m_framebuffer_size.x, &m_framebuffer_size.y);

	//Initialize Vulkan it self.
	Vk_Init::Create_Instance(Debug_Callback, m_instance);
	Vk_Init::Create_Debug_Messanger(m_instance, Debug_Callback, m_debug_messenger);
	Vk_Init::Create_Window_Surface(m_instance, m_window, m_surface);
	Vk_Init::Pick_Physical_Device(m_instance, m_surface, m_gpu);
	Vk_Init::Create_Logical_Device(m_gpu, m_device);
	Vk_Init::Retrive_Device_Queues(m_device, m_gpu.queue_families, m_device_queues);
	Vk_Init::Create_Command_Pools(m_device, m_gpu.queue_families, m_command_pools);
	Vk_Init::Create_Texture_Sampler(m_device, m_gpu, m_sampler);
	Vk_Init::Create_Swapchain(m_device, m_gpu, m_surface, m_window, m_swapchain);
	Vk_Init::Create_Depth_Resources(m_device, m_gpu, m_swapchain, m_command_pools, m_device_queues, m_depth_image);
	Vk_Init::Create_Render_Pass(m_device, m_gpu.formats, m_dummy_render_pass);
	Vk_Init::Create_Framebuffers(m_device, m_dummy_render_pass, m_depth_image, m_swapchain);
	
	m_descriptors.Init(m_device, MAX_FRAMES_IN_FLIGHT);
	Create_Graphics_Pipeline_Layout({ m_descriptors.Get_Layout(Descriptor_Types::Scene), m_descriptors.Get_Layout(Descriptor_Types::Material) });
	Init_Per_Frame_Data();
	Create_Default_Resources();
}

void Bolt::Vk_Renderer::Init_Per_Frame_Data()
{
	std::vector<VkCommandBuffer> cmd_buffers;
	Vk_Init::Create_Command_Buffers(m_device, m_command_pools, MAX_FRAMES_IN_FLIGHT, cmd_buffers);
	for (uint32_t i = 0; i < m_frame_data.size(); i++)
		m_frame_data[i].cmd_buffer = cmd_buffers[i];

	for (auto& frame : m_frame_data)
		Vk_Init::Create_Frame_Syncronization_Objects(m_device, frame.sync);
}

void Bolt::Vk_Renderer::Dest()
{
	vkDeviceWaitIdle(m_device);

	Destory_Default_Resources();

	vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);

	Vk_Dest::Clear_Image(m_device, m_depth_image);
	Vk_Dest::Clear_Swapchain(m_device, m_swapchain);

	for (Frame_Data& frame : m_frame_data)
		Vk_Dest::Clear_Frame_Data(m_device, frame);

	vkDestroySampler(m_device, m_sampler, nullptr);

	vkDestroyRenderPass(m_device, m_dummy_render_pass, nullptr);

	vkDestroyCommandPool(m_device, m_command_pools.reset, nullptr);
	vkDestroyCommandPool(m_device, m_command_pools.transient, nullptr);

	vkDestroyDevice(m_device, nullptr);

	Vk_Dest::Destroy_Debug_Utils_Messenger(m_instance, m_debug_messenger, nullptr);

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);

	glfwDestroyWindow(m_window);
}


void Bolt::Vk_Renderer::Draw_Frame(Render_Submissions& submissions, glm::vec3 clear_color)
{
	Frame_Data& frame = m_frame_data[m_current_frame];

	vkWaitForFences(m_device, 1, &frame.sync.render_fence, VK_TRUE, UINT64_MAX);

	if (Acquire_Image_From_Swapchain(frame) == false)
		return;
	
	m_draw_calls = 0;

	//Only reset the fence if we are submitting work.
	vkResetFences(m_device, 1, &frame.sync.render_fence);

	//Alias for cleaner code.
	VkCommandBuffer& cmd_buffer = frame.cmd_buffer;
	VkFramebuffer& framebuffer = m_swapchain.framebuffers[frame.swapchain_image_index];
	vkResetCommandBuffer(cmd_buffer, 0);

	// ---- Rendering Work Starts Here ---- //
	Vk_Util::RCMD_Begin_Command_Buffer(cmd_buffer);
	
	Vk_Util::RCMD_Set_View_Port(cmd_buffer, m_swapchain);
	Vk_Util::RCMD_Set_Scissors(cmd_buffer, m_swapchain);

	if (submissions.Activate_First_Pass()) 
	{
		Beging_First_Pass(submissions, frame, cmd_buffer, framebuffer, clear_color);	

		while (!submissions.Activate_Next_Pass())
			Beging_Secondary_Pass(submissions, frame, cmd_buffer, framebuffer, clear_color);
	}

	if (vkEndCommandBuffer(cmd_buffer) != VK_SUCCESS)
		ERROR("failed to record command buffer!");
	// ----  Rendering Work Ends Here  ---- //

	Submit_Graphics_Queue(frame);
	Present_To_Surface(frame);

	m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Bolt::Vk_Renderer::Beging_Secondary_Pass(Bolt::Render_Submissions& submissions, Bolt::Frame_Data& frame, VkCommandBuffer& cmd_buffer, VkFramebuffer& framebuffer, glm::vec3& clear_color)
{
	Pass_Submissions& active_pass = submissions.Active_Pass();

	Scene_Descriptor* scene_descriptor = active_pass.info.scene_descriptor;
	if (scene_descriptor)
	{
		scene_descriptor->Update(m_current_frame);
		VkDescriptorSet descriptor = scene_descriptor->pfd[m_current_frame].descriptor_set;
		if (descriptor != VK_NULL_HANDLE)
			vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &descriptor, 0, nullptr);
	}

	Draw_Render_Pass(active_pass, cmd_buffer, framebuffer, clear_color);
}

void Bolt::Vk_Renderer::Beging_First_Pass(Bolt::Render_Submissions& submissions, Bolt::Frame_Data& frame, VkCommandBuffer& cmd_buffer, VkFramebuffer& framebuffer, glm::vec3& clear_color)
{
	Pass_Submissions& active_pass = submissions.Active_Pass();

	Scene_Descriptor* scene_descriptor = active_pass.info.scene_descriptor;
	if (scene_descriptor == nullptr)
		ERROR("First pass has to have a valid scene descriptor!");

	scene_descriptor->Update(m_current_frame);

	VkDescriptorSet descriptor = scene_descriptor->pfd[m_current_frame].descriptor_set;
	if (descriptor == VK_NULL_HANDLE)
		ERROR("First pass has to have a valid scene descriptor!");

	vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0, 1, &descriptor, 0, nullptr);

	Draw_Render_Pass(active_pass, cmd_buffer, framebuffer, clear_color);
}

void Bolt::Vk_Renderer::Draw_Render_Pass(Bolt::Pass_Submissions& active_pass, VkCommandBuffer& cmd_buffer, VkFramebuffer& framebuffer, const glm::vec3& clear_color)
{
	VkRenderPass render_pass = active_pass.info.render_pass->renderpass ? active_pass.info.render_pass->renderpass : m_dummy_render_pass;

	active_pass.Sort_Transparent();

	Vk_Util::RCMD_Begin_Render_Pass(cmd_buffer, render_pass, framebuffer, m_swapchain.extent, clear_color);
	Draw_Submissions(cmd_buffer, active_pass);
	vkCmdEndRenderPass(cmd_buffer);
}

void Bolt::Vk_Renderer::Draw_Submissions(VkCommandBuffer& cmd_buffer, const Pass_Submissions& submissions)
{	
	Draw_Render_Objects(&submissions.models_3D, cmd_buffer);
	Draw_Render_Objects(&submissions.transparent_models_3D, cmd_buffer);
	Draw_Render_Objects(&submissions.billboards, cmd_buffer);
}

void Bolt::Vk_Renderer::Draw_Render_Objects(const std::vector<Render_Object_3D_Model>* render_set, VkCommandBuffer& cmd_buffer)
{
	Material* active_material = nullptr;
	Mesh* active_mesh = nullptr;

	for (const Render_Object_3D_Model& render_obj : *render_set)
	{
		ASSERT(render_obj.mesh, "Mesh is a nullptr!");
		ASSERT(render_obj.mesh->m_vertex_buffer.handle, "Mesh vertex buffer handle is a nullptr!");
		ASSERT(render_obj.mesh->m_vertex_buffer.memory, "Mesh vertex buffer memory is a nullptr!");
		ASSERT(render_obj.mesh->m_index_buffer.handle, "Mesh index buffer handle is a nullptr!");
		ASSERT(render_obj.mesh->m_index_buffer.memory, "Mesh index buffer memory is a nullptr!");
		ASSERT(render_obj.mesh->m_index_count, "Index buffer count is 0!");

		//Bind pipeline if different.
		if (render_obj.material != active_material)
		{
			active_material = render_obj.material;
			vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, active_material->m_pipeline);

			//Bind per material descriptor.
			vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1, &active_material->m_descriptor_set, 0, nullptr);
		}

		//Bind vertex/index buffers if different.
		if (render_obj.mesh != active_mesh)
		{
			active_mesh = render_obj.mesh;
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &active_mesh->m_vertex_buffer.handle, offsets);
			vkCmdBindIndexBuffer(cmd_buffer, active_mesh->m_index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);
		}

		Push_Constant pc = Create_Push_Constant_3D_Model(render_obj.transform);
		vkCmdPushConstants(cmd_buffer, m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Bolt::Push_Constant), &pc);

		vkCmdDrawIndexed(cmd_buffer, active_mesh->m_index_count, 1, 0, 0, 0);
		m_draw_calls++;
	}
}

void Bolt::Vk_Renderer::Draw_Render_Objects(const std::vector<Render_Object_Billboard>* render_set, VkCommandBuffer& cmd_buffer)
{
	Material* active_material = nullptr;
	Mesh* active_mesh = nullptr;

	for (const Render_Object_Billboard& render_obj : *render_set)
	{
		ASSERT(render_obj.mesh, "Mesh is a nullptr!");
		ASSERT(render_obj.mesh->m_vertex_buffer.handle, "Mesh vertex buffer handle is a nullptr!");
		ASSERT(render_obj.mesh->m_vertex_buffer.memory, "Mesh vertex buffer memory is a nullptr!");
		ASSERT(render_obj.mesh->m_index_buffer.handle, "Mesh index buffer handle is a nullptr!");
		ASSERT(render_obj.mesh->m_index_buffer.memory, "Mesh index buffer memory is a nullptr!");
		ASSERT(render_obj.mesh->m_index_count, "Index buffer count is 0!");

		//Bind pipeline if different.
		if (render_obj.material != active_material)
		{
			active_material = render_obj.material;
			ASSERT(active_material, "Active material is a nullptr");
			vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, active_material->m_pipeline);

			//Bind per material descriptor.
			vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 1, 1, &active_material->m_descriptor_set, 0, nullptr);
		}

		//Bind vertex/index buffers if different.
		if (render_obj.mesh != active_mesh)
		{
			active_mesh = render_obj.mesh;
			ASSERT(active_mesh, "Active mesh is a nullptr");
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &active_mesh->m_vertex_buffer.handle, offsets);
			vkCmdBindIndexBuffer(cmd_buffer, active_mesh->m_index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);
		}

		Push_Constant pc = Create_Push_Constant_Billboard(render_obj.position, render_obj.scale);
		vkCmdPushConstants(cmd_buffer, m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Bolt::Push_Constant), &pc);

		vkCmdDrawIndexed(cmd_buffer, active_mesh->m_index_count, 1, 0, 0, 0);
		m_draw_calls++;
	}
}

void Bolt::Vk_Renderer::Load_Texture(const char* file_path, Texture& output_texture)
{
	ASSERT(file_path, "File path is a nullptr!");
	ASSERT(output_texture.m_image.handle == VK_NULL_HANDLE, "Texture handle already created!");
	ASSERT(output_texture.m_image.memory == VK_NULL_HANDLE, "Texture memory already allocated!");
	ASSERT(output_texture.m_image.view == VK_NULL_HANDLE, "Texture view already created!");

	//Load the pixel data into host memory.
	Image_Data img_data = Texture::Load_Image(file_path);
	ASSERT(img_data.pixels, "Failed to load image '" << file_path << "'\n");

	output_texture.has_transparensy = img_data.Is_Transparent();

	//Upload the pixel data into a GPU staging buffer.
	Buffer_Description staging_buffer;
	Vk_Util::Create_Staging_Buffer(m_device, m_gpu, img_data.pixels, img_data.width * img_data.height * 4, staging_buffer);
	img_data.Free();

	//Create a GPU image handle.
	Image_Create_Info image_create_info;
	image_create_info.width = img_data.width;
	image_create_info.height = img_data.height;
	image_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	image_create_info.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	Vk_Util::Create_Image(m_device, m_gpu, image_create_info, output_texture.m_image);

	//Transition the image into the right format.
	Image_Layout_Transition_Info transition_info;
	transition_info.image = output_texture.m_image.handle;
	transition_info.format = VK_FORMAT_R8G8B8A8_SRGB;
	transition_info.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	transition_info.new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	Vk_Util::Transition_Image_Layout(m_device, m_command_pools, m_device_queues, transition_info);

	//Copy the pixel data from the staging buffer into the prepared image.
	Vk_Util::Copy_Buffer_To_Image(m_device, m_command_pools, m_device_queues, staging_buffer.handle, output_texture.m_image.handle, img_data.width, img_data.height);

	//Finaly transition the image layout, to be optimal for the shader to read.
	transition_info.old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	transition_info.new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	Vk_Util::Transition_Image_Layout(m_device, m_command_pools, m_device_queues, transition_info);

	Vk_Dest::Clear_Buffer(m_device, staging_buffer);

	Vk_Util::Create_Image_View(m_device, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, output_texture.m_image);
}


void Bolt::Vk_Renderer::Destroy(Texture& texture)
{
	Vk_Dest::Clear_Image(m_device, texture.m_image);
}


void Bolt::Vk_Renderer::Load_Shader(const char* frag_file_path, const char* vert_file_path, Raw_Shader& output_shader)
{
	ASSERT(frag_file_path, "File path is a nullptr!");
	ASSERT(vert_file_path, "File path is a nullptr!");
	ASSERT(output_shader.m_shader_modules.frag == VK_NULL_HANDLE, "Fragment shader already created!");
	ASSERT(output_shader.m_shader_modules.vert == VK_NULL_HANDLE, "Fragment shader already created!");

	auto frag_byte_code = Raw_Shader::Read_Byte_Code(frag_file_path);
	auto vert_byte_code = Raw_Shader::Read_Byte_Code(vert_file_path);

	output_shader.m_shader_modules = { Create_Shader_Module(frag_byte_code), Create_Shader_Module(vert_byte_code) };
}


void Bolt::Vk_Renderer::Destroy(Raw_Shader& shader)
{
	Vk_Dest::Clear_Shader_Modules(m_device, shader.m_shader_modules);
}


void Bolt::Vk_Renderer::Create_Material(const Material_Properties& properties, const Texture& texture, Material& output_material, const Shader shader)
{
	ASSERT(output_material.m_pipeline == VK_NULL_HANDLE, "Pipeline already created!");
	ASSERT(output_material.m_material_buffer.handle == VK_NULL_HANDLE, "Material buffer already created!");
	ASSERT(output_material.m_material_buffer.memory == VK_NULL_HANDLE, "Material buffer memory already allocated!");

	m_descriptors.Allocate(Descriptor_Types::Material, output_material.m_descriptor_set);
	
	VkDescriptorImageInfo image_info{};
	image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	image_info.imageView = texture.m_image.view;
	image_info.sampler = m_sampler;
	
	std::array<VkWriteDescriptorSet, 2> writes{};
	{
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].dstSet = output_material.m_descriptor_set;
		writes[0].dstBinding = 0;
		writes[0].dstArrayElement = 0;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[0].descriptorCount = 1;
		writes[0].pImageInfo = &image_info;
	}
	
	VkMemoryPropertyFlags memory_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	VkDeviceSize buffer_size = sizeof(Material_Properties);

	Vk_Util::Create_Buffer(m_device, m_gpu, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, memory_properties, buffer_size, output_material.m_material_buffer);

	void* maped_memory = nullptr;
	vkMapMemory(m_device, output_material.m_material_buffer.memory, 0, buffer_size, 0, &maped_memory);
	memcpy(maped_memory, &properties, sizeof(properties));
	vkUnmapMemory(m_device, output_material.m_material_buffer.memory);

	VkDescriptorBufferInfo buffer_info{};
	buffer_info.buffer = output_material.m_material_buffer.handle;
	buffer_info.offset = 0;
	buffer_info.range = buffer_size;

	{
		writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[1].dstSet = output_material.m_descriptor_set;
		writes[1].dstBinding = 1;
		writes[1].dstArrayElement = 0;
		writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writes[1].descriptorCount = 1;
		writes[1].pBufferInfo = &buffer_info;
	}

	vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	Raw_Shader* raw_shader = Extract_Raw_Shader(shader);
	Vk_Init::Create_Graphics_Pipeline(m_device, m_pipeline_layout, m_dummy_render_pass, raw_shader->m_shader_modules, output_material.m_pipeline);

	output_material.has_transparency = texture.has_transparensy || properties.transparensy < 1;
}


void Bolt::Vk_Renderer::Destroy(Material& material)
{
	ASSERT(material.m_pipeline, "Pipeline not created!");

	vkDeviceWaitIdle(m_device);
	Vk_Dest::Clear_Buffer(m_device, material.m_material_buffer);
	vkDestroyPipeline(m_device, material.m_pipeline, nullptr);
}


void Bolt::Vk_Renderer::Load_Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, Mesh& output_mesh)
{
	ASSERT(output_mesh.m_vertex_buffer.handle == VK_NULL_HANDLE, "Vertex buffer already created!");
	ASSERT(output_mesh.m_vertex_buffer.memory == VK_NULL_HANDLE, "Vertex memory already allocated!");
	ASSERT(output_mesh.m_index_buffer.handle == VK_NULL_HANDLE, "Index buffer already created!");
	ASSERT(output_mesh.m_index_buffer.memory == VK_NULL_HANDLE, "Index memory already allocated!");
	ASSERT(!indices.empty(), "Indices vector is empty!");
	ASSERT(!vertices.empty(), "Vertices vector is empty!");

	//Upload the vertex and index buffers into GPU memory and store handles in the mesh members.
	Upload_Buffer_To_GPU((void*)vertices.data(), sizeof(vertices[0]) * vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, output_mesh.m_vertex_buffer);
	Upload_Buffer_To_GPU((void*)indices.data(), sizeof(indices[0]) * indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, output_mesh.m_index_buffer);

	output_mesh.m_index_count = (uint32_t)indices.size();
}


void Bolt::Vk_Renderer::Destroy(Mesh& mesh)
{
	Vk_Dest::Clear_Buffer(m_device, mesh.m_index_buffer);
	Vk_Dest::Clear_Buffer(m_device, mesh.m_vertex_buffer);
}

void Bolt::Vk_Renderer::Create_Render_Pass(Render_Pass& output, Clear_Method color_clear_method, Clear_Method depth_clear_method)
{
	ASSERT(output.renderpass == VK_NULL_HANDLE, "Render pass is already created!");

	Attachments color_attachments;
	switch (color_clear_method)
	{
	case Bolt::Clear_Method::Load:
		color_attachments.push_back(Vk_Util::Color_Load_Attach(m_gpu.formats));
		break;

	case Bolt::Clear_Method::Clear:
		color_attachments.push_back(Vk_Util::Color_Clear_Attach(m_gpu.formats));
		break;

	default:
		ERROR("Invalid clear method");
		break;
	}

	std::optional<VkAttachmentDescription> depth_attachment;
	switch (depth_clear_method)
	{
	case Bolt::Clear_Method::Load:
		depth_attachment =Vk_Util::Depth_Load_Attach(m_gpu.formats);
		break;

	case Bolt::Clear_Method::Clear:
		depth_attachment = Vk_Util::Depth_Clear_Attach(m_gpu.formats);
		break;

	default:
		ERROR("Invalid clear method");
		break;
	}

	Vk_Init::Create_Render_Pass(m_device, color_attachments, depth_attachment, output.renderpass);
}

void Bolt::Vk_Renderer::Destroy(Render_Pass& render_pass)
{
	ASSERT(render_pass.renderpass != VK_NULL_HANDLE, "Render pass is a nullptr");
	vkDeviceWaitIdle(m_device);
	vkDestroyRenderPass(m_device, render_pass.renderpass, nullptr);
}

void Bolt::Vk_Renderer::Create_Scene_Descriptor(Scene_Descriptor& output)
{
	output.device = m_device;
	VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (auto& pfd : output.pfd)
	{
		Vk_Util::Create_Buffer(m_device, m_gpu, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, properties, sizeof(Scene_Uniform_Buffer_Object), pfd.buffer);
		m_descriptors.Allocate(Descriptor_Types::Scene, pfd.descriptor_set);
	}

	output.Write_Default_Values_Into_All_Descriptors();
}

void Bolt::Vk_Renderer::Destroy(Scene_Descriptor& scene_descriptor)
{
	for (auto& pfd : scene_descriptor.pfd)
		Vk_Dest::Clear_Buffer(m_device, pfd.buffer);
}

void Bolt::Vk_Renderer::Create_Default_Resources()
{
	//m_default_circle_shader

	Load_Shader("_Shaders/frag.spv", "_Shaders/vert.spv", m_default_shader);
	Load_Shader("_Shaders/diffuse/frag.spv", "_Shaders/diffuse/vert.spv", m_default_diffuse_shader);
	Load_Shader("_Shaders/billboard/frag.spv", "_Shaders/billboard/vert.spv", m_default_billboard_shader);



	Load_Shader("_Shaders/circle/frag.spv", "_Shaders/circle/vert.spv", m_default_circle_shader);
}


void Bolt::Vk_Renderer::Destory_Default_Resources()
{
	Destroy(m_default_shader);
	Destroy(m_default_diffuse_shader);
	Destroy(m_default_billboard_shader);
	Destroy(m_default_circle_shader);
}


Bolt::Raw_Shader* Bolt::Vk_Renderer::Extract_Raw_Shader(const Shader shader)
{
	if (shader.custom) return shader.custom;

	switch (shader._default)
	{
	case Shader::Defaults::Specular:
		return &m_default_shader;

	case Shader::Defaults::Billboard:
		return &m_default_billboard_shader;

	case Shader::Defaults::Diffuse:
		return &m_default_diffuse_shader;

	case Shader::Defaults::Circle:
		return &m_default_circle_shader;

	default:
		ERROR("Unhandeled shader default!");
	}
}


Bolt::Push_Constant Bolt::Vk_Renderer::Create_Push_Constant_3D_Model(const glm::mat4& model_transform)
{
	Push_Constant pc;

	pc.model_matrix = model_transform;
	//pc.model_matrix[0] = glm::vec4(glm::vec3(1), 1);
	
	glm::mat3 normal_matrix = glm::mat3(1);
	normal_matrix = glm::inverseTranspose(model_transform);
	pc.normal_matrix = normal_matrix;

	return pc;
}


Bolt::Push_Constant Bolt::Vk_Renderer::Create_Push_Constant_Billboard(const glm::vec3& position, const glm::vec2& scale)
{
	Push_Constant pc;
	pc.model_matrix[0] = glm::vec4(position, 1);
	pc.model_matrix[1] = glm::vec4(scale, 1, 1);

	glm::mat3 normal_matrix = glm::mat3(1);
	//normal_matrix = glm::inverseTranspose(model_transform);
	pc.normal_matrix = normal_matrix;

	return pc;
}


void Bolt::Vk_Renderer::Upload_Buffer_To_GPU(void* source, VkDeviceSize size, VkBufferUsageFlags usage, Buffer_Description& output)
{
	Buffer_Description staging_buffer;

	Vk_Util::Create_Buffer(m_device, m_gpu, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, size, output);
	Vk_Util::Create_Staging_Buffer(m_device, m_gpu, source, size, staging_buffer);
	Vk_Util::Copy_Buffer(m_device, m_command_pools, m_device_queues, staging_buffer.handle, output.handle, size);
	Vk_Dest::Clear_Buffer(m_device, staging_buffer);
}


VkShaderModule Bolt::Vk_Renderer::Create_Shader_Module(const std::vector<char>& code)
{
	ASSERT(!code.empty(), "Shader code is empty");

	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shader_module;
	if (vkCreateShaderModule(m_device, &create_info, nullptr, &shader_module) != VK_SUCCESS)
		ERROR("failed to create shader module!");

	return shader_module;
}


void Bolt::Vk_Renderer::Create_Graphics_Pipeline_Layout(std::vector<VkDescriptorSetLayout> layouts)
{
	VkPushConstantRange push_constant;
	push_constant.offset = 0;
	push_constant.size = sizeof(Bolt::Push_Constant);
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	Vk_Init::Create_Graphics_Pipline_Layout(m_device, layouts, m_pipeline_layout, push_constant);
}


void Bolt::Vk_Renderer::Recreate_Swapchain()
{
	Vk_Dest::Clear_Image(m_device, m_depth_image);
	Vk_Dest::Clear_Swapchain(m_device, m_swapchain);

	Vk_Init::Create_Swapchain(m_device, m_gpu, m_surface, m_window, m_swapchain);
	Vk_Init::Create_Depth_Resources(m_device, m_gpu, m_swapchain, m_command_pools, m_device_queues, m_depth_image);
	Vk_Init::Create_Framebuffers(m_device, m_dummy_render_pass, m_depth_image, m_swapchain);
}

bool Bolt::Vk_Renderer::Acquire_Image_From_Swapchain(Frame_Data& frame)
{
	if (m_framebuffer_size.x <= 0 || m_framebuffer_size.y <= 0)
		return false;

	if (m_framebuffer_resized)
	{
		Recreate_Swapchain();
		m_framebuffer_resized = false;
		return false;
	}

	VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain.handle, 1, frame.sync.image_available_semaphore, VK_NULL_HANDLE, &frame.swapchain_image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		Recreate_Swapchain();
		return false;
	}

	if (result == VK_SUCCESS)
		return true;

	ERROR("Failed to acquire swapchain image!");
}


void Bolt::Vk_Renderer::Submit_Graphics_Queue(const Frame_Data& frame)
{
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &frame.sync.image_available_semaphore;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &frame.cmd_buffer;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &frame.sync.render_finished_semaphore;

	if (vkQueueSubmit(m_device_queues.graphics, 1, &submit_info, frame.sync.render_fence) != VK_SUCCESS)
		ERROR("failed to submit draw command buffer!");
}

void Bolt::Vk_Renderer::Present_To_Surface(const Frame_Data& frame)
{
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &frame.sync.render_finished_semaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &m_swapchain.handle;
	present_info.pImageIndices = &frame.swapchain_image_index;
	present_info.pResults = nullptr; // Optional

	VkResult result = vkQueuePresentKHR(m_device_queues.present, &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		Recreate_Swapchain();

	else if (result != VK_SUCCESS)
		ERROR("failed to present swap chain image!");
}

VKAPI_ATTR VkBool32 VKAPI_CALL Bolt::Vk_Renderer::Debug_Callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data_ptr, void* user_data_ptr)
{
	if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		std::cerr << callback_data_ptr->pMessage << "\n\n";

	return VK_FALSE;
}

void Bolt::Vk_Renderer::On_Framebuffer_Resize_Callback(GLFWwindow* window, int width, int height)
{
	Vk_Renderer* instance = (Vk_Renderer*)(glfwGetWindowUserPointer(window));
	instance->m_framebuffer_resized = true;
	instance->m_framebuffer_size.x = width;
	instance->m_framebuffer_size.y = height;
}

GLFWwindow* Bolt::Create_GLFW_Window(glm::u32vec2 dimensions, const char* title)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	return glfwCreateWindow(dimensions.x, dimensions.y, title, nullptr, nullptr);
}