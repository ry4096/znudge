#include "cg_local.h"

// Routines for predicting player movement into the future.

#define ZN_MAX_OFFSET 1000

/*
===============
ZN_GetNudge

Returns the time in seconds to nudge the current frame by.
Nudge is your ping plus frametime.
===============
*/
float ZN_GetNudge() {
	int ping;
	float nudge;

	if (zn_offset.integer > ZN_MAX_OFFSET) 
		zn_offset.integer = ZN_MAX_OFFSET;

	if (zn_offset.integer < -ZN_MAX_OFFSET) 
		zn_offset.integer = -ZN_MAX_OFFSET;

	ping = cg.snap ? cg.snap->ping : 0;
	if (zn_ping_weight.value <= 1.0) {

		if (zn_ping_weight.value < 0.0)
			zn_ping_weight.value = 0.0;

		//CG_Printf("ZN_GetNudge: ping: %d, smooth_ping %d\n", ping, (int)cg.smooth_ping);

		cg.smooth_ping = ((float)ping)*zn_ping_weight.value + cg.smooth_ping*(1.0 - zn_ping_weight.value);
		ping = (int)cg.smooth_ping;
	}

	nudge = (ping + zn_offset.integer)/1000.0;

	return nudge;
}


/*
===============
ZN_TimeToPoint

Computes how long it takes for a point starting at origin with
the given velocity and gravity to reach the point destination.
It just tries to use time_delta = pos_delta/velocity
and use x or y to get better precision. If both x and y are bad it uses z
and uses the quadratic equation if gravity is nonzero.
===============
*/
float ZN_TimeToPoint( vec3_t origin, vec3_t velocity, float gravity, vec3_t destination ) {
	vec3_t diff;
	int coord_num = -1;
	int diff_coords_sorted[3] = {0, 1, 2};
	int temp;
	int i;
	int j;
	int coord;

	diff[0] = destination[0] - origin[0];
	diff[1] = destination[1] - origin[1];
	diff[2] = destination[2] - origin[2];

	// Insertion sort:
	// Sort the coordinates of diff in decreasing order by magnitude.
	for (i = 1; i < 3; i++) {
		for (j = 0; j < i; j++) {
			if (abs(diff[i]) > abs(diff[j])) {
				temp = diff_coords_sorted[i];
				diff_coords_sorted[i] = diff_coords_sorted[j];
				diff_coords_sorted[j] = temp;
			}
		}
	}

	if (diff[diff_coords_sorted[0]] == 0.0) {
		// diff is the zero vector. Return time zero.
		//CG_Printf("ZN_TimeToPoint zero vec: (%f, %f, %f)\n", diff[0], diff[1], diff[2]);
		return 0.0;
	}

	for (i = 0; i < 3; i++) {
		coord = diff_coords_sorted[i];
		if (coord == 2 && gravity != 0.0) {
			// Use quadratic formula:
			float a = -gravity/2.0;
			float b = velocity[2];
			float c = -diff[2];
		
			float disc = b*b - 4.0*a*c;

			if (disc < 0.0) {
				// No real solution.
			}
			else {
				float sqrt_disc = sqrt(disc);
				float root1 = (-b + sqrt_disc)/(2.0*a);
				float root2 = (-b - sqrt_disc)/(2.0*a);
				float time;

				if (root1 >= 0.0 || root2 >= 0.0) {
					if (root1 < 0.0)
						time = root2;
					else if (root2 < 0.0)
						time = root1;
					else if (root1 < root2)
						time = root1;
					else
						time = root2;
				
					//CG_Printf("ZN_TimeToPoint quadratic: %f\n", time);
					return time;
				}
			}
		}
		else if (velocity[coord] != 0.0) {
			// Use linear prediction:
			float time = ((float)diff[coord])/((float)velocity[coord]);
			//CG_Printf("ZN_TimeToPoint linear: %f\n", time);
			return time;
		}
	}

	//CG_Printf("ZN_TimeToPoint failed\n");
	return -1.0;
}

