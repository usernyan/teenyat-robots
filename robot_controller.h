#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "robot.h"
#include "robot_vm/teenyat.h"

//A controller should update the robot's state when AdvanceRobot is called.
class RobotController {
	public:
	virtual ~RobotController();
	virtual void AttachTo(Robot *r);
	virtual void AdvanceRobot(Robot *r);
};

class RobotControllerTeenyAT : public RobotController {
	public:
	RobotControllerTeenyAT();
	~RobotControllerTeenyAT();
	void AttachTo(Robot *r);
	void AdvanceRobot(Robot *r);
	bool LoadFile(const char *s);
	Robot *my_rob {NULL};
	teenyat t;
};

#endif
