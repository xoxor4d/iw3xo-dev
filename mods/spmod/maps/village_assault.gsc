main()
{
	maps\mp\_load::main();
	maps\village_assault_fx::main();

	setExpFog(500, 5500, .05, .08, 0.13, 0);
	VisionSetNaked( "village_assault", 0 );
}