/*
===============
ZN_PredictSimple

Predicts where a player origin will be after nudge seconds,
without considering clipping against obstacles.
===============
*/
void ZN_PredictSimple( vec3_t origin, vec3_t velocity, float gravity, float nudge, vec3_t predicted) {
	predicted[0] = origin[0] + velocity[0]*nudge;
	predicted[1] = origin[1] + velocity[1]*nudge;
	predicted[2] = origin[2] + velocity[2]*nudge;

	if (gravity != 0.0) {
		predicted[2] -= nudge*nudge*gravity/2.0;
	}
}


/*
===============
ZN_CheckGround

Checks if the player is on the ground, and if they need
to be boosted upwards after walking over stairs.
===============
*/
int ZN_CheckGround( centity_t* cent, vec3_t origin, vec3_t velocity, vec3_t predictedOrigin ) {
	vec3_t ground_trace_mins = {-12, -12, 0}, ground_trace_maxs = {-12, -12, 10};
	vec3_t ground_trace_start;
	vec3_t ground_trace_end;
	trace_t	trace;
	int on_ground;

	// Figure out if we are on the ground now.
	ground_trace_start[0] = origin[0];
	ground_trace_start[1] = origin[1];
	ground_trace_start[2] = origin[2] + 32 - ground_trace_maxs[2];

	ground_trace_end[0] = origin[0];
	ground_trace_end[1] = origin[1];
	ground_trace_end[2] = origin[2] - 26;

	//trap_CM_BoxTrace( &trace, ground_trace_start, ground_trace_end, ground_trace_mins, ground_trace_maxs, 0, MASK_PLAYERSOLID );
	CG_Trace( &trace, ground_trace_start, NULL, NULL, ground_trace_end, cent->currentState.number, MASK_PLAYERSOLID );

	// MIN_WALK_NORMAL = .7
	on_ground = (trace.fraction < 1.0)
			//&& (origin[2] - trace.endpos[2] <= 24)
			//&& trace.plane.normal[2] < 0.7
			;

	if (on_ground) {
		float speed;
		float max_speed;
		float new_z = trace.endpos[2] + 24;

		if (new_z > origin[2]) {
			//CG_Printf("ZN_Step: shift up: %f\n", new_z - origin[2]);
			origin[2] = predictedOrigin[2] = new_z;
		}

		// Cap running speed.
		speed = sqrt(velocity[0]*velocity[0] + velocity[1]*velocity[1]);

		max_speed = zn_runningspeed.value;
		if (cent->currentState.powerups & (1 << PW_HASTE))
			max_speed *= 1.3;

		if (speed > max_speed) {
			float ratio = max_speed/speed;
			velocity[0] *= ratio;
			velocity[1] *= ratio;
		}
	}

	return on_ground;
}

void ZN_GetVelocity ( centity_t* cent, vec3_t velocity ) {

	int index = cent->currentState.number;

	if ( zn_smoothweight.value > 1.0 )
		zn_smoothweight.value = 1.0;

	if ( zn_smoothweight.value < 0.0 )
		zn_smoothweight.value = 0.0;

	if ( zn_smoothweight.value == 1.0 ) {
		velocity[0] = cent->currentState.pos.trDelta[0];
		velocity[1] = cent->currentState.pos.trDelta[1];
		velocity[2] = cent->currentState.pos.trDelta[2];
	}
	else {

/*
		// Should be made FPS independent
		float cpuFrameTime = (float)cg.cpuFrameTime;
		float FPS = 1000.0/cpuFrameTime;

		// weight is the FPS-nth root of zn_smoothweight.value
		//float weight = exp(log(zn_smoothweight.value)/FPS);
*/
		float weight = zn_smoothweight.value;

		velocity[0] = cg.smoothVelocities[index][0] =
			(1 - weight)*cg.smoothVelocities[index][0] + weight*cent->currentState.pos.trDelta[0];
		velocity[1] = cg.smoothVelocities[index][1] =
			(1 - weight)*cg.smoothVelocities[index][1] + weight*cent->currentState.pos.trDelta[1];
		velocity[2] = cg.smoothVelocities[index][2] =
			(1 - weight)*cg.smoothVelocities[index][2] + weight*cent->currentState.pos.trDelta[2];
	}
}

