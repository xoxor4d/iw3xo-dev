main()
{
	precachemodel( "com_airduct_square" );

	maps\mp\_load::main();
	maps\aftermath_fx::main();

	VisionSetNaked( "aftermath", 0 );
}
