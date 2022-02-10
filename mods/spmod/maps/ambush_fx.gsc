main()
{
	level thread precacheFX();
	maps\createfx\ambush_fx::main();
}

precacheFX()
{
	
	level._effect["ambush_vl"]						= loadfx ("misc/ambush_vl");
	level._effect["ambush_vl_far"]					= loadfx ("misc/ambush_vl_far");
	level._effect["amb_dust"]						= loadfx ("smoke/amb_dust");
	level._effect["amb_ash"]						= loadfx ("smoke/amb_ash");
	level._effect["amb_smoke_add"]					= loadfx ("smoke/amb_smoke_add");
	level._effect["amb_smoke_add_1"]				= loadfx ("smoke/amb_smoke_add_1");
	level._effect["amb_smoke_add_1_far"]			= loadfx ("smoke/amb_smoke_add_1_far");
	level._effect["amb_smoke_blend"]				= loadfx ("smoke/amb_smoke_blend");
	level._effect["firelp_small_pm"]				= loadfx ("fire/firelp_small_pm");
	level._effect["firelp_small_pm_a"]				= loadfx ("fire/firelp_small_pm_a");
	level._effect["firelp_small_streak_pm_h"]		= loadfx ("fire/firelp_small_streak_pm_h");
	level._effect["hallways_smoke_light"]			= loadfx ("smoke/hallway_smoke_light");
	level._effect["thin_black_smoke_L"]				= loadfx ("smoke/thin_black_smoke_L");
	level._effect["thin_black_smoke_M"]				= loadfx ("smoke/thin_black_smoke_M");

	// gameplay
	level._effect["mg_nest_expl"]			= loadfx ("explosions/small_vehicle_explosion");
	level._effect["bullet_spark"]			= loadfx ("impacts/large_metalhit_1");
	level._effect["head_fatal"]				= loadfx ("impacts/flesh_hit_head_fatal_exit");
	level._effect["bloodpool"]				= loadfx ("impacts/deathfx_bloodpool_ambush");
}