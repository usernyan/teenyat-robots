#include <iostream>
#include <map>
#include <algorithm>
#include "robot_arena.h"
#include "robot.h"
#include "robot_controller.h"
#include "imgui.h"
#include "helpers.h"
#include "texture.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

Missile::Missile(float start_pos[2], uint16_t start_ang, int source_id) {
	pos[0] = start_pos[0];
	pos[1] = start_pos[1];
	ang = start_ang;
	source = source_id;
}

Mine::Mine(float start_pos[2], int source_id) {
	pos[0] = start_pos[0];
	pos[1] = start_pos[1];
	source = source_id;
}

Explosion::Explosion(float init_pos[2], int16_t init_dmg, int16_t init_range) {
	pos[0] = init_pos[0];
	pos[1] = init_pos[1];
	dmg = init_dmg;
	range = init_range;
}

void Explosion::LogicStep() {
	//track the radius
	done = cur_radius >= end_radius;
	if (t >= ticks_per_step) {
		cur_radius += step_radius;
		t = 0;
	}
	t++;
	frame_num++;
}

void RobotArena::LogicStep() {
	//let the controller change the robot this frame
	for (auto p : robs) {
		auto id = std::get<0>(p);
		auto r = std::get<1>(p);
		if (!r->alive)
			continue;
		if (controllers.find(id) != controllers.end())
			controllers[id]->AdvanceRobot(r);
	}
	for (auto p : robs) {
		auto r = std::get<1>(p);
		if (!r->alive)
			continue;
		//correct variables
		r->ang = mod(r->ang, 360);
		r->scan_arc = std::max((int16_t)1, r->scan_arc);
		r->scan_arc = std::min(r->scan_arc_max, r->scan_arc);
		r->scan_len = std::max((int16_t)0, r->scan_len);
		r->scan_len = std::min(r->scan_len_max, r->scan_len);
		//TODO: if you remove this line, the robots sometimes instantly rotate while setting t_ang... why, and will this break other things later?
		r->t_ang = mod(r->t_ang, 360);
		r->tur_ang = mod(r->ang + r->tur_offset, 360);
		//heat loss
		r->heat = r->heat * (1 - r->heat_loss);
	}

	//fire new missiles
	for (auto p : robs) {
		auto r = std::get<1>(p);
		if (!r->alive)
			continue;
		if (r->fire_missile) {
			r->fire_missile = false;
			r->heat += r->firing_heat;
			missiles.push_back(new Missile(r->pos, r->tur_ang, r->id));
		}
	}

	//tick missiles
	auto m_iter = missiles.begin();
	while(m_iter != missiles.end()) {
		auto m = *m_iter;
		//check if we've hit anything
		bool hit = (m->pos[0] < 0 || m->pos[0] > size[0] || m->pos[1] < 0 || m->pos[1] > size[1]);
		for ( auto p : robs ) {
			auto r = std::get<1>(p);
			if (!r->alive)
				continue;
			if (r->id == m->source) //don't collide with our source
				continue;
			if (dist_sq(m->pos, r->pos) <= sqr(m->range)) {
				hit = true;
				/* r->Damage(m->dmg); */
				explosions.push_back(new Explosion(m->pos, m->dmg, m->explode_range));
			}
		}
		if (hit) {
			delete m;
			m_iter = missiles.erase(m_iter);
		}
		else {
			//move the missile forward
			float ms = sin_360(m->ang);
			float mc = cos_360(m->ang);
			m->pos[0] += m->spd * mc;
			m->pos[1] += m->spd * ms;
			m_iter++;
		}
	}

	//drop new mines
	for (auto p : robs) {
		auto r = std::get<1>(p);
		if (!r->alive)
			continue;
		if (r->drop_mine) {
			r->drop_mine = false;
			if (r->num_mines > 0) {
				mines.push_back(new Mine(r->pos, r->id));
				r->num_mines--;
			}
		}
	}

	//tick mines
	auto min_iter = mines.begin();
	while(min_iter != mines.end()) {
		auto min = *min_iter;
		for (auto p : robs) {
			auto r = std::get<1>(p);
			if (!r->alive)
				continue;
			if (r->id == min->source) //don't let the source detonate the mine
				continue;
			min->detonate |= dist_sq(min->pos, r->pos) <= sqr(min->detect_range);
		}
		if (min->detonate) {
			/* for (auto r : robs_in_range) */
			/* 	r->Damage(min->dmg); */
			explosions.push_back(new Explosion(min->pos, min->dmg, min->explode_range));
			delete min;
			min_iter = mines.erase(min_iter);
		}
		else {
			min_iter++;
		}
	}

	//scan
	for(auto p : robs) {
		auto r = std::get<1>(p);
		if (!r->alive)
			continue;
		if (r->scan) {
			r->scan = false;
			Scan(r);
		}
	}

	//turning and plan movement
	for (auto p : robs) {
		Robot *r = std::get<1>(p);
		if (!r->alive)
			continue;
		if (r->t_throttle > 100)
			r->t_throttle = 100;
		else if (r->t_throttle < -100)
			r->t_throttle = -100;
		//resued variables
        float s = sin_360(r->ang);
        float c = cos_360(r->ang);

		//turning
		int ang_diff = std::abs(r->ang - r->t_ang);
		bool within_half = ang_diff < 180;
		bool target_less = r->t_ang < r->ang;
		if (ang_diff <= r->turn_rate || (ang_diff >= 360 - r->turn_rate)) {
			r->ang = r->t_ang;
		}
		else if (within_half == target_less) {
			r->ang -= r->turn_rate; //counter-clockwise
		}
		else {
			r->ang += r->turn_rate; //clockwise
		}
		r->ang = mod(r->ang, 360);

		//plan movement
		r->speed = r->throttle/100.0 * r->max_spd;
		r->vel[0] = r->speed * c;
		r->vel[1] = r->speed * s;
		r->t_pos[0] = r->pos[0] + r->vel[0];
		r->t_pos[1] = r->pos[1] + r->vel[1];

		int throttle_dif = r->t_throttle - r->throttle;
		int shifter = sizeof(r->throttle) * 8 - 1;
		if (std::abs(throttle_dif) > r->throttle_acc ) {
			//-1 if throttle_dif < 0, 1 otherwise
			int dif_sign = 1 + ((throttle_dif >> shifter) << 1);
			r->throttle += dif_sign * r->throttle_acc;
		}
		else {
			r->throttle = r->t_throttle;
		}
	}

	//check if the planned movement causes collision
	for (auto p : robs) {
		auto r = std::get<1>(p);
		if (!r->alive)
			continue;
		//wall collisions
		r->crashed = r->t_pos[0] < 0 || r->t_pos[0] > this->size[0] ||
						   r->t_pos[1] < 0 || r->t_pos[1] > this->size[1];
		//TODO: if we already checked if a collides with b, we don't need to check if b collides with a
		//that's redundant. fix this loop
		for (auto p0 : robs) {
			//other robot
			auto o = std::get<1>(p0);
			if (!o->alive)
				continue;
			if (o->id == r->id)
				continue;
			if (std::abs(dist_sq(r->t_pos, o->t_pos)) <= r->crash_range) {
				r->crashed = true;
				o->crashed = true;
			}
		}
	}

	//perform planned movement
	for (auto p: robs) {
		auto r = std::get<1>(p);
		if (!r->alive)
			continue;
		if (r->crashed) {
			//TODO: apply damage for colliding
			r->throttle = 0;
			r->t_pos[0] = r->pos[0];
			r->t_pos[1] = r->pos[1];
		}
		r->pos[0] = r->t_pos[0];
		r->pos[1] = r->t_pos[1];
		r->crashed = false;
	}

	//set if the robot is alive based on armor
	for (auto p: robs) {
		auto r = std::get<1>(p);
		r->alive = r->armor > 0;
	}
	
	auto exp_iter = explosions.begin();
	while(exp_iter != explosions.end()) {
		auto exp = *exp_iter;
		if (exp->frame_num == 0) {
			for (auto p : robs) {
				auto r = std::get<1>(p);
				if (!r->alive)
					continue;
				if ( dist_sq(exp->pos, r->pos) <= sqr(exp->range))
					r->Damage(exp->dmg);
			}
		}
		exp->LogicStep();
		if (exp->done) {
			delete exp;
			exp_iter = explosions.erase(exp_iter);
		}
		else {
			exp_iter++;
		}
	}
}

