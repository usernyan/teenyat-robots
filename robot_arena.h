#ifndef ROBOT_ARENA_H
#define ROBOT_ARENA_H

#include <map>
#include <list>
#include "robot.h"
#include "robot_controller.h"
#include "texture.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

class Mine {
	public:
	float pos[2] {0, 0};		//position
	int16_t detect_range {11};	//range within which a robot triggers the mine
	int16_t explode_range {detect_range}; //range within which a robot takes damage from a detonation
	int16_t dmg {20};			//damage taken by robots
	bool detonate {false};
	int source {-1};
	Mine(float start_pos[2], int source_id);
};

class Missile {
	public:
	const static int16_t render_length {6}; //half the length of the line rendered to represent the missile
	const static int16_t range {7};	//range within which a robot is hit
	const static int16_t explode_range {range};
	const static int16_t dmg {5};	//damage to the robot hit
	const static int16_t spd {2};	//amount moved per frame
	float pos[2] {0, 0};	//position
	uint16_t ang {0}; 			//[0, 359]
	//TODO: use this to keep score of kills
	int source {-1}; 	//the id of the robot that spawned the missile (-1 = none)
	Missile(float start_pos[2], uint16_t start_ang, int source_id);
};

class Explosion {
	public:
	//functional stuff
	float pos[2] {0, 0};
	int frame_num {0};
	int16_t dmg {0};
	int16_t range{0};
	int source {-1};
	//visual stuff
	float start_radius {0};
	float step_radius {3};
	float end_radius {start_radius + step_radius * 3 + 1};
	float cur_radius {start_radius};
	int ticks_per_step {12};
	int t {0};
	bool done {false};
	Explosion(float init_pos[2], int16_t init_dmg, int16_t init_range);
	void LogicStep();
};

class RobotArena {
	public:
	RobotArena(int w, int h);
	RobotArena(int size[2]);
	~RobotArena();
	int size[2] {0, 0};
	/* int w, h; */
	//id -> robot ptr
	std::map<int, Robot*> robs;
	//id of robot -> that robot's controller
	std::map<int, RobotController*> controllers;
	std::list<Missile *> missiles;
	std::list<Mine *> mines;
	std::list<Explosion *> explosions;

	bool ConnectController(int robot_id, RobotController * c);
	bool IdTaken(uint16_t id);
	void InsertRobot(Robot *r);
	void LogicStep();
	void Scan(Robot *r);
	void RenderToTex(SDL_Renderer* rend, Texture* tex);
};

#endif
