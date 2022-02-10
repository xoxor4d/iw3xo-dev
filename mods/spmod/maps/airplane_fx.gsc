main()
{
	level._effect[ "headshot1" ]					= loadfx( "impacts/flesh_hit_head_fatal_exit" );	//sprays on wall
	level._effect[ "headshot2" ]					= loadfx( "impacts/flesh_hit_splat_large" );		//chunks
	level._effect[ "headshot3" ]					= loadfx( "impacts/flesh_hit_body_fatal_exit" ); //big spray
	level._effect[ "blood_pool" ]					= loadfx( "impacts/deathfx_bloodpool" );
	level._effect[ "flesh_hit" ]					= loadfx( "impacts/flesh_hit" );
	level._effect[ "blood" ]					 	= loadfx( "impacts/sniper_escape_blood" );
	
	/*-----------------------
	AMBIENT FX
	-------------------------*/	

	//clouds
	level._effect[ "cloud" ] 						= loadfx( "misc/ac130_cloud" );
	level._effect[ "cloud_tunnel" ] 				= loadfx( "weather/cloud_tunnel" );

	/*-----------------------
	EXPLODERS
	-------------------------*/	
	level._effect[ "suitcase_explosion" ]					= loadfx( "explosions/suitcase_explosion" );
	level._effect[ "airplane_explosion" ]					= loadfx( "explosions/tanker_explosion" );
	level._effect[ "player_death_explosion" ]				= loadfx( "explosions/player_death_explosion" );

	level._effect[ "fuselage_explosion1" ]					= loadfx( "explosions/fuselage_explosion1" );
	level._effect[ "fuselage_explosion10" ] 				= loadfx( "explosions/decompression_exp1" );
	level._effect[ "fuselage_explosion_wind_suck" ]			= loadfx( "dust/decompression_exitdoor_dust");
	level._effect[ "fuselage_explosion_cabin_dust1" ]		= loadfx( "dust/decompression_cabin_dust" );

	/*-----------------------
	SCRIPTED FX
	-------------------------*/	
	level._effect[ "fuselage_breach_airleak1" ]		= loadfx( "misc/cargoship_sinking_steam_leak" );
	level._effect[ "fuselage_breach_airleak2" ]		= loadfx( "smoke/decompression_leak" );
	
	level._effect[ "exit_door_dust" ]				= loadfx( "dust/decompression_cabin_dust" );
	level._effect[ "exit_door_wind_suck" ]			= loadfx( "dust/decompression_exitdoor_dust" );
	
	level._effect[ "door_kick_dust" ]				= loadfx( "impacts/large_woodhit" );

	maps\createfx\airplane_fx::main();
}

