#include "Render_Submissions.h"

Bolt::Render_Submissions::Render_Submissions()
{
	m_mainp_iter = m_passes.begin();
}

void Bolt::Render_Submissions::Submit_3D_Model(Mesh* mesh, Material* material, const glm::mat4* matrix)
{
	if (material->Has_Transparensy())
		Active_Pass().transparent_models_3D.emplace_back(mesh, material, matrix);
	else
		Active_Pass().models_3D.emplace_back(mesh, material, matrix);
}

void Bolt::Render_Submissions::Submit_Billboard(Mesh* mesh, Material* material, glm::vec3 position, glm::vec2 scale)
{
	Active_Pass().billboards.emplace_back(mesh, material, position, scale);
}

bool Bolt::Render_Submissions::Activate_Next_Pass()
{
	m_active_subpass_index++;
	auto& [main_pass, subpasses] = Get_Active_Main_Pass();
	if (m_active_subpass_index > subpasses.size())
		return Activate_Next_Main_Pass();

	m_active_pass = &subpasses[m_active_subpass_index - 1];
	return false;
}

bool Bolt::Render_Submissions::Activate_Next_Main_Pass()
{
	return Seek_To_Next_Main_Pass();
}

bool Bolt::Render_Submissions::Activate_First_Pass()
{
	if (m_passes.empty()) return false;

	m_active_subpass_index = 0;
	m_mainp_iter = m_passes.begin();
	m_active_pass = &(*m_mainp_iter).first;
	return true;
}

bool Bolt::Render_Submissions::Activate_Last_Main_Pass()
{
	if (m_passes.empty()) return false;

	m_active_subpass_index = 0;
	m_mainp_iter = m_passes.end();
	std::advance(m_mainp_iter, -1);
	m_active_pass = &(*m_mainp_iter).first;

	return true;
}

void Bolt::Render_Submissions::Set_Active_Subpass(u32 index)
{
	auto& [main_pass, subpasses] = Get_Active_Main_Pass();

	m_active_subpass_index = index;
	if (m_active_subpass_index > subpasses.size())
		m_active_subpass_index = 0;
	
	if (m_active_subpass_index == 0)
	{
		m_active_pass = &main_pass;
		return;
	}

	m_active_pass = &subpasses[m_active_subpass_index - 1];
}

Bolt::u32 Bolt::Render_Submissions::Get_Active_Subpass_Index() const
{
	return m_active_subpass_index;
}

void Bolt::Render_Submissions::Push_New_Main_Pass(Render_Pass_Info info)
{
	m_passes.emplace_back(std::make_pair(Pass_Submissions(info), std::vector<Pass_Submissions>{}));
}

void Bolt::Render_Submissions::Push_New_Subpass(Render_Pass_Info info)
{
	Get_Active_Main_Pass().second.emplace_back(Pass_Submissions(info));
	
	m_active_subpass_index = 0;
	m_active_pass = &Get_Active_Main_Pass().first;
}

inline Bolt::Pass_Submissions& Bolt::Render_Submissions::Active_Pass()
{
	ASSERT(m_active_pass, "Acitve pass is not set!");
	return *m_active_pass;
}

void Bolt::Render_Submissions::Clear()
{
	for (auto& [main_pass, subpasses] : m_passes)
	{
		main_pass.Clear();
		for (auto& subpass : subpasses)
			subpass.Clear();
	}
}

Bolt::u32 Bolt::Render_Submissions::Main_Pass_Count()
{
	return u32(m_passes.size());
}

Bolt::u32 Bolt::Render_Submissions::Active_Main_Pass_Subpass_Count()
{
	return u32((*m_mainp_iter).second.size());
}

bool Bolt::Render_Submissions::Seek_To_Next_Main_Pass()
{
	ASSERT(!m_passes.empty(), "Passes list is empty!");
	
	m_active_subpass_index = 0;
	std::advance(m_mainp_iter, 1);

	if(m_mainp_iter == m_passes.end())
	{
		m_mainp_iter = m_passes.begin();
		return true;
	}

	m_active_pass = &(*m_mainp_iter).first;
	return false;
}

Bolt::Render_Submissions::Main_Pass& Bolt::Render_Submissions::Get_Active_Main_Pass()
{
	ASSERT(!m_passes.empty(), "Passes list is empty!");
	return *m_mainp_iter;
}
