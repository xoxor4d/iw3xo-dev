main()
{
	maps\mp\_load::main();
	maps\ambush_fx::main();

	setExpFog(2000, 5500, .462618, .478346, .455313, 0);
	VisionSetNaked( "ambush", 0 );
}