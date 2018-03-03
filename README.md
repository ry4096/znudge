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


## FUTURE PLANS:

This mod can be improved by predicting player movement more accurately.
In this first version, it just assumes players move in a straight line forever
when running, and don't steer in midair when flying. If you have any ideas,
you are more than welcome to send them to me, or to try them out yourself.


## AUTHOR:

I am the quake player, ///y///. You can contact me by email: ry4096@gmail.com

## LICENSE:

GPL. This mod was created by modifying the code provided in
[ID's source release](https://github.com/id-Software/Quake-III-Arena)

## COMPILING:

I had difficulty using the q3lcc and q3asm provided in ID's source release
so I used the ones from [ioquake3](https://github.com/ioquake/ioq3) instead.
Once you have q3lcc and q3asm, make sure they are in the path and then run make.



