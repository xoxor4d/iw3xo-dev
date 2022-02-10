main()
{
	//----------------------------
	// Placed Effects Declarations
	//----------------------------
	level._effect["paper_falling_burning"]				= loadfx( "misc/paper_falling_burning" );
	level._effect["battlefield_smokebank_S"]			= loadfx( "smoke/battlefield_smokebank_S" );
	level._effect["thin_black_smoke_M"]					= loadfx( "smoke/thin_black_smoke_M" );
	level._effect["thin_black_smoke_L"]					= loadfx( "smoke/thin_black_smoke_L" );
	level._effect["dust_wind_slow"]						= loadfx( "dust/dust_wind_slow_yel_loop" );

	
	//-----------------------
	//Mortar effects & sounds
	//-----------------------
	
	level._effect["mortar"]["dirt"]						= loadfx( "explosions/grenadeExp_dirt" );
	level._effect["mortar"]["mud"]						= loadfx( "explosions/grenadeExp_mud" );
	
	level._effect["wall_explosion_small"]				= loadfx( "explosions/wall_explosion_small" );
	level._effect["wall_explosion_grnd"]				= loadfx( "explosions/wall_explosion_grnd" );
	level._effect["wall_explosion_draft"]				= loadfx( "explosions/wall_explosion_draft" );
	level._effect["wall_explosion_round"]				= loadfx( "explosions/wall_explosion_round" );
	level._effect["tank_round_spark"]					= loadfx( "impacts/tank_round_spark" );
	
	level._effect["exploder"]["100"]					= loadfx( "explosions/wall_explosion_small" );
	level._effect["exploder"]["101"]					= loadfx( "explosions/wall_explosion_small" );
	level._effect["exploder"]["102"]					= loadfx( "explosions/wall_explosion_small" );
	level._effect["exploder"]["102"]					= loadfx( "explosions/wall_explosion_draft" );
	level._effect["exploder"]["103"]					= loadfx( "explosions/wall_explosion_small" );
	level._effect["exploder"]["104"]					= loadfx( "explosions/wall_explosion_small" );
	level._effect["exploder"]["105"]					= loadfx( "explosions/wall_explosion_grnd" );
	level._effect["exploder"]["400"]					= loadfx( "explosions/wall_explosion_round" );

	level._effect[ "mg_kill_grenade" ]					= loadfx( "explosions/grenadeExp_wood" );

	level._effect["afterburner"]						= loadfx ("fire/jet_afterburner");
	level._effect[ "contrail" ]							= loadfx( "smoke/jet_contrail" );
	level._effect["abrams_exhaust"]						= loadfx ("distortion/abrams_exhaust");
	
	//End Tank Destruction
	level._effect["t72_explosion"]						= loadfx ("explosions/large_vehicle_explosion");
	level._effect["t72_ammo_breach"]					= loadfx ("explosions/tank_ammo_breach");
	level._effect["t72_ammo_explosion"]					= loadfx ("explosions/t72_ammo_explosion");
	level._effect["firelp_large_pm"]					= loadfx ("fire/firelp_large_pm");

	maps\createfx\bog_b_fx::main();
}