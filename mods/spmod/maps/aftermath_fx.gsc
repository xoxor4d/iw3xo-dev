main()
{
	level thread precacheFX();
	maps\createfx\aftermath_fx::main();

}

precacheFX()
{
	level._effect["ash_turb_aftermath"]				= loadfx ("weather/ash_turb_aftermath");
	level._effect["nuke_bg"]						= loadfx ("explosions/nuke_bg");
	
	level._effect["fallout_aftermath"]				= loadfx ("dust/fallout_aftermath");
	level._effect["sand_aftermath"]					= loadfx ("dust/sand_aftermath");
	level._effect["sand_rooftop_aftermath"]			= loadfx ("dust/sand_rooftop_aftermath");	
	level._effect["fire_vhc_dl_aftermath"]			= loadfx ("fire/fire_vhc_dl_aftermath");
	level._effect["sand_spray_dtl_aftermath"]		= loadfx ("dust/sand_spray_dtl_aftermath");	
	level._effect["powerline_runner"]				= loadfx ("explosions/powerline_runner");	
	level._effect["firepatch_vhc_dl_aftermath"]		= loadfx ("fire/firepatch_vhc_dl_aftermath");	
	level._effect["thin_black_smoke_L"]				= loadfx ("smoke/thin_black_smoke_L_fast");	
	level._effect["building_collapse_aftermath"]	= loadfx ("dust/building_collapse_aftermath_runner");	

}