#ifndef ROBOT_H
#define ROBOT_H

#include <string>
#include <cstdint>
#include "helpers.h"


//TODO:
//We may need to
//protect variables from undefined behavior of overflow by
//sanitizing input from the robot's teenyatvm

//A robot within an arena
class Robot {
	public:
	//identity and appearance
	int16_t id {0};
	std::string name;
	color_t col {0xFF, 0xFF, 0xFF, 0xFF}; //color
	//color as floats. Used only in debug menu.
	float fcol[4] {(float)col.r/0xFF, (float)col.g/0xFF, (float)col.b/0xFF, (float)col.a/0xFF}; 
	//movement and orientation
	float pos[2] {0, 0}; 	//current position
	float vel[2] {0, 0}; 	//velocity, used in movment calcs, (based on speed)
	float speed {0.0}; 		//used in movement calcs, (based on throttle)
	float t_pos[2] {0, 0}; 	//target position used in movement calc, (pos + vel)
	int16_t throttle {0}; 	//[-100, 100] current speed of movement (follows t_throttle)
	int16_t t_throttle {0};	//[-100, 100] target speed of movement
	bool crashed {false};	//has the robot crashed this frame
	int16_t ang {0}; 		//[0, 359] angle of entire robot (follows t_ang)
	int16_t t_ang {0}; 		//[0, 359] target angle
    const static int16_t turn_rate {1}; 	//rate at which ang approaches t_ang
    const static int16_t throttle_acc {1}; 	//rate at which throttle approaches t_throttle
    constexpr static float max_spd {1.0f}; 		//the speed that corresponds to a throttle of 100 
	const static int16_t crash_range {8*8}; //the square of the collision distance
	//turret stuff
	int16_t tur_offset {0};	//[-180, 179] turret angle offset from ang
	int16_t tur_ang {ang};	//[0, 359] the turret's actual angle, (ang + tur_offset) mod 360
	//scanner stuff
	bool scan {false};		//scan this frame
	int16_t scan_arc {20};	//[1, scan_arc_max] angle of the scan arc
	int16_t scan_len {40};	//[0, scan_len_max] distance of the scan
	int16_t scan_arc_max {90};
	int16_t scan_len_max {255};
	/* const static int16_t scan_arc_max {180}; */
	/* const static int16_t scan_len_max {255}; */
	int16_t num_targets {0};//number of targets found in the most recent scan
	//weapon state
	bool fire_missile {false};	//fire a missile this frame
	bool drop_mine {false};	//drop a mine this frame
	int16_t num_mines {5};
	//robot state
	float heat {0.0f};	//how hot the robot is
	constexpr static float firing_heat {20.0f};	//heat to add upon firing a missile
	constexpr static float heat_loss {0.005f};	//how much heat to lose per frame
	constexpr static float heat_per_dmg {1.0f};	//how much heat to add per point of damage taken to armor
	int16_t armor {100};	//the robot's health
	bool alive {true};		//true if armor > 0, but only updated at the end of a tick
	//variables
	Robot(int16_t r_id, std::string r_name);
	Robot(int16_t r_id, std::string r_name, color_t col, float p[2], int16_t ang);
	void GuiDebugMenu();
	void Damage(int16_t dmg);
	void SetCol(color_t c);
	void InitPosition(float new_pos[2]);
	void InitAngle(uint16_t new_ang);
};

#endif
