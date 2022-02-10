main()
{
	level._effect[ "church_roof_exp" ]					= loadfx( "explosions/church_roof_explosion" );	
		
	level._effect[ "dust_wind_leaves_chernobyl" ]		= loadfx( "dust/dust_wind_leaves_chernobyl" );	
	level._effect[ "thin_black_smoke_M" ]				= loadfx( "smoke/thin_black_smoke_M" );
	level._effect[ "thin_black_smoke_L" ]				= loadfx( "smoke/thin_black_smoke_L" );

	level._effect[ "dust_ceiling_ash_large" ]			= loadfx( "dust/dust_ceiling_ash_large" );
	level._effect[ "light_shaft_dust_med" ]				= loadfx( "dust/light_shaft_dust_med" );	
	level._effect[ "light_shaft_dust_large" ]			= loadfx( "dust/light_shaft_dust_large" );	
	level._effect[ "room_dust_200" ]					= loadfx( "dust/room_dust_200" );	
	level._effect[ "room_dust_100" ]					= loadfx( "dust/room_dust_100" );	
	level._effect[ "leaves_fall_gentlewind" ]		 	= loadfx( "misc/leaves_fall_gentlewind" );
	level._effect[ "dust_wind_spiral" ]					= loadfx( "dust/dust_spiral_runner" );

	level._effect[ "bird" ]								= loadfx("misc/bird" );
	level._effect[ "bird_takeoff" ]						= loadfx("misc/bird_takeoff" );

	level._effect[ "water_splash_bodydump" ]			= loadfx("impacts/water_splash_bodydump" );
	
	level._effect[ "dog_bite_blood" ] 					= loadfx("impacts/flesh_hit_body_fatal_exit" );

	level._effect[ "glass_break" ] 						= loadfx( "props/car_glass_large" );


	maps\createfx\scoutsniper_fx::main();
}
