#include "cg_local.h"

// Routines for predicting player movement into the future.

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

	ping = cg.snap ? cg.snap->ping : 0;
	nudge = (ping + cg.frametime)/1000.0;

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
			velocity[0] -= velocity[0]*trace.plane.normal[0];
			velocity[1] -= velocity[1]*trace.plane.normal[1];
			velocity[2] -= velocity[2]*trace.plane.normal[2];
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


