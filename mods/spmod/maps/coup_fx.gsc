main()
{
	level._effect["firelp_med_pm"]					= loadfx ("fire/firelp_med_pm");	
	level._effect["firelp_small_pm"]				= loadfx ("fire/firelp_small_pm");
	level._effect["firelp_small_pm_a"]				= loadfx ("fire/firelp_small_pm_a");
	level._effect["dust_wind_slow"]					= loadfx ("dust/dust_wind_slow_yel_loop");
	level._effect["dust_wind_spiral"]				= loadfx ("dust/dust_spiral_runner");
	level._effect["hawk"]							= loadfx ("weather/hawk");
	level._effect["birds_takeoff"]					= loadfx ("misc/birds_takeoff_coup");
	level._effect["bird_seagull_flock_large"]		= loadfx ("misc/bird_seagull_flock_large");
	level._effect["wavebreak_runner"]				= loadfx ("misc/wavebreak_runner");
	level._effect["execution_muzzleflash"]          = loadfx ("muzzleflashes/execution_flash_view");
	level._effect["execution_shell_eject"]          = loadfx ("shellejects/execution_pistol");
	level._effect["ak47_muzzleflash"]		        = loadfx ("muzzleflashes/ak47_flash_wv");
	level._effect["ak47_shelleject"]		        = loadfx ("muzzleflashes/ak47_flash_wv");
	level._effect["car_interior"]					= loadfx ("misc/coup_car_interior");

	maps\createfx\coup_fx::main();
}