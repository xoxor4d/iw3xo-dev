main()
{
	precacheModel( "com_flashlight_on" );

	setdvar( "r_specularcolorscale", "2.3" );

	maps\mp\_load::main();
	maps\hunted_fx::main();

	setExpFog(512, 6145, 0.132176, 0.192839, 0.238414, 0);
	VisionSetNaked( "hunted", 0 );
}