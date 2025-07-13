main()
{
	maps\mp\mp_crash_fx::main();
	maps\createart\mp_crash_art::main();
	maps\mp\_load::main();
	
	maps\mp\_compass::setupMiniMap("compass_map_mp_crash");
	
	ambientPlay("ambient_armada_ext0_sur");

	game["allies"] = "marines";
	game["axis"] = "opfor";
	game["attackers"] = "allies";
	game["defenders"] = "axis";
	game["allies_soldiertype"] = "desert";
	game["axis_soldiertype"] = "desert";
	
	setdvar( "r_specularcolorscale", "1" );
	
	setdvar("compassmaxrange","1600");

/*	
var = 100;

while(1)
	{
		var = var +10;
		setdvar("compassMaxRange", var);
		if (var >5000)
			var = 100;
		wait .05;
	}
*/
}
