main()
{
	level._effect["firelp_vhc_lrg_pm_farview"]		= loadfx ("fire/firelp_vhc_lrg_pm_farview");
	level._effect["lighthaze"]						= loadfx ("misc/lighthaze"); 
	level._effect["firelp_med_pm"]					= loadfx ("fire/firelp_med_pm");
	level._effect["firelp_small_pm"]				= loadfx ("fire/firelp_small_pm");
	level._effect["firelp_small_pm_a"]				= loadfx ("fire/firelp_small_pm_a");
	level._effect["fog_hunted"]						= loadfx ("weather/fog_hunted");
	level._effect["fog_hunted_a"]					= loadfx ("weather/fog_hunted_a");
	level._effect["bird_pm"]						= loadfx ("misc/bird_pm");
	level._effect["bird_takeoff_pm"]				= loadfx ("misc/bird_takeoff_pm");
	level._effect["leaves"]							= loadfx ("misc/leaves");
	level._effect["leaves_runner"]					= loadfx ("misc/leaves_runner");
	level._effect["leaves_runner_1"]				= loadfx ("misc/leaves_runner_1");
	level._effect["leaves_lp"]						= loadfx ("misc/leaves_lp");
	level._effect["leaves_gl"]						= loadfx ("misc/leaves_gl");
	level._effect["leaves_gl_a"]					= loadfx ("misc/leaves_gl_a");
	level._effect["leaves_gl_b"]					= loadfx ("misc/leaves_gl_b");	
	level._effect["hunted_vl"]						= loadfx ("misc/hunted_vl");	
	level._effect["hunted_vl_sm"]					= loadfx ("misc/hunted_vl_sm");
	level._effect["hunted_vl_od_lrg"]				= loadfx ("misc/hunted_vl_od_lrg");	
	level._effect["hunted_vl_od_lrg_a"]				= loadfx ("misc/hunted_vl_od_lrg_a");	
	level._effect["hunted_vl_od_sml"]				= loadfx ("misc/hunted_vl_od_sml");	
	level._effect["hunted_vl_od_sml_a"]				= loadfx ("misc/hunted_vl_od_sml_a");
	level._effect["hunted_vl_od_dtl_a"]				= loadfx ("misc/hunted_vl_od_dtl_a");
	level._effect["hunted_vl_od_dtl_b"]				= loadfx ("misc/hunted_vl_od_dtl_b");			
	level._effect["mist_hunted_add"]				= loadfx ("weather/mist_hunted_add");	
	level._effect["insects_light_hunted"]			= loadfx ("misc/insects_light_hunted");	
	level._effect["insects_light_hunted_a"]			= loadfx ("misc/insects_light_hunted_a");	
	level._effect["hunted_vl_white_eql"]			= loadfx ("misc/hunted_vl_white_eql");	
	level._effect["hunted_vl_white_eql_flare"]		= loadfx ("misc/hunted_vl_white_eql_flare");
	level._effect["hunted_vl_white_eql_a"]			= loadfx ("misc/hunted_vl_white_eql_a");	
	level._effect["grenadeexp_fuel"]				= loadfx ("explosions/grenadeexp_fuel");
	level._effect["hunted_fel"]						= loadfx ("misc/hunted_fel");	
	level._effect["greenhouse_fog_spot_lit"]		= loadfx ("smoke/greenhouse_fog_spot_lit");	
	level._effect["waterfall_hunted"]				= loadfx ("misc/waterfall_hunted");	
	level._effect["stream_hunted"]					= loadfx ("misc/stream_hunted");
	
	// level script effects
	level._effect["truck_smoke"]					= loadfx ("smoke/car_damage_blacksmoke");
	level._effect["flashlight"]						= loadfx ("misc/flashlight");

	// "hunted light" required zfeather == 1 and r_zfeather is undefined on console.  So, test for != "0".
	if ( getdvarint( "sm_enable" ) && getdvar( "r_zfeather" ) != "0" )
		level._effect["spotlight"]						= loadfx ("misc/hunted_spotlight_model");
	else
		level._effect["spotlight"]						= loadfx ("misc/spotlight_large");

	//gas station destruction
	level._effect["gasstation_explosion"]			= loadfx ("explosions/hunted_gasstation_explosion");
	level._effect["big_explosion"]					= loadfx ("explosions/helicopter_explosion");
	level._effect["small_explosion"]				= loadfx ("explosions/small_vehicle_explosion");
	level._effect["tracer_incoming"]				= loadfx ("misc/tracer_incoming");
	level._effect["gas_pump_fire"]					= loadfx ("fire/gas_pump_fire");
	level._effect["thin_black_smoke_M"]				= loadfx ("smoke/thin_black_smoke_M");
	level._effect["tire_fire_med"]					= loadfx ("fire/tire_fire_med");

	level._effect["heli_dlight_blue"]					= loadfx ("misc/aircraft_light_cockpit_blue");
	level._effect["heli_dlight_red"]					= loadfx ("misc/aircraft_light_cockpit_red");
	level._effect["missile_explosion"]				= loadfx ("explosions/small_vehicle_explosion");

	maps\createfx\hunted_fx::main();
}