main()
{
	maps\mp\_load::main();
	maps\airlift_fx::main();
	
	setExpFog(400, 6500, 0.678352, 0.511225, 0.372533, 0);
	VisionSetNaked( "airlift", 0 );
}