/*
===============
ZN_PredictPlayer

Predict's the origin of the player cent after nudge seconds
have elapsed. This function will be improved over time...
===============
*/
void ZN_PredictPlayer( centity_t* cent, float nudge, vec3_t predictedOrigin ) {
	//vec3_t mins = {-15, -15, -24}, maxs = {15, 15, 32};
	vec3_t mins = {-13, -13, -22}, maxs = {13, 13, 30};
	int in_water = 0;
	int on_ground = (cent->currentState.groundEntityNum != ENTITYNUM_NONE);
	int was_on_ground = on_ground;
	trace_t	trace;
	float nudge_remaining = nudge;
	int clips = 0;
	vec3_t origin;
	vec3_t velocity;
	vec3_t temp;
	float gravity;
	int content_mask;
	int contents;

	origin[0] = cent->lerpOrigin[0];
	origin[1] = cent->lerpOrigin[1];
	origin[2] = cent->lerpOrigin[2];

	ZN_GetVelocity ( cent, velocity );

	predictedOrigin[0] = origin[0];
	predictedOrigin[1] = origin[1];
	predictedOrigin[2] = origin[2];

	while (nudge_remaining > 0.0 && clips < zn_maxclips.integer) {
/*
		if (clips > 0)
			CG_Printf("ZN_Loop: clips = %d, on_ground = %d, ground_ent=%d, nudge_remaining = %f\n",
					clips, on_ground, cent->currentState.groundEntityNum, nudge_remaining);
*/

		temp[0] = origin[0];
		temp[1] = origin[1];
		temp[2] = origin[2] - 24;
		contents = trap_CM_PointContents( temp, 0 );
		in_water = contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA );

		if (on_ground) {
			// reduce player bounding box from below to avoid clipping
			// the ground and stairs.
			mins[2] = -24 + zn_climbheight.integer;
			gravity = 0.0;
		}
		else {
			mins[2] = -24;

			if (cent->currentState.powerups & (1 << PW_FLIGHT) || in_water) {
				gravity = 0.0;
			}
			else {
				gravity = zn_gravity.value;
			}
		}

		ZN_PredictSimple( origin, velocity, gravity, nudge_remaining, predictedOrigin );

		//trap_CM_BoxTrace( &trace, origin, predictedOrigin, mins, maxs, 0, MASK_PLAYERSOLID | CONTENTS_JUMPPAD );
		trap_CM_BoxTrace( &trace, origin, predictedOrigin, mins, maxs, 0, MASK_PLAYERSOLID );

		if (trace.fraction == 1.0) {
			// No obstruction.
			break;
		}
		else {
			// Clipped against something...
			float travel_time;
			float dot;
			clips++;

			travel_time = ZN_TimeToPoint( origin, velocity, gravity, trace.endpos );
			if (travel_time < 0.0) {
				// Shouldn't happen.
				travel_time = .05;
			}
			nudge_remaining -= travel_time;

			origin[0] = predictedOrigin[0] = trace.endpos[0];
			origin[1] = predictedOrigin[1] = trace.endpos[1];
			origin[2] = predictedOrigin[2] = trace.endpos[2];

			//CG_Printf("Travel_time: %f\n", travel_time);

			if (!on_ground) {
				// Update velocity with gravity
				velocity[2] -= gravity*travel_time;
			}

			// Check jump pad.
			//void BG_TouchJumpPad( playerState_t *ps, entityState_t *jumppad )
			//VectorCopy( jumppad->origin2, ps->velocity );
			//if (trace.contents & CONTENTS_JUMPPAD) {
				//CG_Printf("Jumppad hit!\n");
			//}

			// Clip velocity against the plane, assuming sliding friction.
			dot = velocity[0]*trace.plane.normal[0] +
				velocity[1]*trace.plane.normal[1] +
				velocity[2]*trace.plane.normal[2];

			velocity[0] -= dot*trace.plane.normal[0];
			velocity[1] -= dot*trace.plane.normal[1];
			velocity[2] -= dot*trace.plane.normal[2];
		}

		on_ground = ZN_CheckGround( cent, origin, velocity, predictedOrigin );

/*
		if (on_ground && !was_on_ground)
			CG_Printf("not on_ground to on_ground\n");
		if (!on_ground && was_on_ground)
			CG_Printf("on_ground to not on_ground\n");
*/

		was_on_ground = on_ground;
	}
}



