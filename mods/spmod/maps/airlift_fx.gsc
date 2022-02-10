main()
{
	level thread precacheFX();
	maps\createfx\airlift_fx::main();
}

precacheFX()
{
	//tanker trucks
	level._effect[ "tanker_fire" ]					= loadfx ( "fire/firelp_large_pm" );
	
	//misc
	level._effect[ "antiair_runner" ]					= loadfx ( "misc/antiair_runner_night" );	

	
	//smoke
	level._effect[ "battlefield_smokebank_S" ]			= loadfx ( "smoke/battlefield_smokebank_bog_a" );
	level._effect[ "thin_black_smoke_M" ]				= loadfx ( "smoke/thin_black_smoke_M" );
	level._effect[ "thin_black_smoke_L" ]				= loadfx ( "smoke/thin_black_smoke_L" );
	level._effect[ "thin_light_smoke_S" ]				= loadfx ( "smoke/thin_light_smoke_S" );
	level._effect[ "thin_light_smoke_M" ]				= loadfx ( "smoke/thin_light_smoke_M" );
	level._effect[ "thin_light_smoke_L" ]				= loadfx ( "smoke/thin_light_smoke_L" );
	
	//fire
	level._effect[ "tire_fire_med" ]					= loadfx ( "fire/tire_fire_med" );
	level._effect[ "firelp_large_pm" ]					= loadfx ( "fire/firelp_large_pm" );
	level._effect[ "firelp_small_dl_a" ]				= loadfx ( "fire/firelp_small_dl_a" );
	level._effect[ "firelp_small_dl_b" ]				= loadfx ( "fire/firelp_small_dl_b" );
	level._effect[ "firelp_small_dl_c" ]				= loadfx ( "fire/firelp_small_dl_c" );
	level._effect[ "firelp_small_dl_d" ]				= loadfx ( "fire/firelp_small_dl_d" );
	level._effect[ "firelp_vhc_lrg_pm_farview" ]		= loadfx ( "fire/firelp_vhc_lrg_pm_farview" );
	level._effect[ "oil_field_fire" ]					= loadfx ( "fire/oil_field_fire" );
	
	//dust
	level._effect[ "dust_wind_slow" ]					= loadfx ( "dust/dust_wind_slow_yel_loop" );
	level._effect[ "smoke_oilfire_01" ] 				= loadfx ( "smoke/tunnel_smoke_bog_a" );
	level._effect[ "smoke_oilfire_02" ] 				= loadfx ( "smoke/tunnel_smoke_bog_a" );	

	//exploders
	level._effect[ "statue_explosion" ]					= loadfx( "explosions/exp_pack_doorbreach" );
	level._effect[ "statue_impact" ]					= loadfx( "explosions/grenadeExp_concrete_1" );
	level._effect[ "statue_smoke" ] 					= loadfx( "smoke/thin_light_smoke_S" );	
	level._effect[ "gas_station_explosion" ] 			= loadfx( "explosions/tanker_explosion" );	
	level._effect[ "green_car_explosion" ] 				= loadfx( "explosions/small_vehicle_explosion_airlift" );
	level._effect[ "green_car_explosion2" ] 			= loadfx( "explosions/small_vehicle_explosion_airlift" );
	level._effect[ "cobra_crash_01" ] 					= loadfx( "explosions/grenadeExp_concrete_1" );	
	level._effect[ "cobra_crash_03" ] 					= loadfx( "explosions/large_vehicle_explosion" );	
	level._effect[ "cobra_crash_end" ] 					= loadfx( "explosions/large_vehicle_explosion" );	
	level._effect[ "cobra_crash_path_explosion_01" ] 	= loadfx( "explosions/belltower_explosion" );	
	level._effect[ "cobra_crash_path_explosion_02" ] 	= loadfx( "explosions/clusterbomb" );
	level._effect[ "cobra_crash_path_explosion_03" ] 	= loadfx( "explosions/clusterbomb_exp" );
	level._effect[ "cobra_crash_path_explosion_04" ] 	= loadfx( "explosions/exp_pack_hallway" );
	level._effect[ "cobra_crash_path_explosion_05" ] 	= loadfx( "explosions/fuel_med_explosion" );
	level._effect[ "cobra_crash_path_explosion_06" ] 	= loadfx( "explosions/concussion_grenade" );
	
	/*-----------------------
	FX: SCRIPTED SEQUENCES
	-------------------------*/	
	level._effect[ "nuke_explosion" ]	 				= loadfx ( "explosions/nuke_explosion" );
	level._effect[ "nuke_flash" ]	 					= loadfx ( "explosions/nuke_flash" );
	level._effect[ "nuke_dirt_shockwave" ]	 			= loadfx ( "explosions/nuke_dirt_shockwave" );
	level._effect[ "nuke_smoke_fill" ]	 				= loadfx ( "explosions/nuke_smoke_fill" );

	level._effect[ "building_collapse_nuke" ]	 		= loadfx ( "dust/building_collapse_nuke" );

	level._effect[ "nuked_chopper_smoke_trail" ]		= loadfx ("fire/fire_smoke_trail_L");	
	level._effect[ "nuked_chopper_explosion" ] 			= loadfx( "explosions/aerial_explosion" );
	
	level._effect[ "turret_overheat_haze" ]				= loadfx ( "distortion/abrams_exhaust" );
	level._effect[ "turret_overheat_smoke" ]			= loadfx ( "distortion/armored_car_overheat" );
	
	level._effect[ "smoke_green_signal" ] 				= loadfx ( "smoke/signal_smoke_green" );
	level._effect[ "heat_shimmer_door" ]				= loadfx ( "distortion/abrams_exhaust" );
	level._effect[ "headshot" ]							= loadfx ( "impacts/flesh_hit_head_fatal_exit" );
	level._effect[ "heli_dust_default" ] 				= loadfx ( "treadfx/heli_dust_airlift" );	

	level._effect[ "palace_at4" ]						= loadfx( "explosions/wall_explosion_pm_b" );

	//cobra crash
	level._effect[ "smoke_cobra_crash" ] 				= loadfx ( "smoke/thin_black_smoke_M" );	
	level._effect[ "heli_aerial_explosion" ]			= loadfx ("explosions/aerial_explosion");	
	level._effect[ "heli_aerial_explosion_large" ]		= loadfx ("explosions/aerial_explosion_large");	
	level._effect[ "smoke_trail_heli" ]					= loadfx ("fire/fire_smoke_trail_L");	
}