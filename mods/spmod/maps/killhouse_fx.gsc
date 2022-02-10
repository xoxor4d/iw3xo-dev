main()
{
	level._effect[ "watermelon" ]					 = loadfx( "props/watermelon" );
 	level._effect[ "fog_river_200" ]				 = loadfx( "weather/fog_river_200" );
	level._effect[ "insect_trail_runner" ]			 = loadfx( "misc/insect_trail_runner" );	
	level._effect[ "moth_runner" ]					 = loadfx( "misc/moth_runner" );	
	level._effect[ "insects_carcass_runner" ]		 = loadfx( "misc/insects_carcass_runner" );
	level._effect[ "amb_dust_hangar" ]				 = loadfx( "dust/amb_dust_hangar" );
	level._effect[ "light_shaft_dust_large" ]		 = loadfx( "dust/light_shaft_dust_large" );
	level._effect[ "light_shaft_dust_med" ]			 = loadfx( "dust/light_shaft_dust_med" );
	
	maps\createfx\killhouse_fx::main();
}
