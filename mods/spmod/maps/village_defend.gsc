main()
{
	maps\mp\_load::main();
	maps\village_defend_fx::main();
	
	setExpFog( 2000, 5500, 0.462745, 0.478431, 0.454902, 0 );
	VisionSetNaked( "village_defend", 0 );
}