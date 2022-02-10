main()
{
	level.fogvalue["near"] = 100;
	level.fogvalue["half"] = 15000;	
	level.fogvalue["r"] = 0/256;
	level.fogvalue["g"] = 0/256; 
	level.fogvalue["b"] = 0/256;
	setExpFog (level.fogvalue["near"], level.fogvalue["half"], level.fogvalue["r"], level.fogvalue["g"], level.fogvalue["b"], 0.1);
	
	level.fogvalue["near"] = 100;
	level.fogvalue["half"] = 4000;	
	setExpFog (level.fogvalue["near"], level.fogvalue["half"], level.fogvalue["r"], level.fogvalue["g"], level.fogvalue["b"], 20);
	
	maps\mp\_load::main();
	maps\cargoship_fx::main();
	maps\createfx\cargoship_fx::main();
	
	setDvar("r_specularColorScale", "3");

	VisionSetNaked( "cargoship", 0 );
}