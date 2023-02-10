#pragma once

#include "../Core/Bolt_Types.h"
#include "../Core/Assert.h"

#include "Vk_Render_Objects.h"

#include <list>

namespace Bolt
{
	struct Pass_Submissions
	{
		Pass_Submissions(Render_Pass_Info _info) : info(_info) {}

		Render_Pass_Info info;

		std::vector<Render_Object_3D_Model> models_3D;
		std::vector<Render_Object_Billboard> billboards;

		void Clear()
		{
			models_3D.clear();
			billboards.clear();
		}
	};


	class Render_Submissions final
	{
		typedef std::pair<Pass_Submissions, std::vector<Pass_Submissions>> Main_Pass;

	public:
		Render_Submissions();

		//Submit calls.
	public:
		void Submit_3D_Model(Mesh* mesh, Material* material, const glm::mat4* matrix);
		void Submit_Billboard(Mesh* mesh, Material* material, glm::vec3 position, glm::vec2 scale);
		
		//Internal list navigation calls.
	public:
		bool Activate_Next_Pass();
		bool Activate_Next_Main_Pass();
		bool Activate_First_Pass();
		bool Activate_Last_Main_Pass();
		void Set_Active_Subpass(u32 index);
		u32 Active_Subpass_Index() const;

		//Pass creation calls.
	public:
		void Push_New_Main_Pass(Render_Pass_Info info);
		void Push_New_Subpass(Render_Pass_Info info);

	public:
		Pass_Submissions& Active_Pass();
		void Clear();
		u32 Main_Pass_Count();
		u32 Active_Main_Pass_Subpass_Count();

	private:
		bool Seek_To_Next_Main_Pass();
		Main_Pass& Get_Active_Main_Pass();

	private:
		std::list<Main_Pass> m_passes;
		Pass_Submissions* m_active_pass = nullptr;
		u32 m_active_subpass_index = 0;

		std::list<Main_Pass>::iterator m_mainp_iter{};
	};
}