main()
{

	level._effect["player_death_explosion"]						= loadfx ("explosions/player_death_explosion");

	//ambient
	level._effect["hallway_steam_flood"]						= loadfx ("smoke/hallway_steam_flood");
	level._effect["hallway_steam_loop"]							= loadfx ("smoke/hallway_steam_loop");
	level._effect["steam_jet_med"]								= loadfx ("smoke/steam_jet_med");
	level._effect["steam_jet_med_loop"]							= loadfx ("smoke/steam_jet_med_loop");
	level._effect["steam_jet_med_loop_rand"]					= loadfx ("smoke/steam_jet_med_loop_random");
	level._effect["glow_stick_glow_pile"]						= loadfx ("misc/glow_stick_glow_pile");

	level._effect["launchtube_fire"]							= loadfx ("fire/launchtube_fire");
	level._effect["launchtube_smokeloop"]					 	= loadfx ("smoke/launchtube_smokeloop");
	level._effect["launchtube_fire_light"]					 	= loadfx ("misc/launchtube_fire_light");

	level._effect["fog_bog_a"]									= loadfx ("weather/fog_bog_a");
	level._effect["fog_launchb_red"]							= loadfx ("weather/fog_launchb_red");

	level._effect["shower_wall_large"]							= loadfx ("distortion/shower_wall_large");
	level._effect["shower_spray"]								= loadfx ("misc/shower_spray");
	level._effect["shower_steam"]								= loadfx ("misc/shower_steam");
	level._effect["rain_noise"]									= loadfx ("weather/rain_noise");
	level._effect["rain_noise_ud"]								= loadfx ("weather/rain_noise_ud");
	level._effect["light_glow_emergency_mist"]					= loadfx ("misc/light_glow_emergency_mist"); 

	maps\createfx\launchfacility_b_fx::main();
}
