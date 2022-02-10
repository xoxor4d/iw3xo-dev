main()
{
	//sinking sequence fx
	level._effect["sinking_explosion"]					= loadfx ("explosions/cobrapilot_vehicle_explosion"); 
	
	level._effect["sinking_leak_large"]					= loadfx ("misc/cargoship_sinking_leak_large"); 
	level._effect["event_waterleak"]					= loadfx ("misc/cargoship_sinking_leak_med"); 
	level._effect["event_steamleak"]					= loadfx ("misc/cargoship_sinking_steam_leak");
	level._effect["event_sparks"]						= loadfx ("explosions/sparks_d");
	level._effect["sparks_runner"]						= loadfx ("explosions/sparks_runner"); 
	
	level._effect["sinking_waterlevel_center"]			= loadfx ("misc/cargoship_water_noise");
	level._effect["sinking_waterlevel_edge"]			= loadfx ("misc/cargoship_water_noise");
	
	level._effect["escape_waternoise"]					= loadfx ("weather/rain_noise");
	level._effect["escape_waternoise_ud"]				= loadfx ("weather/rain_noise_ud");
	level._effect["escape_waterdrips"]					= loadfx ("misc/cgoshp_drips_a");
	level._effect["escape_water_drip_stairs"]			= loadfx ("misc/water_drip_stairs");
	level._effect["escape_water_gush_stairs"]			= loadfx ("misc/water_gush_stairs");
	level._effect["escape_caustics"]					= loadfx ("misc/caustics");

	//random
	level._effect["vodka_bottle"]						= loadfx ("props/vodka_bottle"); 
	level._effect["coffee_mug"]							= loadfx ("misc/coffee_mug_cargoship"); 
	
	level._effect["cargo_vl_red_thin"]					= loadfx ("misc/cargo_vl_red_thin"); 
	level._effect["cargo_vl_white"]						= loadfx ("misc/cargo_vl_white"); 
	level._effect["cargo_vl_white_soft"]				= loadfx ("misc/cargo_vl_white_soft");
	level._effect["cargo_vl_white_eql"]					= loadfx ("misc/cargo_vl_white_eql");
	level._effect["cargo_vl_white_eql_flare"]			= loadfx ("misc/cargo_vl_white_eql_flare");
	level._effect["cargo_vl_red_lrg"]					= loadfx ("misc/cargo_vl_red_lrg");	
	level._effect["cargo_steam"]						= loadfx ("smoke/cargo_steam");

	//fx for helicopter
	level._effect["heli_spotlight"]						= loadfx ("misc/spotlight_medium_cargoship"); 
	level._effect["spotlight_dlight"]					= loadfx ("misc/spotlight_dlight"); 
	level._effect["cigar_glow"]							= loadfx ("fire/cigar_glow");
	level._effect["cigar_glow_puff"]					= loadfx ("fire/cigar_glow_puff");	
	level._effect["cigar_smoke_puff"]					= loadfx ("smoke/cigarsmoke_puff");
	level._effect["cigar_exhale"]						= loadfx ("smoke/cigarsmoke_exhale");
	level._effect["heli_minigun_shells"]				= loadfx ("shellejects/20mm_cargoship");
	
	//fx for heli interior/exterior lights
	level._effect["aircraft_light_cockpit_red"]			= loadfx ("misc/aircraft_light_cockpit_red_powerfull");
	level._effect["aircraft_light_cockpit_blue"]		= loadfx ("misc/aircraft_light_cockpit_blue");
	level._effect["aircraft_light_red_blink"]			= loadfx ("misc/aircraft_light_red_blink");
	level._effect["aircraft_light_white_blink"]			= loadfx ("misc/aircraft_light_white_blink");
	level._effect["aircraft_light_wingtip_green"]		= loadfx ("misc/aircraft_light_wingtip_green");
	level._effect["aircraft_light_wingtip_red"]			= loadfx ("misc/aircraft_light_wingtip_red");
	 
	//lights
	level._effect["cgoshp_lights_cr"]			= loadfx ("misc/cgoshp_lights_cr");
	level._effect["cgoshp_lights_flr"]			= loadfx ("misc/cgoshp_lights_flr"); 
	level._effect["flashlight"]					= loadfx ("misc/flashlight_cargoship");
 
	//ambient fx
	level._effect["watersplash"]				= loadfx ("misc/cargoship_splash");
	level._effect["cgo_ship_puddle_small"]		= loadfx ("distortion/cgo_ship_puddle_small");
	level._effect["cgo_ship_puddle_large"]		= loadfx ("distortion/cgo_ship_puddle_large");
	level._effect["cgoshp_drips"]			 	= loadfx ("misc/cgoshp_drips");
	level._effect["cgoshp_drips_a"]			 	= loadfx ("misc/cgoshp_drips_a");
	level._effect["rain_noise"]					= loadfx ("weather/rain_noise");
	level._effect["rain_noise_ud"]				= loadfx ("weather/rain_noise_ud");
	level._effect["fire_med_nosmoke"]			= loadfx ("fire/tank_fire_engine");
	level._effect["watersplash_small"]			= loadfx ("misc/watersplash_small");
	level._effect["water_gush"]					= loadfx ("misc/water_gush");
	level._effect["steam"]						= loadfx ("impacts/pipe_steam");

	// Rain
	level._effect["rain_heavy_mist_heli_hack"]	= loadfx ("weather/rain_heavy_mist_heli_hack");
	level._effect["rain_drops_fastrope"]		= loadfx ("weather/rain_drops_fastrope");

	level._effect["rain_heavy_cloudtype"]		= loadfx ("weather/rain_heavy_cloudtype");
	if( getdvarint( "r_zFeather" ) )
	{
		level._effect["rain_10"]	= loadfx ("weather/rain_heavy_mist");
		level._effect["rain_9"]		= loadfx ("weather/rain_heavy_mist");
		level._effect["rain_8"]		= loadfx ("weather/rain_heavy_mist");
		level._effect["rain_7"]		= loadfx ("weather/rain_heavy_mist");
		level._effect["rain_6"]		= loadfx ("weather/rain_heavy_mist");
		level._effect["rain_5"]		= loadfx ("weather/rain_heavy_mist");
		level._effect["rain_4"]		= loadfx ("weather/rain_heavy_mist");
		level._effect["rain_3"]		= loadfx ("weather/rain_heavy_mist");
		level._effect["rain_2"]		= loadfx ("weather/rain_heavy_mist");
		level._effect["rain_1"]		= loadfx ("weather/rain_heavy_mist");	
		level._effect["rain_0"]		= loadfx ("weather/rain_heavy_mist");
	}
	else
	{
		level._effect["rain_10"]	= loadfx ("misc/blank");
		level._effect["rain_9"]		= loadfx ("misc/blank");
		level._effect["rain_8"]		= loadfx ("misc/blank");
		level._effect["rain_7"]		= loadfx ("misc/blank");
		level._effect["rain_6"]		= loadfx ("misc/blank");
		level._effect["rain_5"]		= loadfx ("misc/blank");
		level._effect["rain_4"]		= loadfx ("misc/blank");
		level._effect["rain_3"]		= loadfx ("misc/blank");
		level._effect["rain_2"]		= loadfx ("misc/blank");
		level._effect["rain_1"]		= loadfx ("misc/blank");	
		level._effect["rain_0"]		= loadfx ("misc/blank");
	}
	

	// Thunder & Lightning
	level._effect["lightning"]				= loadfx ("weather/lightning");
	level._effect["lightning_bolt"]			= loadfx ("weather/lightning_bolt");
	level._effect["lightning_bolt_lrg"]		= loadfx ("weather/lightning_bolt_lrg");
}