void ZN_PredictMissile( centity_t* cent, float nudge, vec3_t predictedOrigin ) {
	trace_t tr;

	predictedOrigin[0] = cent->lerpOrigin[0] + nudge*cent->currentState.pos.trDelta[0];
	predictedOrigin[1] = cent->lerpOrigin[1] + nudge*cent->currentState.pos.trDelta[1];
	predictedOrigin[2] = cent->lerpOrigin[2] + nudge*cent->currentState.pos.trDelta[2];

	CG_Trace( &tr, cent->lerpOrigin, NULL, NULL, predictedOrigin, -1, MASK_SHOT );

	predictedOrigin[0] = tr.endpos[0];
	predictedOrigin[1] = tr.endpos[1];
	predictedOrigin[2] = tr.endpos[2];
}



void ZN_PredictGrenade( centity_t* cent, float nudge, vec3_t predictedOrigin ) {
	trace_t tr;
	int clips = 0;
	float nudge_remaining = nudge;
	float stick_speed_sq = zn_stick_speed.value*zn_stick_speed.value;
	vec3_t origin;
	vec3_t velocity;

	origin[0] = cent->lerpOrigin[0];
	origin[1] = cent->lerpOrigin[1];
	origin[2] = cent->lerpOrigin[2];

	velocity[0] = cent->currentState.pos.trDelta[0];
	velocity[1] = cent->currentState.pos.trDelta[1];
	velocity[2] = cent->currentState.pos.trDelta[2];

	while ( nudge_remaining > 0.0 && clips < zn_maxclips.integer ) {
		float nudge_step = nudge_remaining;

		if (zn_step_size.value > 0.0 && nudge_step > zn_step_size.value) {
			nudge_step = zn_step_size.value;
		}

		ZN_PredictSimple( origin, velocity, zn_gravity.value, nudge_step, predictedOrigin );

		CG_Trace( &tr, origin, NULL, NULL, predictedOrigin, -1, MASK_SHOT );

		if (tr.allsolid || tr.startsolid) {
			predictedOrigin[0] = origin[0];
			predictedOrigin[1] = origin[1];
			predictedOrigin[2] = origin[2];
			break;
		}
		else if (tr.fraction == 1.0) {
			nudge_remaining -= nudge_step;

			origin[0] = predictedOrigin[0];
			origin[1] = predictedOrigin[1];
			origin[2] = predictedOrigin[2];

			velocity[2] -= nudge_step*zn_gravity.value;
		}
		else {
			float travel_time;
			float speed_sq;
			float dot;
			clips++;

			travel_time = ZN_TimeToPoint( origin, velocity, zn_gravity.value, tr.endpos );
			if (travel_time < 0.0) {
				// Shouldn't happen.
				travel_time = .05;
			}
			nudge_remaining -= travel_time;

			origin[0] = predictedOrigin[0] = tr.endpos[0];
			origin[1] = predictedOrigin[1] = tr.endpos[1];
			origin[2] = predictedOrigin[2] = tr.endpos[2];

			velocity[2] -= zn_gravity.value*travel_time;

			// Reflect velocity by plane.
			dot = velocity[0]*tr.plane.normal[0] + velocity[1]*tr.plane.normal[1] + velocity[2]*tr.plane.normal[2];

			velocity[0] -= 2.0*dot*tr.plane.normal[0];
			velocity[1] -= 2.0*dot*tr.plane.normal[1];
			velocity[2] -= 2.0*dot*tr.plane.normal[2];

			// Dampen by bounce factor.
			velocity[0] *= zn_bounce_factor.value;
			velocity[1] *= zn_bounce_factor.value;
			velocity[2] *= zn_bounce_factor.value;

			//VectorScale( ent->s.pos.trDelta, 0.65, ent->s.pos.trDelta );

			speed_sq = velocity[0]*velocity[0] + velocity[1]*velocity[1] + velocity[2]*velocity[2];

			//if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40 ) { }
			if ( tr.plane.normal[2] > zn_plane_up.value && speed_sq < stick_speed_sq ) {
				// Grenade remain stationary forever here.
				break;
			}
		}
	}
}


