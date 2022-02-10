main()
{
	level._effect["saw_sparks"]						= loadfx ("misc/rescuesaw_sparks");
	level._effect["headshot"]						= loadfx ("impacts/flesh_hit_head_fatal_exit");
	level._effect["c4_secondary_explosion_01"]		= loadfx ("explosions/exp_pack_hallway");  																																																																																																																																	
	level._effect["c4_secondary_explosion_02"]	 	= loadfx ("explosions/grenadeExp_metal");
	level._effect["hind_explosion"]					= loadfx ("explosions/helicopter_explosion");
	level._effect["heli_aerial_explosion"]			= loadfx ("explosions/aerial_explosion");
	level._effect["heli_aerial_explosion_large"]	= loadfx ("explosions/aerial_explosion_large");
	level._effect["smoke_trail_heli"]				= loadfx ("fire/fire_smoke_trail_L");
	level._effect["smoke_trail_bmp"]				= loadfx ("smoke/damaged_vehicle_smoke");
	level._effect["tree_fire_fx"][0]				= loadfx ("smoke/thin_black_smoke_L");
	level._effect["tree_fire_fx"][1]				= loadfx ("fire/firelp_large_pm");
	level._effect["tree_fire_fx"][2]				= loadfx ("fire/firelp_large_pm");
	level._effect["tree_fire_fx"][3]				= loadfx ("smoke/smoke_large");
	level._effect["tree_fire_fx"][4]				= loadfx ("fire/firelp_large_pm");
	level._effect["tree_fire_fx"][5]				= loadfx ("fire/firelp_med_pm");
	level._effect["tree_fire_fx"][6]				= loadfx ("smoke/smoke_large");
	level._effect["tree_fire_fx"][7]				= loadfx ("fire/firelp_med_pm");
	level._effect["tree_fire_fx"][8]				= loadfx ("fire/firelp_large_pm");
	level._effect["launchtube_steam"]				= loadfx ("smoke/launchTube_steam");
	level._effect["rappel_objective"]				= loadfx ("misc/ui_pickup_available");

	//ambient fx
	level._effect["smoke_missile_launched_01"] 		= loadfx ("smoke/smoke_launchtubes");
	level._effect["smoke_missile_launched_02"] 		= loadfx ("smoke/smoke_launchtubes");
	level._effect["steam_large_vent"]				= loadfx ("smoke/steam_large_vent");
	level._effect["steam_large_vent_shaft"]			= loadfx ("smoke/steam_large_vent_shaft");
	level._effect["firelp_large_pm"]				= loadfx ("fire/firelp_large_pm");
	level._effect["snow_wind"]						= loadfx("weather/snow_wind");
	level._effect["ground_smoke_launch_a"]			= loadfx("smoke/ground_smoke_launch_a");

	maps\createfx\launchfacility_a_fx::main();
}
