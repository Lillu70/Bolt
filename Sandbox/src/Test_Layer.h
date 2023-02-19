#pragma once
#include <Bolt.h>


class Test_Layer : public Bolt::Layer
{
public:
	void Initialize() override;
	void Update(Bolt::Time time_step) override;

	void Display_Name_Tags();
	void Transform_Select_And_Modify(float time_step);

private:
	Bolt::Entity player;
	Bolt::Entity camera;

};

