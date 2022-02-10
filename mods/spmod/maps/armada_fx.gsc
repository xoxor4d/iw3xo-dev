main()

{
	level._effect["thin_black_smoke_M"]				= loadfx ("smoke/thin_black_smoke_M");
	level._effect["thin_black_smoke_L"]				= loadfx ("smoke/thin_black_smoke_L");
	level._effect["tire_fire_med"]					= loadfx ("fire/tire_fire_med");
	level._effect["dust_wind_slow"]					= loadfx ("dust/dust_wind_slow_yel_loop");
	level._effect["hawk"]							= loadfx ("weather/hawk");
	level._effect["power_tower_light_red_blink"]	= loadfx ("misc/power_tower_light_red_blink");

	maps\createfx\armada_fx::main();

}