// Only used for the client.
void ZN_CalcMuzzlePoint( playerState_t* ps, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {

	AngleVectors (ps->viewangles, forward, right, up);

	VectorCopy( ps->origin, muzzlePoint );
	muzzlePoint[2] += ps->viewheight;
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );

	// Compatibility with game.
	SnapVector( muzzlePoint );
}




localEntity_t* ZN_LocalProjectile( playerState_t* ps, float speed ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			forward, right, up, muzzlePoint;

	int lifetime = (int)(ZN_GetNudge() * 1000.0) + zn_localextend.integer;

	ZN_CalcMuzzlePoint( ps, forward, right, up, muzzlePoint );

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + lifetime;

	le->pos.trType = TR_LINEAR;
	le->pos.trTime = cg.time - zn_localprestep.integer;

	VectorCopy( muzzlePoint, re->origin );
	VectorCopy( muzzlePoint, re->oldorigin );
	VectorCopy( re->origin, le->pos.trBase );
	VectorScale( forward, speed, le->pos.trDelta );
	SnapVector( le->pos.trDelta );

	le->bounceFactor = 0.0;

	le->angles.trType = TR_STATIONARY;
	VectorCopy( ps->viewangles, le->angles.trBase );

	le->leFlags = 0;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_NONE;

	AnglesToAxis( ps->viewangles, re->axis );

	return le;
}


localEntity_t* ZN_LocalGrenade( playerState_t* ps, float speed, int weapon_num ) {
	const weaponInfo_t *weapon = &cg_weapons[ weapon_num ];

	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			forward, right, up, muzzlePoint;

	int lifetime = (int)(ZN_GetNudge() * 1000.0) + zn_localextend.integer;

	ZN_CalcMuzzlePoint( ps, forward, right, up, muzzlePoint );
	
	forward[2] += zn_grenade_shift.value;
	VectorNormalize( forward );

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + lifetime;

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - zn_localprestep.integer;

	VectorCopy( muzzlePoint, re->origin );
	VectorCopy( muzzlePoint, re->oldorigin );
	VectorCopy( re->origin, le->pos.trBase );
	VectorScale( forward, speed, le->pos.trDelta );
	SnapVector( le->pos.trDelta );

	le->bounceFactor = zn_bounce_factor.value;

	le->angles.trType = TR_LINEAR;
	VectorCopy( ps->viewangles, le->angles.trBase );

	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_NONE;

	AnglesToAxis( ps->viewangles, re->axis );

	re->reType = RT_MODEL;
	re->skinNum = cg.clientFrame & 1;
	re->hModel = weapon->missileModel;
	re->renderfx = weapon->missileRenderfx | RF_NOSHADOW;


	return le;
}



void ZN_LocalPlasma( playerState_t* ps, float speed ) {
	localEntity_t	*le = ZN_LocalProjectile( ps, speed );
	refEntity_t		*re;
	re = &le->refEntity;

	re->reType = RT_SPRITE;
	re->radius = 16;
	re->rotation = 0;
	re->customShader = cgs.media.plasmaBallShader;
}


void ZN_LocalMissile( playerState_t* ps, float speed, int weapon_num ) {
	const weaponInfo_t *weapon = &cg_weapons[ weapon_num ];

	localEntity_t	*le = ZN_LocalProjectile( ps, speed );
	refEntity_t		*re;
	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->skinNum = cg.clientFrame & 1;
	re->hModel = weapon->missileModel;
	re->renderfx = weapon->missileRenderfx | RF_NOSHADOW;
}


void ZN_LocalRail( playerState_t* ps ) {
	clientInfo_t* ci = &cgs.clientinfo[ cg.clientNum ];
	vec3_t	forward, right, up, muzzlePoint;
	vec3_t start;
	vec3_t end;
	trace_t trace;

	ZN_CalcMuzzlePoint( ps, forward, right, up, muzzlePoint );

	// Match offset in game code.
	VectorMA( muzzlePoint, 4, right, start );
	VectorMA( start, -1, up, start );

	VectorMA( muzzlePoint, 8192, forward, end );

	CG_Trace (&trace, muzzlePoint, NULL, NULL, end, cg.clientNum, CONTENTS_SOLID );
	VectorCopy( trace.endpos, end );

	CG_RailTrail( ci, start, end );
}




