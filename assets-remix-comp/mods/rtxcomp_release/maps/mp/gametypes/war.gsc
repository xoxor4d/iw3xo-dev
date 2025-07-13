#include maps\mp\_utility;
#include maps\mp\gametypes\_hud_util;
/*
	War
	Objective: 	Score points for your team by eliminating players on the opposing team
	Map ends:	When one team reaches the score limit, or time limit is reached
	Respawning:	No wait / Near teammates

	Level requirements
	------------------
		Spawnpoints:
			classname		mp_tdm_spawn
			All players spawn from these. The spawnpoint chosen is dependent on the current locations of teammates and enemies
			at the time of spawn. Players generally spawn behind their teammates relative to the direction of enemies.

		Spectator Spawnpoints:
			classname		mp_global_intermission
			Spectators spawn from these and intermission is viewed from these positions.
			Atleast one is required, any more and they are randomly chosen between.

	Level script requirements
	-------------------------
		Team Definitions:
			game["allies"] = "marines";
			game["axis"] = "opfor";
			This sets the nationalities of the teams. Allies can be american, british, or russian. Axis can be german.

		If using minefields or exploders:
			maps\mp\_load::main();

	Optional level script settings
	------------------------------
		Soldier Type and Variation:
			game["american_soldiertype"] = "normandy";
			game["german_soldiertype"] = "normandy";
			This sets what character models are used for each nationality on a particular map.

			Valid settings:
				american_soldiertype	normandy
				british_soldiertype		normandy, africa
				russian_soldiertype		coats, padded
				german_soldiertype		normandy, africa, winterlight, winterdark
*/

/*QUAKED mp_tdm_spawn (0.0 0.0 1.0) (-16 -16 0) (16 16 72)
Players spawn away from enemies and near their team at one of these positions.*/

/*QUAKED mp_tdm_spawn_axis_start (0.5 0.0 1.0) (-16 -16 0) (16 16 72)
Axis players spawn away from enemies and near their team at one of these positions at the start of a round.*/

/*QUAKED mp_tdm_spawn_allies_start (0.0 0.5 1.0) (-16 -16 0) (16 16 72)
Allied players spawn away from enemies and near their team at one of these positions at the start of a round.*/

//preCacheModel ( "axis" );

main()
{
	preCacheItem("deserteagle_rtx_mp");
	preCacheItem("rain_blaster_mp");
	preCacheItem("deserteagle_nv_mp");

	preCacheModel ( "rtx_lamp_post_proxy" );
	preCacheModel ( "rtx_suzanne" );

	//preCacheModel ( "fx_pistol_shell" );
	//PreCacheModel ( "foliage_tree_palm_tall_1" );

	//preCacheModel ( "ak47_sm2_mp" );

	/* precacheString( &"Current Position X:^2 &&1" );
	precacheString( &"Current Position Y:^2 &&1" );
	precacheString( &"Current Position Z:^2 &&1" );

	precacheString( &"Distance:^2 &&1 ^3 2D" );
	precacheString( &"Distance:^2 &&1" ); */

	setdvar( "scr_game_playerwaittime", "0" );

	if(getdvar("mapname") == "mp_background")
		return;
	
	maps\mp\gametypes\_globallogic::init();
	maps\mp\gametypes\_callbacksetup::SetupCallbacks();
	maps\mp\gametypes\_globallogic::SetupCallbacks();

	maps\mp\gametypes\_globallogic::registerTimeLimitDvar( "war", 10, 0, 1440 );
	maps\mp\gametypes\_globallogic::registerScoreLimitDvar( "war", 500, 0, 5000 );
	maps\mp\gametypes\_globallogic::registerRoundLimitDvar( "war", 1, 0, 10 );
	maps\mp\gametypes\_globallogic::registerNumLivesDvar( "war", 0, 0, 10 );

	//level common_scripts\_dump::main();

	level.teamBased = true;
	level.onStartGameType = ::onStartGameType;
	level.onSpawnPlayer = ::onSpawnPlayer;

	level.rtx_pop = loadFx( "rtxcomp/balls_release" );
	level.rtx_pole = loadFx( "rtxcomp/lamp_post" );
	level.rtx_nvlogo = loadFx( "rtxcomp/nvlogo_expl" );
	level.rtx_meteor = loadFx( "rtxcomp/meteor" );

	level.rtx_rain = loadFx( "rtxcomp/rain" );

	thread onPlayerConnect();

	level spawn_poles();
	//level suzanne_init(2);

	game["dialog"]["gametype"] = "team_deathmtch";
}



suzanne_wait_for_hit()
{
	level endon("suzanne_over");

	self setcandamage(true);
	self waittill("damage", amount, attacker );

	// check if damage was caused by a player
	was_player = false;
	players = getEntArray("player", "classname");
	for (i = 0; i < players.size; i++) 
	{
		if (attacker == players[i])
		{
			was_player = true;
			break;
		}
	}

	if (was_player)
	{
		self playSound("suzanne");

		level.suzanne_shot_count++;
		IPrintLn("Suzanne was damaged caused by player");

		self notify("suz_break");
		self spawn_pop();
		self hide();

		wait 0.5;
		self delete();
	}
}

