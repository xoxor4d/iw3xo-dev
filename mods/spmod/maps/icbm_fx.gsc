main()

{
	level._effect["knife_stab"]						= loadfx ("misc/parabolic_knife_stab"); 
	
	level._effect["fog_icbm"]					= loadfx ("weather/fog_icbm");
	level._effect["fog_icbm_a"]					= loadfx ("weather/fog_icbm_a");
	level._effect["fog_icbm_b"]					= loadfx ("weather/fog_icbm_b");
	level._effect["fog_icbm_c"]					= loadfx ("weather/fog_icbm_c");	
	level._effect["leaves_runner_lghtr"]		= loadfx ("misc/leaves_runner_lghtr");
	level._effect["leaves_runner_lghtr_1"]		= loadfx ("misc/leaves_runner_lghtr_1");
	level._effect["insect_trail_runner_icbm"]	= loadfx ("misc/insect_trail_runner_icbm");
	level._effect["cloud_bank"]					= loadfx ("weather/cloud_bank");
	level._effect["cloud_cover"]				= loadfx ("weather/cloud_cover_icbm");
	level._effect["cloud_bank_a"]				= loadfx ("weather/cloud_bank_a");
	level._effect["cloud_bank_far"]				= loadfx ("weather/cloud_bank_far");
	level._effect["moth_runner"]				= loadfx ("misc/moth_runner");
	level._effect["hawks"]						= loadfx ("misc/hawks");
	level._effect["mist_icbm"]					= loadfx ("weather/mist_icbm");	
	level._effect["icbm_vl_int"]				= loadfx ("misc/icbm_vl_int");
	level._effect["icbm_vl_od"]					= loadfx ("misc/icbm_vl_od");
	level._effect["icbm_vl_od_a"]				= loadfx ("misc/icbm_vl_od_a");
	level._effect["icbm_vl_int_wide"]			= loadfx ("misc/icbm_vl_int_wide");
	level._effect["icbm_vl"]					= loadfx ("misc/icbm_vl");
	level._effect["icbm_vl_a"]					= loadfx ("misc/icbm_vl_a");
	level._effect["icbm_vl_b"]					= loadfx ("misc/icbm_vl_b");	
	level._effect["icbm_vl_int_ls"]				= loadfx ("misc/icbm_vl_int_ls");
	level._effect["icbm_dust_int"]				= loadfx ("smoke/icbm_dust_int");	
	level._effect["icbm_smoke_add"]				= loadfx ("smoke/icbm_smoke_add");
	level._effect["icbm_smoke_add_clr"]			= loadfx ("smoke/icbm_smoke_add_clr");
	level._effect["icbm_smoke_add_clr_a"]		= loadfx ("smoke/icbm_smoke_add_clr_a");	
	level._effect["birds_icbm_runner"]			= loadfx ("misc/birds_icbm_runner");
	level._effect["grenade_smoke"]				= loadfx ("props/american_smoke_grenade");	
	level._effect["vehicle_explosion"]			= loadfx ("explosions/large_vehicle_explosion");
	level._effect["snow_light"]					= loadfx ("weather/snow_light");
	level._effect["smoke_geotrail_icbm"]		= loadfx ("smoke/smoke_geotrail_icbm");
	level._effect["icbm_launch"]				= loadfx ("smoke/icbm_launch");
	level._effect["flashlight"]					= loadfx ("misc/flashlight");
	level._effect["powerTower_leg"]				= loadfx ("props/powerTower_leg");
	level._effect["powerTower_crash"]			= loadfx ("dust/powerTower_crash");
	level._effect["powerTower_spark_exp"]		= loadfx ("explosions/powerTower_spark_exp");
	level._effect["icbm_post_light_red"]		= loadfx ("misc/icbm_post_light_red");
	level._effect["hallway_smoke_light"]		= loadfx ("smoke/hallway_smoke_light");

	level._effect["freezespray"]				= loadfx ("props/freezespray");

	maps\createfx\icbm_fx::main();
}