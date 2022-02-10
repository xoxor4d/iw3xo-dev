main()
{
    precacheModel( "com_powerline_tower_destroyed" );
    precacheModel( "com_flashlight_on" );
    precacheModel( "weapon_parabolic_knife" );
    precacheModel( "com_spray_can01" );
    precacheModel( "prop_flex_cuff" );
    precacheModel( "prop_flex_cuff_obj" );
    precacheModel( "com_folding_chair" );
	
	maps\mp\_load::main();
	maps\icbm_fx::main();

	setExpFog(0, 1819.93, 0.566786, 0.495098, 0.45532, 0);
	VisionSetNaked( "icbm", 0 );
}