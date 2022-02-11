main()
{
	precachemodel( "fx" );
	precachemodel( "viewhands_player_usmc" );
	precachemodel( "weapon_desert_eagle_silver_HR_promo" );
	precachemodel( "com_door_01_handleright" );
	precachemodel( "chicken" );
	precachemodel( "com_cellphone_on" );
	precachemodel( "com_trashcan_metal" );
	precachemodel( "com_spray_can01" );

	maps\mp\_load::main();
	maps\coup_fx::main();

	setExpFog(0, 2610.72, 0.531857, 0.529929, 0.474802, 0);
	VisionSetNaked( "coup", 0 );
}