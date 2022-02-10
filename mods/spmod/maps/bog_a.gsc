main()
{
	SetDvar( "r_specularColorScale", "2.42" );
	
	maps\mp\_load::main();
	maps\bog_a_fx::main();
	
	maps\createfx\bog_a_fx::main();

	setExpFog(477.057, 4556.45, 0.544852, 0.394025, 0.221177, 0);
	VisionSetNaked( "bog_a", 0 );
}