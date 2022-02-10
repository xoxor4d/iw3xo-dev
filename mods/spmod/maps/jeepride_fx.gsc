main()
{
	level._effect["griggs_brains"] 				= loadfx("impacts/flesh_hit_head_fatal_exit");
	//Ambient FX
	level._effect[ "cloud_bank_far" ]			= loadfx( "weather/jeepride_cloud_bank_far" );
	level._effect[ "hawks" ]					= loadfx( "misc/hawks" );
	level._effect[ "birds" ]					= loadfx( "misc/birds_icbm_runner" ); 	
	level._effect[ "mist_icbm" ]				= loadfx( "weather/mist_icbm" );	
	level._effect[ "exp_wall" ]					= loadfx( "props/wallExp_concrete" );

	//Moment FX	 
	level._effect[ "tunnel_column" ]			= loadfx( "props/jeepride_pillars" );
	level._effect[ "tunnel_brace" ]				= loadfx( "props/jeepride_brace" );
	
// I need to blank these out for when I do mymapents	
//	level._effect[ "tunnel_column" ]			= loadfx( "misc/blank" );
//	level._effect[ "tunnel_brace" ]				= loadfx( "misc/blank" );
	
	level._effect[ "tunnelspark" ] 				= loadfx( "misc/jeepride_tunnel_sparks" );
	level._effect[ "tunnelspark_dl" ] 			= loadfx( "misc/jeepride_tunnel_sparks" );
	level._effect[ "tanker_sparker" ] 			= loadfx( "misc/jeepride_tanker_sparks" );

	level._effect[ "tire_deflate" ]				= loadfx( "impacts/jeepride_tire_shot" ); 	

	level._effect[ "truck_busts_pillar" ]						= loadfx( "explosions/wall_explosion_draft" );
	level._effect[ "truck_crash_flame_spectacular" ]			= loadfx( "misc/blank" );
	level._effect[ "truck_crash_flame_spectacular_arial" ]		= loadfx( "misc/blank" );
	level._effect[ "truck_splash" ] 							= loadfx( "explosions/mortarExp_water" );

	level._effect[ "cliff_explode" ]							= loadfx( "misc/blank" );
	level._effect[ "cliff_explode_jeepride" ] 					= loadfx( "explosions/cliff_explode_jeepride" );
	level._effect[ "tanker_explosion" ] 						= loadfx( "explosions/tanker_explosion" );
	level._effect[ "tanker_explosion_groundfire" ]				= loadfx( "explosions/tanker_explosion_groundfire" );
	                                                                                             
//	level._effect[ "bridge_tanker_explode" ] 						= loadfx( "explosions/tanker_explosion" );
	level._effect[ "bridge_tanker_explode" ]							= loadfx( "misc/blank" );
//	level._effect[ "bridge_tanker_flames" ]				= loadfx( "explosions/tanker_explosion_groundfire" );
	level._effect[ "bridge_tanker_flames" ]							= loadfx( "misc/blank" );
	
	                                                                                             
	level._effect[ "rpg_trail" ]				= loadfx( "smoke/smoke_geotrail_rpg" );
	level._effect[ "rpg_flash" ]				= loadfx( "muzzleflashes/at4_flash" );
	level._effect[ "rpg_explode" ]				= loadfx( "explosions/default_explosion" );
	
	level._effect[ "rocket_trail" ]				= loadfx( "smoke/smoke_geotrail_rocket_jeepride" );
	
	level._effect[ "player_explode" ]			= loadfx( "explosions/default_explosion" );

	level._effect[ "bridge_segment" ] 			= loadfx( "explosions/jeepride_bridge_explosion_seg" );
	level._effect[ "bridge_segment_sounder" ] 	= loadfx( "explosions/jeepride_bridge_explosion_seg_s" );
//	level._effect[ "bridge_segment" ]			= loadfx( "misc/blank" );
//	level._effect[ "bridge_segment_sounder" ]			= loadfx( "misc/blank" );
	
	
	level._effect[ "bridge_chunks" ] 			= loadfx( "misc/jeepride_chunk_thrower" );
	level._effect[ "bridge_chunks2" ] 			= loadfx( "misc/jeepride_chunk_thrower2" );
	level._effect[ "bridge_hubcaps" ] 			= loadfx( "misc/jeepride_hubcap_thrower" );
	
	level._effect[ "bridge_tanker_fire" ] 		= loadfx( "fire/jeepride_tanker_fire" );
	level._effect[ "bridge_tire_fire" ] 		= loadfx( "fire/tire_fire_med" );
	level._effect[ "bridge_amb_smoke" ] 		= loadfx( "smoke/amb_smoke_blend" );

	// I placed a bunch of emitters between the player and the heli to act as a frame of reference for motion
	level._effect[ "bridge_floaty_stuff" ] 			= loadfx( "smoke/amb_ash" );

	level._effect[ "bridge_amb_ash" ] 			= loadfx( "smoke/amb_ash" );

	
	if( getdvar( "consoleGame" ) != "true" && getdvarint("drew_notes") < 3 )
	{
		level._effect[ "bridge_crack_smoke" ] 		= loadfx( "misc/blank" );
		level._effect[ "bridge_sidesmoke" ] 		= loadfx( "misc/blank" );
	}
	else
	{
		level._effect[ "bridge_crack_smoke" ] 		= loadfx( "smoke/jeepride_crack_smoke" );
		level._effect[ "bridge_sidesmoke" ] 		= loadfx( "smoke/jeepride_bridge_sidesmoke" );
	}

	level._effect["griggs_pistol"] 				= loadfx("muzzleflashes/desert_eagle_flash_wv");
	level._effect["griggs_saw"] 				= loadfx("muzzleflashes/saw_flash_wv");

	level._effect["bloodpool"] 				= loadfx("impacts/deathfx_bloodpool");
	
	level._effect["smoke_blind"] = loadfx("test/jeepride_smokeblind");
	level._effect["bridge_crash_smoke"] = loadfx("test/jeepride_smokeblind");

}  