void RobotArena::Scan(Robot *r) {
	r->num_targets = 0;
	for(auto p : robs) {
		auto o = std::get<1>(p);
		if (!o->alive)
			continue;
		if (o->id == r->id)
			continue;
		int16_t c = r->scan_arc;// angle of the arc
		int16_t b = r->tur_ang; // center of the arc
		int16_t a = find_angle(r->pos, o->pos); //angle from r to o
		int16_t ab_dist = std::abs(b - a);
		bool dist_within = dist_sq(r->pos, o->pos) <= r->scan_len*r->scan_len;
		bool angle_inside = (ab_dist <= c) || (ab_dist >= 360 - c);
		if(angle_inside && dist_within)
			r->num_targets++;
	}
}

void RobotArena::RenderToTex(SDL_Renderer* rend, Texture* tex) {
    SDL_Texture* old_target = SDL_GetRenderTarget(rend);
    SDL_SetRenderTarget(rend, tex->t);
    SDL_SetRenderDrawColor(rend, 0, 0, 0, 100);
    SDL_RenderClear(rend);
	for (std::pair<const int, Robot *> p : robs) {
		Robot *r = std::get<1>(p);
		if (!r->alive)
			continue;
		//TODO: this name collides with size class member
		int size = 4;
		SDL_Point points[size] = { {8, 0}, {-4, 4}, {-4, -4}, {8, 0} };
		SDL_Point rotated[size];
		float s = sin_360(r->ang);
		float c = cos_360(r->ang);
		for (int i = 0; i < size; i++) {
			SDL_Point p = points[i];
			rotated[i] = {
				(int)(p.x * c - p.y * s + r->pos[0]),
				(int)(p.x * s + p.y * c + r->pos[1])
			};
		}
		color_t m = r->col;
		SDL_SetRenderDrawColor(rend, m.r, m.g, m.b, m.a);
		SDL_RenderDrawLines(rend, rotated, size);

		//t_ang indicator
		float t_s = sin_360(r->t_ang);
		float t_c = cos_360(r->t_ang);
		SDL_RenderDrawLine(rend,
			r->pos[0] + t_c * 20, r->pos[1] + t_s * 20,
			r->pos[0] + t_c * 25, r->pos[1] + t_s * 25
		);
		//tur_offset indicator
		float tur_s = sin_360(r->tur_ang);
		float tur_c = cos_360(r->tur_ang);
		SDL_RenderDrawLine(rend,
			r->pos[0] + tur_c * 15, r->pos[1] + tur_s * 15,
			r->pos[0] + tur_c * 20, r->pos[1] + tur_s * 20
		);

		//scan arc
		pieColor(rend, r->pos[0], r->pos[1],
			r->scan_len,
			r->tur_ang - r->scan_arc,
			r->tur_ang + r->scan_arc,
			color_to_int({0xFF, 0xFF, 0xFF, 0x55})
		);
	}

	struct color_t missile_col = {0xFF, 0xFF, 0xFF, 0xFF};
	SDL_SetRenderDrawColor(rend, missile_col.r, missile_col.g, missile_col.b, missile_col.a);
	for (auto m : missiles) {
        float s = sin_360(m->ang);
        float c = cos_360(m->ang);
		SDL_RenderDrawLine(rend,
			m->pos[0] - c * m->render_length, m->pos[1] - s * m->render_length,
			m->pos[0] + c * m->render_length, m->pos[1] + s * m->render_length
		);
	}

	for (auto m : mines) {
		filledCircleColor(rend, m->pos[0], m->pos[1], 1, color_to_int({0xFF, 0xFF, 0xFF, 0xFF}));
        circleColor(rend, m->pos[0], m->pos[1], m->detect_range, color_to_int({0xFF, 0xFF, 0xFF, 0x55}));
	}

	for (auto exp : explosions) {
		circleColor(rend, exp->pos[0], exp->pos[1], exp->cur_radius, color_to_int({0xFF, 0xFF, 0xFF, 0xFF}));
	}

    SDL_SetRenderTarget(rend, old_target);
}

RobotArena::RobotArena(int size[2]) {
	this->size[0] = size[0];
	this->size[1] = size[1];
}

RobotArena::RobotArena(int w, int h) {
	this->size[0] = w;
	this->size[1] = h;
}

RobotArena::~RobotArena() {
	//robots and controllers might have ptrs to them somewhere else,
	//so don't delete them
	//we can delete missiles and mines here though
	for (auto m : missiles) {
		delete m;
	}
	missiles.clear();
	for (auto m : mines) {
		delete m;
	}
	mines.clear();
	for (auto exp : explosions) {
		delete exp;
	}
	explosions.clear();
}

bool RobotArena::ConnectController(int robot_id, RobotController * c) {
	bool id_exists = robs.find(robot_id) != robs.end();
	if ( id_exists ) {
		controllers[robot_id] = c;
		c->AttachTo(robs[robot_id]);
	}
	return id_exists;
}

bool RobotArena::IdTaken(uint16_t id) {
	return robs.find(id) != robs.end();
}

void RobotArena::InsertRobot(Robot *r) {
	robs[r->id] = r;
}