int ZN_FireDelay( playerState_t* ps, int weapon ) {
	int delay;

	switch (weapon) {
	case WP_GAUNTLET:
		delay = 400;
		break;
	case WP_LIGHTNING:
		delay = 50;
		break;
	case WP_SHOTGUN:
		delay = 1000;
		break;
	case WP_MACHINEGUN:
		delay = 100;
		break;
	case WP_GRENADE_LAUNCHER:
		delay = 800;
		break;
	case WP_ROCKET_LAUNCHER:
		delay = 800;
		break;
	case WP_PLASMAGUN:
		delay = 100;
		break;
	case WP_RAILGUN:
		delay = 1500;
		break;
	case WP_BFG:
		delay = 200;
		break;
	case WP_GRAPPLING_HOOK:
		delay = 400;
		break;
#ifdef MISSIONPACK
	case WP_NAILGUN:
		delay = 1000;
		break;
	case WP_PROX_LAUNCHER:
		delay = 800;
		break;
	case WP_CHAINGUN:
		delay = 30;
		break;
#endif
	}

	if ( ps->powerups[PW_HASTE] ) {
		delay /= 1.3;
	}

	return delay;
}


static int zn_old_weapon_time = 0;
static int zn_old_weapon_state = 0;

void ZN_CheckFireEvent() {
	usercmd_t cmd;
	int cmdNum;
	int fire;

	cmdNum = trap_GetCurrentCmdNumber();
	trap_GetUserCmd( cmdNum, &cmd );

	cg.fire_held = cmd.buttons & 1;
	//cg.weapon_num = cmd.weapon;
	cg.weapon_num = cg.predictedPlayerState.weapon;

	//fire = cg.fire_held && (cg.next_fire_time <= cg.time);

	// Make this more robust, and less complicated...
	fire = cg.fire_held && (zn_old_weapon_time <= cg.frametime) &&
		cg.predictedPlayerState.ammo[cg.weapon_num] > 0 &&
		((zn_old_weapon_state == WEAPON_READY) ||
		 (zn_old_weapon_state == WEAPON_FIRING)) &&
		((cg.predictedPlayerState.weaponstate == WEAPON_READY) ||
		 (cg.predictedPlayerState.weaponstate == WEAPON_FIRING)) &&
		(cg.predictedPlayerState.stats[STAT_HEALTH] > 0) &&
                !(cg.predictedPlayerState.pm_flags & PMF_RESPAWNED ) &&
		(cg.predictedPlayerState.persistant[PERS_TEAM] != TEAM_SPECTATOR);

	zn_old_weapon_time = cg.predictedPlayerState.weaponTime;
	zn_old_weapon_state = cg.predictedPlayerState.weaponstate;

/*
	fire = cg.fire_held && cg.next_fire_time <= cg.time &&
		(cg.predictedPlayerState.weaponstate == WEAPON_READY ||
		 cg.predictedPlayerState.weaponstate == WEAPON_FIRING);
*/

	if (fire) {
		float speed;
		cg.next_fire_time = cg.time + ZN_FireDelay( &cg.predictedPlayerState, cg.weapon_num );


		switch( cg.weapon_num ) {
		case WP_GRENADE_LAUNCHER:
			if (zn_projectiles.integer && zn_localprojectiles.integer)
				ZN_LocalGrenade( &cg.predictedPlayerState, 700.0, cg.weapon_num );
			break;
		case WP_ROCKET_LAUNCHER:
			if (zn_projectiles.integer && zn_localprojectiles.integer)
				ZN_LocalMissile( &cg.predictedPlayerState, 900.0, cg.weapon_num );
			break;
		case WP_RAILGUN:
			if (zn_localrail.integer)
				ZN_LocalRail( &cg.predictedPlayerState );
			break;
		case WP_PLASMAGUN:
			if (zn_projectiles.integer && zn_localprojectiles.integer)
				ZN_LocalPlasma( &cg.predictedPlayerState, 2000.0 );
			break;
		case WP_BFG:
			if (zn_projectiles.integer && zn_localprojectiles.integer)
				ZN_LocalMissile( &cg.predictedPlayerState, 2000.0, cg.weapon_num );
			break;
		}
	}
}