suzanne_animate_rotation()
{
	self endon("suz_break");
	level endon("suzanne_over");

	while (1)
	{
		self RotateVelocity( (00,140,00), 12 );
		wait 11.9;
	}
}

suzanne_animate_move()
{
	self endon("suz_break");
	level endon("suzanne_over");

	while (1)
	{
		self MoveZ( 20, 1, 0.3, 0.3 );
		self waittill("movedone");
		self MoveZ( -20, 1, 0.3, 0.3 );
		wait 1.05;
	}
}

suzanne_cleanup()
{
	iPrintLn("^1 Suzanne Cleanup");
	self notify("suzanne_over");

	if (isDefined(level.suzanne_objects)) 
	{
		for (i = 0; i < 5; i++)
		{
			if (isDefined(level.suzanne_objects[i])) 
			{
				level.suzanne_objects[i] notify("suz_break");
				level.suzanne_objects[i] delete();
				level.suzanne_objects[i] = undefined;
			}
		}
	}
}

// on level
suzanne_init(total)
{
	suzanne_cleanup();

	level.suzanne_spawn_count = 0;
	level.suzanne_shot_count = 0;
	level.suzanne_max_count = total;
	level.suzanne_timer = 100;
	level.suzanne_failed = false;
	level.suzanne_finished = false;

	zofs = 50;
	yawoffs = 90;

	counter = 0;
	level.suzanne_data = [];
	level.suzanne_data[counter][0] = (1745, 490, 580 + zofs);
	level.suzanne_data[counter][1] = (0, 150 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (1724, 1348, 136 + zofs);
	level.suzanne_data[counter][1] = (0, 220 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (373, 1346, 273 + zofs);
	level.suzanne_data[counter][1] = (0, 0 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (860, 280, 140 + zofs);
	level.suzanne_data[counter][1] = (0, 100 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (296, -654, 270 + zofs);
	level.suzanne_data[counter][1] = (0, 90 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (580, -688, 397 + zofs);
	level.suzanne_data[counter][1] = (0, 90 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (1560, -1530, 226 + zofs);
	level.suzanne_data[counter][1] = (0, 150 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (1430, -1910, 66 + zofs);
	level.suzanne_data[counter][1] = (0, 96 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (289, -2032, 135 + zofs);
	level.suzanne_data[counter][1] = (0, 96 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (-334, -260, 111 + zofs);
	level.suzanne_data[counter][1] = (0, 320 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (-681, 444, 234 + zofs);
	level.suzanne_data[counter][1] = (0, 0 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (-872, 1325, 241 + zofs);
	level.suzanne_data[counter][1] = (0, 0 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (111, 2123, 406 + zofs);
	level.suzanne_data[counter][1] = (0, 270 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (36, 1840, 246 + zofs);
	level.suzanne_data[counter][1] = (0, 0 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (86, 511, 136 + zofs);
	level.suzanne_data[counter][1] = (0, 350 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (707, 200, 432 + zofs);
	level.suzanne_data[counter][1] = (0, 90 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (780, -48, 152 + zofs);
	level.suzanne_data[counter][1] = (0, 180 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (1485, -618, 207 + zofs);
	level.suzanne_data[counter][1] = (0, 270 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (-700, 1561, 413 + zofs);
	level.suzanne_data[counter][1] = (0, 320 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (164, 508, 573 + zofs);
	level.suzanne_data[counter][1] = (0, 320 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (-760, -1770, 100 + zofs);
	level.suzanne_data[counter][1] = (0, 20 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (335, -1144, 96 + zofs);
	level.suzanne_data[counter][1] = (0, 20 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (1609, 592, 308 + zofs);
	level.suzanne_data[counter][1] = (0, 20 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (1653, -398, 68 + zofs);
	level.suzanne_data[counter][1] = (0, 180 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (1306, 0, 152 + zofs);
	level.suzanne_data[counter][1] = (0, 0 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (1663, 640, 140 + zofs);
	level.suzanne_data[counter][1] = (0, 180 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (615, 525, 135 + zofs);
	level.suzanne_data[counter][1] = (0, 270 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (176, 1516, 126 + zofs);
	level.suzanne_data[counter][1] = (0, 270 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (65, 1233, 650 + zofs);
	level.suzanne_data[counter][1] = (0, 320 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (947, -1014, 345 + zofs);
	level.suzanne_data[counter][1] = (0, 90 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (107, -1118, 370 + zofs);
	level.suzanne_data[counter][1] = (0, 90 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level thread suzanne_respawn();
}

suzane_single_spawn(obj)
{
	rand = randomIntRange(0, level.suzanne_data.size - 1);
	
	// loop until we find a new, unique suzanne spawn point
	while (level.suzanne_data[rand][2]) {
		rand = randomIntRange(0, level.suzanne_data.size - 1);
	}

	//IPrintLn("Spawn @ " + level.suzanne_data[rand][0]);

	obj = spawn("script_model", level.suzanne_data[rand][0]);
	obj SetModel("rtx_suzanne");
	obj.angles = level.suzanne_data[rand][1];
	level.suzanne_data[rand][2] = true; // set to used

	if (isDefined(obj))
	{
		IPrintLn("Waiting for hit ..");
		obj thread suzanne_animate_move();
		obj thread suzanne_animate_rotation();
		obj thread suzanne_wait_for_hit();

		return obj;
	} 
	else {
		IPrintLnBold("Spawn: Failed to Spawn suzanne - undefined");
	}
}

// on level
suzanne_respawn()
{
	//level endon("suzanne_over");

	if (!isDefined(self.suzanne_objects)) {
		level.suzanne_objects = [];
	}

	while (1)
	{
		for (i = 0; i < 6; i++)
		{
			if (level.suzanne_spawn_count < level.suzanne_max_count && !isDefined(level.suzanne_objects[i])) 
			{
				level.suzanne_objects[i] = suzane_single_spawn(level.suzanne_objects[i]);
				level.suzanne_spawn_count++;
			}
		}

		if (level.suzanne_shot_count >= level.suzanne_max_count)
		{
			level.suzanne_finished = true;
			//IPrintLnBold("notify suzanne_over");
			self notify("suzanne_over");
			thread poles_reset();
			thread do_meteor();
			return;
		}
		
		wait 1;
	}
}

suzanne_hud_init()
{
	if (!isDefined(self.suzanne_hud))
	{
		self.suzanne_hud = CreateFontString( "default", 1.4 );
		self.suzanne_hud setPoint( "LEFT", "TOP" , -200, 100 );
		self.suzanne_hud.glowColor = (0.2, 0.3, 0.7);
		self.suzanne_hud.glowAlpha = 0.5;
		self.suzanne_hud.hideWhenInMenu = false;
		self.suzanne_hud.label = &"&&1 / 20 Suzanne's";
	} self.suzanne_hud setValue( 0 );

	if (!isDefined(self.suzanne_hud_timer_min))
	{
		self.suzanne_hud_timer_min = CreateFontString( "default", 1.4 );
		self.suzanne_hud_timer_min setPoint( "LEFT", "TOP" , -200, 120 );
		self.suzanne_hud_timer_min.glowColor = (0.2, 0.3, 0.7);
		self.suzanne_hud_timer_min.glowAlpha = 0.5;
		self.suzanne_hud_timer_min.hideWhenInMenu = false;
		self.suzanne_hud_timer_min.label = &"Time Remaining: &&1";
	} self.suzanne_hud_timer_min setValue( 0 );

	if (!isDefined(self.suzanne_hud_timer_sec))
	{
		self.suzanne_hud_timer_sec = CreateFontString( "default", 1.4 );
		self.suzanne_hud_timer_sec setPoint( "LEFT", "TOP" , -100, 120 );
		self.suzanne_hud_timer_sec.glowColor = (0.2, 0.3, 0.7);
		self.suzanne_hud_timer_sec.glowAlpha = 0.5;
		self.suzanne_hud_timer_sec.hideWhenInMenu = false;
		self.suzanne_hud_timer_sec.label = &" : &&1";
	} self.suzanne_hud_timer_sec setValue( 0 );
}

suzanne_check_timeout()
{
	level endon("suzanne_over");
	
	if (isDefined(level.suzanne_timer)) {
		wait level.suzanne_timer;
	} else {
		wait 100;
	}

	if (!level.suzanne_finished)
	{
		self IPrintLnBold("FAILED - DIE!");
		level.suzanne_failed = true;
		self suicide();
		suzanne_cleanup();
	}

	IPrintLnBold("check_timeout was active even tho suzanne_over was notified");
}

suzanne_hud_update_timer()
{
	self endon("death");
	self endon("disconnect");

	//level endon("suzanne_over");

	if (isDefined(self.suzanne_hud_timer_min) && isDefined(self.suzanne_hud_timer_sec))
	{
		for (i = level.suzanne_timer; i >= 0; i--)
    	{
			if (level.suzanne_finished) {
				break;
			}

    	    // Convert seconds to minutes and seconds
    	    minutes = INT(i / 60); // Integer division for minutes
    	    seconds = INT(i % 60); // Remainder for seconds

    	    // Format with leading zeros
			/* if (minutes < 10) minutesStr = "0" + minutes;
			else minutesStr = minutes;

			if (seconds < 10) secondsStr = "0" + seconds;
			else secondsStr = seconds; */

    	    // Update HUD with MM:SS format
    	    self.suzanne_hud_timer_min setValue(minutes); //setText(minutesStr + ":" + secondsStr);
			self.suzanne_hud_timer_sec setValue(seconds);
    	    wait 1; // Wait 1 second
    	}

		if (level.suzanne_finished)
		{
			self.suzanne_hud_timer_min destroy();
			self.suzanne_hud_timer_min = undefined;

			self.suzanne_hud_timer_sec destroy();
			self.suzanne_hud_timer_sec = undefined;
			return;
		}
	}
}

suzanne_hud_update_counter()
{
	self endon("death");
	self endon("disconnect");

	while (1)
	{
		if (!isDefined(self.suzanne_hud_current) || self.suzanne_hud_current != level.suzanne_shot_count)
		{
			self.suzanne_hud_current = level.suzanne_shot_count;
			self.suzanne_hud SetValue(self.suzanne_hud_current);
		}

		if (level.suzanne_finished)
		{
			//self.suzanne_hud setText("");
			self.suzanne_hud destroy();
			self.suzanne_hud = undefined;
			return;
		}

		wait 0.05;
	}
}

// --------------------

spawn_poles()
{
	counter = 0;
	// Array of pole data: origin, angles
	level.pole_data = [];
	level.pole_data[counter][0] = (67, -94, 196 - 60);
	level.pole_data[counter][1] = (0, 33, 0);
	counter++;

	level.pole_data[counter][0] = (718, -287, 194 - 60);
	level.pole_data[counter][1] = (0, 270, 0);
	counter++;

	level.pole_data[counter][0] = (237, 703, 188 - 60);
	level.pole_data[counter][1] = (0, 70, 0);
	counter++;

	level.pole_data[counter][0] = (822, 1099, 191 - 60);
	level.pole_data[counter][1] = (0, 0, 0);
	counter++;

	level.pole_data[counter][0] = (1444, 231, 196 - 60);
	level.pole_data[counter][1] = (0, 270, 0);
	counter++;

	level.pole_data[counter][0] = (-413, -1446, 140 - 60);
	level.pole_data[counter][1] = (0, 0, 0);
	counter++;

	level.pole_data[counter][0] = (-814, -2113, 122);
	level.pole_data[counter][1] = (0, 140, 0);
	counter++;

	level.pole_data[counter][0] = (-740, 508, 236);
	level.pole_data[counter][1] = (0, 360, 0);
	counter++;

	level.pole_data[counter][0] = (-533, 1202, 312 - 60);
	level.pole_data[counter][1] = (0, 90, 0);
	counter++;

	// outside

	level.pole_data[counter][0] = (-12, 2714, 237);
	level.pole_data[counter][1] = (0, 270, 0);
	counter++;

	level.pole_data[counter][0] = (-880, 2710, 232);
	level.pole_data[counter][1] = (0, 270, 0);
	counter++;

	level.pole_data[counter][0] = (-660, 2725, 239);
	level.pole_data[counter][1] = (0, 270, 0);
	counter++;

	level.pole_data[counter][0] = (2266, 769, 139 - 60);
	level.pole_data[counter][1] = (0, 270, 0);
	counter++;

	level.pole_data[counter][0] = (3144, 113, 149 - 60);
	level.pole_data[counter][1] = (0, 270, 0);
	counter++;

	// Initialize array to store pole entities
	level.poles = [];

	// Spawn each pole
	for (i = 0; i < level.pole_data.size; i++)
	{
		pole = spawn("script_model", level.pole_data[i][0]);
		pole SetModel("rtx_lamp_post_proxy");
		pole.angles = level.pole_data[i][1];
		level.poles[i] = pole; // Store in level.poles array
	}

	//self.pole1 = spawn( "script_model", (192, -496, 141) );
	//self.pole1 SetModel( "rtx_lamp_post_proxy" );
	//self.pole1.angles = (0, 90, 0);
}

poles_reset()
{
	for (i = 0; i < level.poles.size; i++) 
	{
		// delete fx if any
		/* if (isDefined(level.poles_fx) && isDefined(level.poles_fx[i])) {
			level.poles_fx[i] delete();
		} */

		// delete poles
		if (isDefined(level.poles[i])) {
			level.poles[i] delete();
		}
	}
	
	// respawn
	spawn_poles();
}

poles_make_dynamic()
{
	// hide all poles and their FX if they exist
	for (i = 0; i < level.poles.size; i++) 
	{
		if (isDefined(level.poles[i])) {
			//level.poles[i] hide(); //delete();
			level.poles[i] physicsLaunch((0, 0, 0), (10, 10, 5));
		}
	}
}

onPlayerConnect()
{
	for(;;)
	{
		level waittill( "connected", player );
		player thread onPlayerSpawned();
	}
}

intro()
{
	if (!isDefined(self.black_transition))
	{
		self.black_transition = newClientHudElem(self);
		self.black_transition.x = 0;
		self.black_transition.y = 0;
		self.black_transition.alignX = "left";		
		self.black_transition.alignY = "top";
		self.black_transition.horzAlign = "fullscreen";
		self.black_transition.vertAlign = "fullscreen";
		self.black_transition setShader("black", 640, 480);
	} self.black_transition.alpha = 1;
	
	wait 0.01;

	self thread maps\mp\gametypes\_hud_message::hintMessage("Shoot all 20 Suzanne's before the time is up!");
	self thread giveWeap();

	self.black_transition FadeOverTime( 1.5 );
	self.black_transition.alpha = 0;
}

onPlayerSpawned()
{
	//self endon("death");
	self endon("disconnect");

	while (1)
	{
		self waittill( "spawned");
		//self IPrintLnBold("Spawned!");

		self thread intro();
		wait 2;

		level suzanne_init(20);
		self playSound("suzanne_ambient"); // 1:40

		// hud element tracking suzanne break count
		self suzanne_hud_init();
		self thread suzanne_hud_update_timer();
		self thread suzanne_hud_update_counter();
		self thread suzanne_check_timeout();
		//self thread rain_on_player();

		//self thread watch_melee();
		//self thread giveWeap();
		self thread watch_buttons();
	}
}

do_meteor()
{
	//rtx_marker_35

	if (isDefined(level.meteor_snd)) {
		level.meteor_snd delete();
	}

	if (isDefined(level.meteor_fx)) {
		level.meteor_fx delete();
	}

	if (isDefined(level.nvexpl_fx)) {
		level.nvexpl_fx delete();
	}

	//IPrintLnBold("Spawned meteor!");

	level.meteor_fx = spawnFx(level.rtx_meteor, (12000, 12000, 5700));
	triggerFx(level.meteor_fx);

	level.meteor_snd = spawn("script_model", (621, 498, 157));
	level.meteor_snd hide();
	level.meteor_snd playSound("meteor_incoming");

	rtxSetConfig("meteor", 10.0);

	wait 8.5;
	level.meteor_snd playSound("airlift_nuke_impact");
	IPrintLnBold("sound impact!");

	if (isDefined(level.suzanne_break_fx)) {
		level.suzanne_break_fx delete();
	}

	wait 1.25;
	nvexpl_fx = spawnFx(level.rtx_nvlogo, (621, 498, 157));
	IPrintLnBold("sound expl!");
	
	wait 1.5;
	nvexpl_fx playSound("meteor_explode");
	level.meteor_fx delete();

	earthquake(0.5, 1.8, (621, 498, 157), 5000);
	poles_make_dynamic();

	//IPrintLnBold("trig expl!");
	triggerFx(nvexpl_fx);

	wait 1;
	nvexpl_fx playSound("nvintro");

	wait 3;
	rtxResetVarsToLevel(6.0);

	wait 6;
	rtxTransitionDay();

	/* level.meteor = spawn("script_model", (0, 0, 200));
	level.meteormeteor SetModel("rtx_marker_64");

	physicsObject physicsLaunch( (0, 0, 0), (-2000, -2000, -2000) ); */
}

watch_buttons()
{
	self endon("death");
	self endon("disconnect");

	button_state = false;

	while(1)
	{
		wait 0.05;

		/* if (self UseButtonPressed())
		{
			earthquake(0.3, 2.3, self.origin, 3000);
			level spawn_poles_dyn();
			wait 1;
		} */

		if (self MeleeButtonPressed())
		{
			//self spawn_pop();
			rtxTransitionDay();
			wait 1;
		}

		if (self fragButtonPressed())
		{
			poles_reset();
			do_meteor();
			wait 1;
		}

		if (self UseButtonPressed())
		{
			if (!button_state)
			{
				self rtxToggleFlashlight();
				self playSound("rtxflashlight");
				//self rtxSetConfig("meteor", 5, 0.0, 0.15);
				//wait 6;
				//self rtxResetVarsToLevel(1.0);

				button_state = true;
			}
		}
		else {
			button_state = false;
		}
	}
}

giveWeap()
{
	wait 0.05;
	//self IPrintLnBold( "^2Setting up Weapons!" );

	self TakeAllWeapons();
	self GiveWeapon( "ak47_mp" );
	//self GiveWeapon( "deserteagle_rtx_mp" );
	self GiveWeapon( "deserteagle_nv_mp" );
	self GiveWeapon( "ammo_mp" );
	wait 0.05;
	//self SwitchToWeapon( "deserteagle_rtx_mp" );
	self SwitchToWeapon( "deserteagle_nv_mp" );
}

spawn_pop()
{
	if (isDefined(level.suzanne_break_fx)) {
		level.suzanne_break_fx delete();
	}

	level.suzanne_break_fx = spawnFx( level.rtx_pop, self.origin + (0, 0, 20) );
	triggerFx( level.suzanne_break_fx );
}

rain_on_player()
{
	self endon("death");
	self endon("disconnect");

	if (isDefined(level.rtx_rain_fx)) {
		level.rtx_rain_fx delete();
	}

	if (isDefined(level.rtx_rain_fx2)) {
		level.rtx_rain_fx2 delete();
	}

	level.rtx_rain_fx = spawnFx(level.rtx_rain, self.origin + (0, 0, 400)); triggerFx(level.rtx_rain_fx);
	level.rtx_rain_fx2 = spawnFx(level.rtx_rain, self.origin + (0, 0, 400)); triggerFx(level.rtx_rain_fx2);
	self IPrintLn("Spawned rain!");

	while (1)
	{
		wait 1;
		level.rtx_rain_fx.origin = self.origin + (0, 0, 400);
		level.rtx_rain_fx.origin2 = self.origin + (50, 50, 450);

		triggerFx(level.rtx_rain_fx);
		triggerFx(level.rtx_rain_fx2);	

		//level.rtx_rain_fx = spawnFx(level.rtx_rain, self.origin + (0, 0, 400)); triggerFx(level.rtx_rain_fx);
		//level.rtx_rain_fx2 = spawnFx(level.rtx_rain, self.origin + (50, 50, 450)); triggerFx(level.rtx_rain_fx2);
	}
}

/* watch_melee()
{
	self endon("death");
	self endon("disconnect");

	while(1)
	{
		wait 0.05;

		if (self MeleeButtonPressed())
		{
			self spawn_pop();
			wait 1;
		}
	}
} */

watch_ads()
{
	self endon("disconnect");

	self.init = false;
	self.inittemp = false;

	self.point1 = false;
	self.point2 = false;

	self.point1temp = false;
	self.point2temp = false;

	while(1)
	{
		wait 0.05;

		if( self.lineMode == "Temp" )
		{
			if( self UseButtonPressed() && !isdefined( self.TempCrate2 ) )
			{
				//if( isdefined( self.TempCrate2 ))
				//	self.TempCrate2 delete();

				if( isdefined( self.TempLineP1 ))
					self.TempLineP1 delete();

				if( isdefined( self.TempLineP2 ))
					self.TempLineP2 delete();

				self IPrintLn( "deleted TempLine Points!" );
				self.inittemp = false;
				self.point1temp = false;
				self.point2temp = false;
				self.debugCurrPos = false;

				self.distHUDCurrPosX setValue( 0 );
				self.distHUDCurrPosY setValue( 0 );
				self.distHUDCurrPosZ setValue( 0 );
				//self.distHUDCurrPos SetText( self.debugCurrPosString );
				//self.distHUDCurrPos ClearAllTextAfterHudelem();

				wait 0.5;

				continue;
			}

			if( self AdsButtonPressed() && !self.inittemp )
			{
				self.inittemp = true;
				self thread spawnTempPoint();
			}

			if( !self AdsButtonPressed() && self.inittemp && !self.point1temp )
			{
				if( isdefined( self.TempCrate2 ) )
				{
					savedOrg = self.TempCrate2.origin;
					self notify( "stopTemp" );
					self.TempCrate2 delete();

					self.TempLineP1 = spawn( "script_model", savedOrg );
					self.TempLineP1 SetModel( "axis" );
					self.TempLineP1.angles = ( self GetPlayerAngles() * ( 0, 1, 0 ) + ( 0, 180, 0 ) );

					self IPrintLn( "Spawned ^1TempLine ^7Point1!" );
					self notify( "stopScale" );
					self setClientDvar( "cg_fovscale", 1 );

					self.point1temp = true;
					self.inittemp = false;
					self.debugCurrPos = false;

					self.distHUDCurrPosX setValue( 0 );
					self.distHUDCurrPosY setValue( 0 );
					self.distHUDCurrPosZ setValue( 0 );
					//self.distHUDCurrPos SetText( self.debugCurrPosString );
					//self.distHUDCurrPos ClearAllTextAfterHudelem();

					continue;
				}
			}

			if( !self AdsButtonPressed() && self.inittemp && self.point1temp && !self.point2temp )
			{
				if( isdefined( self.TempCrate2 ) )
				{
					savedOrg = self.TempCrate2.origin;
					self notify( "stopTemp" );
					self.TempCrate2 delete();

					self.TempLineP2 = spawn( "script_model", savedOrg );
					self.TempLineP2 SetModel( "axis" );
					///self.TempLineP2.angles = ( self GetPlayerAngles() * ( -1, 1, 0 ) + ( 0, 180, 0 ) );

					self.TempLineP2.angles = VectorToAngles( (self.TempLineP1.origin - self.TempLineP2.origin ) );
					self.TempLineP1.angles = VectorToAngles( (self.TempLineP2.origin - self.TempLineP1.origin ) );

					self IPrintLn( "Spawned ^1TempLine ^7Point2!" );
					self notify( "stopScale" );
					self setClientDvar( "cg_fovscale", 1 );

					self.point2temp = true;
					self.debugCurrPos = false;

					self.distHUDCurrPosX setValue( 0 );
					self.distHUDCurrPosY setValue( 0 );
					self.distHUDCurrPosZ setValue( 0 );
					//self.distHUDCurrPos SetText( self.debugCurrPosString );
					//self.distHUDCurrPos ClearAllTextAfterHudelem();
				}
			}
		}

		else if( self.lineMode == "Measure" )
		{
			if( self UseButtonPressed() && !isdefined( self.TempCrate ) )
			{
				//if( isdefined( self.TempCrate ))
				//	self.TempCrate delete();

				if( isdefined( self.Crate1 ))
					self.Crate1 delete();

				if( isdefined( self.Crate2 ))
					self.Crate2 delete();

				self IPrintLn( "deleted Measure Points!" );
				self.init = false;
				self.point1 = false;
				self.point2 = false;

				self.newpos = false;

				self.debugCurrPos = false;

				self.distHUDCurrPosX setValue( 0 );
				self.distHUDCurrPosY setValue( 0 );
				self.distHUDCurrPosZ setValue( 0 );
				//self.distHUDCurrPos SetText( self.debugCurrPosString );
				//self.distHUDCurrPos ClearAllTextAfterHudelem();

				wait 0.5;

				continue;
			}

			if( self AdsButtonPressed() && !self.init )
			{
				self.init = true;
				self thread spawnTempPoint();
			}

			if( !self AdsButtonPressed() && self.init && !self.point1 )
			{
				if( isdefined( self.TempCrate ) )
				{
					savedOrg = self.TempCrate.origin;
					self notify( "stopTemp" );
					self.TempCrate delete();

					self.Crate1 = spawn( "script_model", savedOrg );
					self.Crate1 SetModel( "axis" );
					self.Crate1.angles = ( self GetPlayerAngles() * ( 0, 1, 0 ) + ( 0, 180, 0 ) );

					self IPrintLn( "Spawned ^2Measure ^7Point1!" );
					self notify( "stopScale" );
					self setClientDvar( "cg_fovscale", 1 );

					self.point1 = true;
					self.init = false;

					self.debugCurrPos = false;

					self.distHUDCurrPosX setValue( 0 );
					self.distHUDCurrPosY setValue( 0 );
					self.distHUDCurrPosZ setValue( 0 );
					//self.distHUDCurrPos SetText( self.debugCurrPosString );
					//self.distHUDCurrPos ClearAllTextAfterHudelem();

					continue;
				}

				else
					self IPrintLn( "^1TempPoint1 wasnt defined!" );
			}

			if( !self AdsButtonPressed() && self.init && self.point1 && !self.point2 )
			{
				if( isdefined( self.TempCrate ) )
				{
					savedOrg = self.TempCrate.origin;
					self notify( "stopTemp" );
					self.TempCrate delete();

					self.Crate2 = spawn( "script_model", savedOrg );
					self.Crate2 SetModel( "axis" );

					//self.Crate2.angles = ( self GetPlayerAngles() * ( -1, 1, 0 ) + ( 0, 180, 0 ) );
					self.Crate2.angles = VectorToAngles( (self.Crate1.origin - self.Crate2.origin ) );
					self.Crate1.angles = VectorToAngles( (self.Crate2.origin - self.Crate1.origin ) );

					self IPrintLn( "Spawned ^2Measure ^7Point2!" );
					self notify( "stopScale" );
					self setClientDvar( "cg_fovscale", 1 );

					self.point2 = true;

					self.debugCurrPos = false;

					self.distHUDCurrPosX setValue( 0 );
					self.distHUDCurrPosY setValue( 0 );
					self.distHUDCurrPosZ setValue( 0 );
					//self.distHUDCurrPos SetText( self.debugCurrPosString );
					//self.distHUDCurrPos ClearAllTextAfterHudelem();

					self.newpos = false;
				}

				else
					self IPrintLn( "^1TempPoint2 wasnt defined!" );
			}
		}
	}
}

spawnTempPoint()
{
	self endon( "stopTemp" );

	if( self.lineMode == "Temp" )
	{
		self.TempCrate2 = spawn( "script_model", self.origin );
		self.TempCrate2 SetModel( "fx_pistol_shell" );
	}

	else
	{
		self.TempCrate = spawn( "script_model", self.origin );
		self.TempCrate SetModel( "fx_pistol_shell" );
	}

	//self thread scaleFOV();

	self.debugCurrPos = true;

	while(1)
	{
		wait 0.05;

		if( self getStance() == "prone" )
			origin = self.origin + ( 0, 0, 11 );

		else if( self getStance() == "crouch" )
			origin = self.origin + ( 0, 0, 40 );
		else 
			origin = self.origin + ( 0, 0, 60 );

		//vec = AnglesToForward( self GetPlayerAngles() );
		//start = self getTagOrigin("tag_eye");
		vec = AnglesToForward( self GetPlayerAngles() );
		end = ( vec[0] * 20000, vec[1] * 20000, vec[2] * 20000 );
		//Location = BulletTrace( self GetTagOrigin( "tag_eye" ), self GetTagOrigin("tag_eye") + end, 0, self )[ "position" ];
		//Location = BulletTrace( self getTagOrigin("j_head"), self getTagOrigin("j_head") + end, 0, self )[ "position" ];
		//Location = PhysicsTrace( start, self getTagOrigin("tag_eye") + end );

		Location = PhysicsTrace( origin, origin + end );

		
		//crateScale = Distance( origin, Location) * 0.0022;
		//self.crateScaleHUD = 1 / crateScale;
		


/*
		if( crateScale <= 1 )
			crateSize = 1;
		else if( crateScale >= 4 )
			crateSize = 4;

		crateSize = crateScale;
*/
		if( self.lineMode == "Temp" )
		{
			self.TempCrate2.angles = ( self GetPlayerAngles() * ( 0, 1, 0 ) + ( 0, 180, 0 ) ); // -1 -1 0
			self.TempCrate2.origin = Location + ( 0, 0, 0 );
		}

		else
		{
			self.TempCrate.angles = ( self GetPlayerAngles() * ( 0, 1, 0 ) + ( 0, 180, 0 ) );
			//self.TempCrate.scale = crateSize;
			self.TempCrate.origin = Location + ( 0, 0, 0 );
		}
	}
}

scaleFOV()
{
	self endon( "stopScale" );

	//self thread fovhud();

	while(1)
	{
		wait 0.05;

		if( self.crateScaleHUD <= 0.2 )
		{
			self setClientDvar( "cg_fovscale", 0.2 );
			continue;
		}

		if( self.crateScaleHUD >= 1 )
		{
			self setClientDvar( "cg_fovscale", 1 );
			continue;
		}

		self setClientDvar( "cg_fovscale", self.crateScaleHUD );

		//self IPrintLn( "CrateScale: " +self.crateScaleHUD );
	}
}

fovhud()
{
	self endon( "stopScale" );

	while(1)
	{
		wait 1;
		self IPrintLn( "FovScale: " +self.crateScaleHUD );
	}
}

onStartGameType()
{
	setClientNameMode("auto_change");

	maps\mp\gametypes\_globallogic::setObjectiveText( "allies", &"OBJECTIVES_WAR" );
	maps\mp\gametypes\_globallogic::setObjectiveText( "axis", &"OBJECTIVES_WAR" );
	
	if ( level.splitscreen )
	{
		maps\mp\gametypes\_globallogic::setObjectiveScoreText( "allies", &"OBJECTIVES_WAR" );
		maps\mp\gametypes\_globallogic::setObjectiveScoreText( "axis", &"OBJECTIVES_WAR" );
	}
	else
	{
		maps\mp\gametypes\_globallogic::setObjectiveScoreText( "allies", &"OBJECTIVES_WAR_SCORE" );
		maps\mp\gametypes\_globallogic::setObjectiveScoreText( "axis", &"OBJECTIVES_WAR_SCORE" );
	}
	maps\mp\gametypes\_globallogic::setObjectiveHintText( "allies", &"OBJECTIVES_WAR_HINT" );
	maps\mp\gametypes\_globallogic::setObjectiveHintText( "axis", &"OBJECTIVES_WAR_HINT" );
			
	level.spawnMins = ( 0, 0, 0 );
	level.spawnMaxs = ( 0, 0, 0 );	
	maps\mp\gametypes\_spawnlogic::placeSpawnPoints( "mp_tdm_spawn_allies_start" );
	maps\mp\gametypes\_spawnlogic::placeSpawnPoints( "mp_tdm_spawn_axis_start" );
	maps\mp\gametypes\_spawnlogic::addSpawnPoints( "allies", "mp_tdm_spawn" );
	maps\mp\gametypes\_spawnlogic::addSpawnPoints( "axis", "mp_tdm_spawn" );
	
	level.mapCenter = maps\mp\gametypes\_spawnlogic::findBoxCenter( level.spawnMins, level.spawnMaxs );
	setMapCenter( level.mapCenter );
	
	allowed[0] = "war";
	
	if ( getDvarInt( "scr_oldHardpoints" ) > 0 )
		allowed[1] = "hardpoint";
	
	level.displayRoundEndText = false;
	maps\mp\gametypes\_gameobjects::main(allowed);
	
	// elimination style
	if ( level.roundLimit != 1 && level.numLives )
	{
		level.overrideTeamScore = true;
		level.displayRoundEndText = true;
		level.onEndGame = ::onEndGame;
	}
}

onSpawnPlayer()
{
	self.usingObj = undefined;

	if ( level.inGracePeriod )
	{
		spawnPoints = getentarray("mp_tdm_spawn_" + self.pers["team"] + "_start", "classname");
		
		if ( !spawnPoints.size )
			spawnPoints = getentarray("mp_sab_spawn_" + self.pers["team"] + "_start", "classname");
			
		if ( !spawnPoints.size )
		{
			spawnPoints = maps\mp\gametypes\_spawnlogic::getTeamSpawnPoints( self.pers["team"] );
			spawnPoint = maps\mp\gametypes\_spawnlogic::getSpawnpoint_NearTeam( spawnPoints );
		}
		else
		{
			spawnPoint = maps\mp\gametypes\_spawnlogic::getSpawnpoint_Random( spawnPoints );
		}		
	}
	else
	{
		spawnPoints = maps\mp\gametypes\_spawnlogic::getTeamSpawnPoints( self.pers["team"] );
		spawnPoint = maps\mp\gametypes\_spawnlogic::getSpawnpoint_NearTeam( spawnPoints );
	}
	
	self spawn( spawnPoint.origin, spawnPoint.angles );
	//self thread init_dist();
}


onEndGame( winningTeam )
{
	if ( isdefined( winningTeam ) && (winningTeam == "allies" || winningTeam == "axis") )
		[[level._setTeamScore]]( winningTeam, [[level._getTeamScore]]( winningTeam ) + 1 );	
}