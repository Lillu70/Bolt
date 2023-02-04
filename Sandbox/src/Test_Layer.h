#pragma once
#include <Bolt.h>

class Test_Layer : public Bolt::Layer
{
public:
	void Initialize() override;
	void Update(Bolt::Time time_step) override;

private:
	Bolt::Entity player;

};

