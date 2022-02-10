main()
{
	maps\mp\_load::main();
	level thread maps\launchfacility_b_fx::main();

	setExpFog( 512, 6145, 0.545098, 0.501961, 0.501961, 0 );
	VisionSetNaked( "launchfacility_b", 0 );
}