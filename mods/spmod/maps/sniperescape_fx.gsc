main()
{
	//Scripted FX
	level._effect[ "bird_pm" ]					 	= loadfx( "misc/bird_pm" );
	level._effect[ "bird_takeoff_pm" ]			 	= loadfx( "misc/bird_takeoff_pm" );
	level._effect[ "blood" ]					 	= loadfx( "impacts/sniper_escape_blood" );
	level._effect[ "tread_burnout" ]			 	= loadfx( "treadfx/tread_burnout_default" );
	level._effect[ "bullet_geo" ]					= loadfx( "smoke/smoke_geotrail_barret" );
	level._effect[ "rocket_geo" ]					= loadfx( "smoke/smoke_geotrail_rocket" );
	level._effect[ "wind_controlled_leaves" ]		= loadfx( "misc/wind_controlled_leaves" );
	level._effect[ "blood_pool" ]					= loadfx( "impacts/deathfx_bloodpool" );
	level._effect[ "ghillie_leaves" ]				= loadfx( "misc/gilli_leaves" );	
	level._effect[ "wall_explosion" ]				= loadfx( "explosions/wall_explosion_sniperescape" );
	level._effect[ "hind_fire" ]					= loadfx( "muzzleflashes/bmp_flash_wv" );
	

	//Ambient FX
	level._effect[ "snow_wind" ]						= loadfx( "weather/snow_wind" );
	level._effect[ "ground_smoke" ]						= loadfx( "smoke/ground_smoke_launch_a" );
	level._effect[ "firelp_med_pm" ]					= loadfx( "fire/firelp_med_pm" );
	level._effect[ "village_smolder_slow" ]				= loadfx( "smoke/village_smolder_slow" );	
	level._effect[ "village_smolder_hall" ]				= loadfx( "smoke/village_smolder_hall" );	
	level._effect[ "firelp_small_streak_pm_v" ]			= loadfx( "fire/firelp_small_streak_pm_v" );
	level._effect[ "firelp_small_streak_pm_h" ]			= loadfx( "fire/firelp_small_streak_pm_h" );
	level._effect[ "fire_wall_50" ]						= loadfx( "fire/fire_wall_50" );
	level._effect[ "lava" ]								= loadfx( "misc/lava" );	
	level._effect[ "lava_large" ]						= loadfx( "misc/lava_large" );	
	level._effect[ "lava_a" ]							= loadfx( "misc/lava_a" );
	level._effect[ "lava_a_large" ]						= loadfx( "misc/lava_a_large" );	
	level._effect[ "lava_b" ]							= loadfx( "misc/lava_b" );	
	level._effect[ "lava_c" ]							= loadfx( "misc/lava_c" );
	level._effect[ "lava_d" ]							= loadfx( "misc/lava_d" );
	level._effect[ "lava_ash_runner" ]					= loadfx( "misc/lava_ash_runner" );		
	
	// main building fx
	level._effect[ "aerial_explosion" ]					 = loadfx( "explosions/aerial_explosion" );
	level._effect[ "window_explosion" ]					 = loadfx( "explosions/window_explosion" );
	level._effect[ "window_rock" ]						 = loadfx( "explosions/window_rock" );
	level._effect[ "window_fire_large" ]				 = loadfx( "fire/window_fire_large" );
	level._effect[ "dust_ceiling_ash_large" ]			 = loadfx( "dust/dust_ceiling_ash_large" );
	level._effect[ "dust_ceiling_ash_large_stairwell" ]	 = loadfx( "dust/dust_ceiling_ash_large_stairwell" );
	level._effect[ "light_shaft_dust_med" ]				 = loadfx( "dust/light_shaft_dust_med" );	
	level._effect[ "light_shaft_dust_large" ]			 = loadfx( "dust/light_shaft_dust_large" );	
	level._effect[ "room_dust_200" ]					 = loadfx( "dust/room_dust_200" );	

	level._effect[ "heli_explosion" ]					= loadfx( "explosions/helicopter_explosion" );
	level._effect[ "aerial_explosion_heli" ]			= loadfx( "explosions/aerial_explosion_heli" );
	level._effect[ "helicopter_crash_dirt" ]			= loadfx( "explosions/helicopter_crash_dirt" );
	level._effect[ "aerial_explosion_large" ]			= loadfx( "explosions/aerial_explosion_large" );
	level._effect[ "detpack_explosion" ]				= loadfx( "explosions/exp_pack_hallway" );
	level._effect[ "heli_missile_launch" ]				= loadfx( "muzzleflashes/cobra_rocket_flash_wv" );
	level._effect[ "heli_engine_smolder" ]				= loadfx( "smoke/heli_engine_smolder" );	
	level._effect[ "fire_trail_heli" ]					= loadfx( "fire/fire_smoke_trail_L" );	
	level._effect[ "smoke_trail_heli" ]					= loadfx( "smoke/smoke_trail_black_heli" );	
	level._effect[ "brick_chunk" ]						= loadfx( "explosions/brick_chunk" );	
	level._effect[ "helicopter_tail_sparks" ]			= loadfx( "misc/helicopter_tail_sparks" );
	level._effect[ "rotor_smash" ]						= loadfx( "misc/rotor_smash" );
	level._effect[ "heli_dirt" ]						= loadfx( "explosions/heli_dirt" );
	level._effect[ "heli_dirt_rear" ]					= loadfx( "explosions/heli_dirt_rear" );
	level._effect[ "heli_rotor_dirt" ]					= loadfx( "explosions/heli_rotor_dirt" );
	level._effect[ "heli_crash_dust" ]					= loadfx( "dust/heli_crash_dust" );

	maps\createfx\sniperescape_fx::main();
}