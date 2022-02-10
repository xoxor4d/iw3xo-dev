main()
{
	level._effect["village_smolder"]			= loadfx ("smoke/village_smolder");
	level._effect["village_smolder_alt"]		= loadfx ("smoke/village_smolder_alt");	
	level._effect["firelp_small_dl"]			= loadfx ("fire/firelp_small_dl");	
	level._effect["insect_trail_runner"]		= loadfx ("misc/insect_trail_runner");	
	level._effect["moth_runner"]				= loadfx ("misc/moth_runner");	
	level._effect["insects_carcass_runner"]		= loadfx ("misc/insects_carcass_runner");
	level._effect["insects_carcass_runner_far"]	= loadfx ("misc/insects_carcass_runner_far");			
	level._effect["lava"]						= loadfx ("misc/lava");	
	level._effect["lava_a"]						= loadfx ("misc/lava_a");
	level._effect["lava_b"]						= loadfx ("misc/lava_b");	
	level._effect["lava_c"]						= loadfx ("misc/lava_c");
	level._effect["lava_d"]						= loadfx ("misc/lava_d");
	level._effect["lava_ash_runner"]			= loadfx ("misc/lava_ash_runner");		
	level._effect["firelp_small_dl_h"]			= loadfx ("fire/firelp_small_dl_h");
	level._effect["firelp_small_dl"]			= loadfx ("fire/firelp_small_dl");		
	level._effect["village_def_vl_sml"]			= loadfx ("misc/village_def_vl_sml");
	level._effect["village_def_vl_sml_a"]		= loadfx ("misc/village_def_vl_sml_a");	
	level._effect["village_vl_sml"]				= loadfx ("misc/village_vl_sml");
	level._effect["village_vl_int"]				= loadfx ("misc/village_vl_int");
	level._effect["village_vl_int_a"]			= loadfx ("misc/village_vl_int_a");	
	level._effect["village_vl_lrg"]				= loadfx ("misc/village_vl_lrg");	
	level._effect["village_cloud_far"]			= loadfx ("weather/village_cloud_far");
	level._effect["icbm_dust_int"]				= loadfx ("smoke/icbm_dust_int");
	level._effect["village_bounce"]				= loadfx ("misc/village_bounce");	
	level._effect["hawks"]						= loadfx ("misc/hawks");
	level._effect["leaves_runner_pine"]			= loadfx ("misc/leaves_runner_pine");
	level._effect["birds_village_runner"]		= loadfx ("misc/birds_village_runner");
	level._effect["birds_village_runner_far"]	= loadfx ("misc/birds_village_runner_far");
	level._effect["sewer_stream_village"]		= loadfx ("distortion/sewer_stream_village");
	level._effect["sewer_stream_village_far"]	= loadfx ("distortion/sewer_stream_village_far");
	
	level._effect["belltower_explosion"]		= loadfx ("explosions/belltower_explosion");
	level._effect["tracer_incoming"]			= loadfx ("misc/tracer_incoming");
	level._effect["killzone_marker"]			= loadfx ("misc/ui_flagbase_gold");
	
	level._effect[ "turret_overheat_haze" ]		= loadfx ( "distortion/abrams_exhaust" );
	level._effect[ "turret_overheat_smoke" ]	= loadfx ( "distortion/armored_car_overheat" );

	maps\createfx\village_defend_fx::main();
}
