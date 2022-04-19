## ABOUT:

ZNudge is a client side mod for Quake 3 Arena (TM)
that tries to reduce the effect of lag.
It's essentially just an improved version of cl_timenudge.
It will automatically detect your ping and draw the other players forward
into the future by your ping. You can aim directly at targets
without leading shots, and as long as they don't change direction,
your shots will hit. This mod may be a reinvention of some of the work
by the authors of instaunlagged.


## INSTALLATION:

To install, just copy the [zzznudge_v1.pk3](https://github.com/ry4096/znudge/raw/master/znudge_v1/zzznudge_v1.pk3) file into your baseq3 folder.


## CVARS:

This mod has the following cvars:

\znudge : 0 or 1 : default 1
	If znudge is 0, the normal player rendering code is run.
	If znudge is 1, then the other player positions are nudged into the future.

\zn_drawball : 0 or 1 : default 1
	If not 0, draw a plasma ball at each player's real position.
	If they are changing directions frequently, you may want
	to aim closer to this ball than their projected future position.

\zn_smoothweight : float, 0 through 1 : default .3
	Averages out other player velocities each frame,
	with zn_smoothweight given to the player's actual velocity.
	When set to 1.0 prediction will be accurate if they keep walking
	in a straight line but jumpy when they change directions.
	When set to lower values like 0.1 the jumps from direction changes
	will be more smooth. If set too low, the velocity used for prediction
	will lag behind the real velocity and be less accurate.

\zn_lightning : int, 0 or 1 : default 1
	Make the lightning gun draw perfectly straight ahead regardless of how high
	your ping is. 

\zn_projectiles: int, 0 or 1 : default 1
	Nudge projectiles forward by your ping. Doesn't work as well as I would like
	so it is off by default for now.


\zn_offset: milliseconds : default 0
	Add this number to your ping when applying znudge. This is here
	to account for other sources of delay. Setting this to 30 seems
	to work well but it is 0 by default.


\zn_localprojectiles : int, 0 or 1, default 1
	When \zn_projectiles is 1, your projectile shots
	may not become visible until they are far away from you.
	When \zn_localprojectiles is 1, your client will display
	your projectiles shots immediately, rather than waiting
	for the server to report of them.
	

\zn_localrail : int, 0 or 1, default 1
	When 1, display rail trails immediately when you fire them.


\zn_serverrail : int, 0 or 1, default 1
	When 1, display the delayed rail trails reported by the server,
	after you fire them. With both \zn_localrail and
	\zn_serverrail set to 1, you will see two rail shots,
	for every rail you fire, one instant, and one delayed.

\zn_ping_weight : float, 0 to 1, default .05
	Use this weight to average out the ping values. When set to a number
	close to zero, fluctuations in ping wont affect the nudge so much.
	When set to 1, the nudge is set to the current ping.

\zn_proj_trail : int, 0 or 1, default 1
	When set to 1, draws a rail trail behind projectiles
	to show how much they have been nudged from their original
	position. 


Other cvars you probably don't need to change:

\zn_gravity : integer : default 800
	Set this to whatever the gravity is on the map.
	In future versions this variable may not be needed.


\zn_maxclips : integer : default 5
	This is the number of clipping events that can happen during
	a trajectory prediction. Players often clip against walls and
	floors and their path will continue to be predicted
	up until this many clippings have occurred.

\zn_climbheight : integer : default 20
	Assume that players will be able to walk over or jump over
	obstacles that are this high. Stair height is 18.

\zn_runningspeed : integer : default 320
	Assume that players will not run faster than this speed
	when on the ground.

\zn_localextend : float, default 50
	Local projectiles will live for the duration of your ping,
	plus this amount. Then they will disappear and the nudged projectile
	from the server should take its place.


\zn_localprestep : float, default 50
	Time in milliseconds to advance projectiles by
	the instant they are fired.


\zn_bounce_factor : float, default .65
	Grenades will have their velocity scaled by this amount
	after bouncing.

\zn_stick_speed : float, default 40
	Grenades moving slower than this after bouncing on a floor
	will stick in place.

\zn_plane_up : float, default .2
	The game considers planes with normal vector z-component higher
	than this value to be a floor.

\zn_step_size : float, default .05
	The time step in seconds to use while predicting grenade movement.

\zn_grenade_shift : float, default .2
	This is the shift that makes grenades fire higher than you aim.

\zn_proj_trail_rate : float, default .02
	Time in seconds to wait before drawing a new proj trail.

\zn_proj_trail_life: float, default 3.0
	The lifetime of a proj trail is this number times zn_proj_trail_rate

\zn_gauntlet_effects : integer, default 1
	There was a fun bug that caused gauntlet to look like it was shooting missiles or rails.
	The bug is fixed now, so you can use this option to see it.
	\zn_gauntlet_effects 7 will make it look like gauntlet shoots
	a curtain of rails.


## FUTURE PLANS:

This mod can be improved by predicting player movement more accurately.
In this first version, it just assumes players move in a straight line forever
when running, and don't steer in midair when flying. If you have any ideas,
you are more than welcome to send them to me, or to try them out yourself.

## LICENSE:

GPL. This mod was created by modifying the code provided in
[ID's source release](https://github.com/id-Software/Quake-III-Arena)

## COMPILING:

I had difficulty using the q3lcc and q3asm provided in ID's source release
so I used the ones from [ioquake3](https://github.com/ioquake/ioq3) instead.
Once you have q3lcc and q3asm, make sure they are in the path and then run make.
