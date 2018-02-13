## ABOUT:

ZNudge is a client side mod that tries to reduce the effect of lag.
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

\zn_gravity : integer : default 800
	Set this to whatever the gravity is on the map.
	In future versions this variable may not be needed.

\zn_drawball : 0 or 1 : default 1
	If not 0, draw a plasma ball at each player's real position.
	If they are changing directions frequently, you may want
	to aim closer to this ball than their projected future position.


## FUTURE PLANS:

This mod can be improved by predicting player movement more accurately.
In this first version, it just assumes players move in a straight line forever
when running, and don't steer in midair when flying. If you have any ideas,
you are more than welcome to send them to me, or to try them out yourself.


## AUTHOR:

I am the quake player, ///y///. You can contact me by email: ry4096@gmail.com

## LICENSE:

GPL. This mod was created by modifying the code provided here:
https://github.com/id-Software/Quake-III-Arena

