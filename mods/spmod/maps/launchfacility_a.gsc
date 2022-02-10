main()
{
	setdvar( "r_specularcolorscale", "1.2" );
	
	maps\mp\_load::main();
	maps\launchfacility_a_fx::main();
	
	setExpFog( 0, 7562.12, 0.168627, 0.168627, 0.188814, 0 );
	VisionSetNaked( "launchfacility_a", 0 );
}