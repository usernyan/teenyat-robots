#include "robot_controller.h"
#include "robot.h"
#include "imgui.h"
//for FILE in teenyat
#include <stdio.h>
#include <iostream>
#include "robot_vm/teenyat.h"

RobotControllerTeenyAT::RobotControllerTeenyAT() {
	t.initialized = false;
	t.ex_data = NULL;
	return;
}

RobotControllerTeenyAT::~RobotControllerTeenyAT() {
	return;
}

#define ADDR_BASE				(1 << 15)
#define ADDR_T_THROTTLE			(ADDR_BASE +  0) //read/write
#define ADDR_THROTTLE			(ADDR_BASE +  1) //read
#define ADDR_T_ANGLE			(ADDR_BASE +  2) //read/write
#define ADDR_ANGLE				(ADDR_BASE +  3) //read
#define ADDR_TUR_OFFSET			(ADDR_BASE +  4) //read/write

#define ADDR_SCAN				(ADDR_BASE +  8) //write
#define ADDR_SCAN_ARC			(ADDR_BASE +  9) //read/write
#define ADDR_SCAN_LEN			(ADDR_BASE + 10) //read/write

#define ADDR_MISSILE_FIRE		(ADDR_BASE + 16) //write

#define ADDR_MINE_DROP			(ADDR_BASE + 32) //write
#define ADDR_MINE_NUM			(ADDR_BASE + 33) //read

//access the teenyat ex_data pointer for the robot bound to this teenyat
void robot_bus_read(teenyat *t, tny_uword addr, tny_word *data, uint16_t *delay) {
	Robot *r = (Robot *)t->ex_data;
	*delay = 0;
	if (r == NULL)
		return;
	std::cout << r->name << " read";
	std::cout << "(" << addr << ")";
	switch (addr) {
		case ADDR_T_THROTTLE:
			data->s = r->t_throttle;
			*delay = 1;
			std::cout << " t_throttle";
			break;
		case ADDR_THROTTLE:
			data->s = r->throttle;
			*delay = 1;
			std::cout << " throttle";
			break;
		case ADDR_T_ANGLE:
			data->s = r->t_ang;
			*delay = 1;
			std::cout << " t_angle";
			break;
		case ADDR_ANGLE:
			data->s = r->ang;
			*delay = 1;
			std::cout << " angle";
			break;
		case ADDR_TUR_OFFSET:
			data->s = r->tur_offset;
			*delay = 1;
			std::cout << " tur_offset";
			break;
		case ADDR_SCAN_ARC:
			data->s = r->scan_arc;
			*delay = 1;
			std::cout << " scan_arc";
			break;
		case ADDR_SCAN_LEN:
			data->s = r->scan_len;
			*delay = 1;
			std::cout << " scan_len";
			break;
		case ADDR_MINE_NUM:
			data->s = r->num_mines;
			*delay = 1;
			std::cout << " num_mines";
			break;
		default:
			//apply penalty for a read from an unused address
			data->s = 0;
			*delay = 20;
			std::cout << " ERROR";
			break;
	}
	std::cout << std::endl;
	return;
}

void robot_bus_write(teenyat *t, tny_uword addr, tny_word data, uint16_t *delay) {
	Robot *r = (Robot *)t->ex_data;
	*delay = 0;
	if (r == NULL)
		return;
	std::cout << r->name << " writ";
	std::cout << "(" << addr << ")";
	switch (addr) {
		case ADDR_T_THROTTLE:
			r->t_throttle = data.s;
			*delay = 1;
			std::cout << " t_throttle";
			break;
		case ADDR_T_ANGLE:
			r->t_ang = data.s;
			*delay = 1;
			std::cout << " t_angle";
			break;
		case ADDR_TUR_OFFSET:
			r->tur_offset = data.s;
			*delay = 1;
			std::cout << " tur_offset";
			break;
		case ADDR_SCAN:
			r->scan = data.s != (int16_t)0;
			*delay = 1;
			std::cout << " scan";
			break;
		case ADDR_SCAN_ARC:
			r->scan_arc = data.s;
			*delay = 1;
			std::cout << " scan_arc";
			break;
		case ADDR_SCAN_LEN:
			r->scan_len = data.s;
			*delay = 1;
			std::cout << " scan_len";
			break;
		case ADDR_MISSILE_FIRE:
			r->fire_missile = data.s;
			*delay = 1;
			std::cout << " fire_missile";
			break;
		case ADDR_MINE_DROP:
			r->drop_mine = data.s;
			*delay = 1;
			std::cout << " drop_mine";
			break;
		default:
			//apply a penalty for a write to an unused address
			*delay = 20;
			std::cout << " ERROR";
			break;
	}
	std::cout << std::endl;
	return;
}

bool RobotControllerTeenyAT::LoadFile(const char *s) {
	bool success = false;
	FILE *bin_file = fopen(s, "rb");
	if (bin_file != NULL) {
		success = true;
		tny_init_from_file(&t, bin_file, robot_bus_read, robot_bus_write);
		fclose(bin_file);
	}
	//we need to do this again, since tny_init_from_file zeros the whole struct
	t.ex_data = (void *) my_rob;
	return success;
}

void RobotControllerTeenyAT::AttachTo(Robot *r) {
	my_rob = r;
}

void RobotControllerTeenyAT::AdvanceRobot(Robot *r) {
	//TODO: use the robot's heat to penalize it with missed cycles
	if (t.initialized)
		tny_clock(&t);
}

void RobotController::AdvanceRobot(Robot *r) {return;}

void RobotController::AttachTo(Robot *r) {return;}

RobotController::~RobotController() {return;}
