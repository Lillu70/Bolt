#pragma once
#include <Bolt.h>


class Test_Layer : public Bolt::Layer
{
public:
	void Initialize() override;
	void User_Update(Bolt::Time time_step) override;
	void User_Renderer_Submit(Bolt::Render_Submissions& submissions, glm::vec3 camera_position) override;

	void Select_And_Modify_Transforms(float time_step);
	bool Get_KP_Directional_Input(glm::vec3& direction);

private:
	Bolt::Entity player;
	Bolt::Entity camera;

	bool m_visualize_transforms = false;
	Bolt::Entity m_selected_entity;
};

