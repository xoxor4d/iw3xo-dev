main()
{
	/* -- -- -- -- -- -- -- -- -- -- -- - 
	PARTICLE EFFECTS
	 -- -- -- -- -- -- -- -- -- -- -- -- -*/ 	
	level._effect[ "water_stop" ]						 = loadfx( "misc/parabolic_water_stand" );
	level._effect[ "water_movement" ]					 = loadfx( "misc/parabolic_water_movement" );
	level._effect[ "spotlight" ]						 = loadfx( "misc/flashlight_spotlight" );
	level._effect[ "flashlight" ]						 = loadfx( "misc/flashlight" );
	level._effect[ "pistol_muzzleflash" ]				 = loadfx( "muzzleflashes/pistolflash" );
	
	//level._effect[ "knife_stab" ]						 = loadfx( "misc/parabolic_knife_stab" );
	
	// Ambient
	level._effect[ "firelp_med_pm" ]					 = loadfx( "fire/firelp_med_pm" );
 	level._effect[ "fog_river_200" ]					 = loadfx( "weather/fog_river_200" );
 	level._effect[ "insects_firefly_a" ]				 = loadfx( "misc/insects_firefly_a" );
	level._effect[ "dust_ceiling_ash_small" ]			 = loadfx( "dust/dust_ceiling_ash_small" );	
	level._effect[ "light_shaft_dust_med" ]				 = loadfx( "dust/light_shaft_dust_med" );	
	level._effect[ "light_shaft_dust_field" ]			 = loadfx( "dust/light_shaft_dust_field" );	
	level._effect[ "moth_runner" ]						 = loadfx( "misc/moth_runner" );
	level._effect[ "insects_carcass_runner" ]			 = loadfx( "misc/insects_carcass_runner" );
	level._effect[ "hallway_smoke_dark" ]				 = loadfx( "smoke/hallway_smoke_dark" );

	// Fire House
	level._effect[ "lava" ]								 = loadfx( "misc/lava" );	
	level._effect[ "lava_large" ]						 = loadfx( "misc/lava_large" );	
	level._effect[ "lava_a" ]							 = loadfx( "misc/lava_a" );
	level._effect[ "lava_a_large" ]						 = loadfx( "misc/lava_a_large" );	
	level._effect[ "lava_b" ]							 = loadfx( "misc/lava_b" );	
	level._effect[ "lava_c" ]							 = loadfx( "misc/lava_c" );
	level._effect[ "lava_d" ]							 = loadfx( "misc/lava_d" );
	level._effect[ "lava_ash_runner" ]					 = loadfx( "misc/lava_ash_runner" );		
	level._effect[ "village_smolder_slow" ]				 = loadfx( "smoke/village_smolder_slow" );	
	level._effect[ "firelp_small_streak_pm_v" ]			 = loadfx( "fire/firelp_small_streak_pm_v" );
	level._effect[ "firelp_small_streak_pm_h" ]			 = loadfx( "fire/firelp_small_streak_pm_h" );
	
	level._effect[ "mortar" ]							 = loadfx( "explosions/grenadeExp_mud_1" );

	maps\createfx\blackout_fx::main();
	
}