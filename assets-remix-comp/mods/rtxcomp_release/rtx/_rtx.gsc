/* Scr_AddMethod("LNR",						(xfunction_t)(PlayerCmd_LNR), 0);
 * Scr_AddMethod("setvelocity",				(xfunction_t)(PlayerCmd_SetVelocity), 0);
 * Scr_AddMethod("sprintButtonPressed",		(xfunction_t)(PlayerCmd_SprintButtonPressed), 0);
 * Scr_AddMethod("leanLeftButtonPressed",	(xfunction_t)(PlayerCmd_LeanLeftButtonPressed), 0);
 * Scr_AddMethod("leanRightButtonPressed", 	(xfunction_t)(PlayerCmd_LeanRightButtonPressed), 0);
 * Scr_AddMethod("reloadButtonPressed",		(xfunction_t)(PlayerCmd_ReloadButtonPressed), 0);
 * Scr_AddMethod("jumpButtonPressed",		(xfunction_t)(PlayerCmd_JumpButtonPressed), 0);
 * Scr_AddMethod("forwardButtonPressed",	(xfunction_t)(PlayerCmd_ForwardButtonPressed), 0);
 * Scr_AddMethod("backButtonPressed",		(xfunction_t)(PlayerCmd_BackButtonPressed), 0);
 * Scr_AddMethod("rightButtonPressed",		(xfunction_t)(PlayerCmd_RightButtonPressed), 0);
 * Scr_AddMethod("leftButtonPressed",		(xfunction_t)(PlayerCmd_LeftButtonPressed), 0); 
 * Scr_AddMethod("checkJump",				(xfunction_t)(PlayerCmd_CheckJump), 0); */

#include maps\mp\gametypes\_hud_util;
#include common_scripts\utility;
//#include maps\_utility_code;
//#include maps\_utility;

main() 
{
	/* if( !isDefined( game[ "allies" ] ) )
		game[ "allies" ] = "marines";
	if( !isDefined( game[ "axis" ] ) )
		game[ "axis" ] = "opfor";

	if( getDvar( "scr_allies" ) != "" )
		game[ "allies" ] = getDvar( "scr_allies" );
	if( getDvar( "scr_axis" ) != "" )
		game[ "axis" ] = getDvar( "scr_axis" ); */

	level.dbgPrints = false;
	level.dev = false;

	rtx\_rtx_setup::precacheAddons();
	rtx\_rtx_setup::serverDvars();
	
	level water_init();
	thread water_rise(300, 20);

	level spawn_poles();
	thread onPlayerConnect();

	flag_init( "global_hint_in_use" );

	makeDvarServerInfo( "ui_hud_hardcore", 1 );
	setDvar( "ui_hud_hardcore", 1 );

	//thread spawn_control_buttons();
}

onPlayerConnect()
{
	while( true )
	{
		level waittill("connected", player);

		if( level.dbgPrints )
            println( " ++ ^2rtx^7::onPlayerConnect ++ -> connected client!" );

		player rtx\_rtx_setup::clientDvars();

		player thread onPlayerSpec();
		player thread onPlayerSpawned();
	}
}

onPlayerSpec()
{
	self endon("disconnect");

	while( true )
	{
		self waittill("joined_spectators");

		if( level.dbgPrints )
            println( " ++ ^2rtx^7::onPlayerSpec ++ -> spectating client!" );
	}
}

onPlayerSpawned()
{
	self endon("disconnect");

	while( true )
	{
		self waittill("spawned_player");
		self thread player_setup_on_spawn();
		wait 0.05;
	}
}

player_setup_on_spawn()
{
	self endon("death");
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	if( level.dbgPrints )
        println( " ++ ^2rtx^7::onPlayerSpawned ++ -> spawned client!" );

	// disable debug lights used for flashlight
	rtxEnableDebugLight(1, 0);
	rtxEnableDebugLight(2, 0);

	self stopLocalSound("suzanne_ambient");
	//self stopLocalSound("suspense");

	self thread intro();

	self thread watch_buttons();
	self thread weaponSetup();
	self thread removePerks();
	self thread unlimitedHealth();
	//self thread movementSounds();

	self thread water_life(5);
	//self thread water_bubbles(5); // not worth it - hard to see

	if (!level.dev)
	{
		self thread wait_for_meteor();
	}

	wait 0.5;
	self playSound("spawn");

	wait 1.0;
	self playLocalSound("suzanne_ambient"); // 1:46

	if (!level.dev)
	{
		wait 4.5;
		level suzanne_init(20);

		//self thread water_handle_below_water(); // wait a bit because the game needs to assign our playermodel first

		self suzanne_hud_init();
		self thread suzanne_hud_fade(1.0);
		self thread suzanne_hud_update_timer();
		self thread suzanne_hud_update_counter();
		self thread suzanne_check_timeout();
		self thread suzanne_on_finish();

		wait 0.5;
		self thread suzanne_hud_timer_fade(1.0); // looks cooler
	}
}

// called from gametypes/rtx
on_player_killed()
{	
	if (!level.dev)
	{
		if( level.dbgPrints )
        println( " ++ ^2rtx^7::on_player_killed" );

		isdef = isDefined(level.suzanne_failed);

		// do not "re-" trigger when timer was up
		if (!isdef || (isdef && !level.suzanne_failed)) {
			self thread suzanne_on_fail(true);
		}
	}
}

wait_for_meteor()
{
	self endon("death");
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	level waittill("meteor_done");

	self thread maps\mp\gametypes\_hud_message::hintMessage("Congratulations!");
	wait 2;
	self thread maps\mp\gametypes\_hud_message_custom::hintMessage("Please go to the roof of the three-story building ...", 7);
	wait 6;
	self thread maps\mp\gametypes\_hud_message::hintMessage("and use the \"buttons\" :)");
	wait 2;
}

water_life(initial_wait)
{
	self endon("death");
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	wait initial_wait;
	self thread water_sound();

	while(1)
	{
		if (rtxIsUnderwater()) {
			PlayFX( level.water_life, self.origin );
		}
		wait 0.5;
	}
}

water_sound()
{
	self endon("death");
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	self.is_underwater = false;
	underwater_timer = 0;

	while(1)
	{
		was_underwater = self.is_underwater;
		self.is_underwater = rtxIsUnderwater();

		if (!was_underwater && self.is_underwater) {
			self playLocalSound("underwater");
		}
		else if (was_underwater && !self.is_underwater) 
		{
			self stopLocalSound("underwater");
			underwater_timer = 0;
		}

		wait 0.05;

		if (self.is_underwater)
		{
			underwater_timer += 0.05;
			if (underwater_timer >= 46.0)
			{
				self stopLocalSound("underwater");
				self playLocalSound("underwater");
				underwater_timer = 0;
			}
		}
	}
}

water_bubbles(initial_wait)
{
	self endon("death");
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	wait initial_wait;

	/* tag = spawn(  "script_model", self.origin + ( 0, 0, 50 ) );
	tag.angles = ( 0, 90, 0 );	
	tag setModel( "tag_origin" );
	tag linkto( self );

	wait 1; // linkto needs wait */
	wait initial_wait;

	while(1)
	{
		if(!isdefined(self) )
		{
			/* if( isdefined(tag)) {
				tag delete();
			} */
			break;
		}

		eye = self getTagOrigin("tag_eye") + (0, 0, 2);
		if (rtxIsUnderwater()) {
			PlayFX( level.water_bubbles, eye);
		}
		
		wait 0.1;
	}
}

water_init()
{
	level.water_plane_initial_height = 40;
	level.water_plane_goal_height = 145;
	level.water_plane = spawn("script_model", (0, 0, level.water_plane_initial_height));
	level.water_plane SetModel("rtx_water_plane");
	level.water_plane.angles = (0, 0, 0);
	level.water_plane_init = true;
}

water_reset()
{
	if (isDefined(level.water_plane_init) && level.water_plane_init) {
		level.water_plane.origin = (0, 0, level.water_plane_initial_height);
	}
}

water_rise(time, delay)
{
	wait delay;
	if (isDefined(level.water_plane_init) && level.water_plane_init)
	{
		water_reset();
		level.water_plane MoveTo( (0, 0, level.water_plane_goal_height), time, 20.0, 20.0 );
	}
}

water_set_height(time, delay, z_height)
{
	wait delay;

	if (isDefined(level.water_plane_init) && level.water_plane_init) {
		level.water_plane MoveTo( (0, 0, z_height), time, time * 0.1, time * 0.1 );
	}
}

// on_player
water_handle_below_water()
{
	self endon("death");
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	self.underwater = false;

	while (1)
	{
		if (isDefined(level.water_plane_init) && level.water_plane_init)
		{
			eye = self getTagOrigin("tag_eye") + (0, 0, 2);
			//self iPrintLn("eye-z:" + eye[2]);

			// transition into water
			if (!self.underwater && level.water_plane.origin[2] > eye[2])
			{
				self iPrintLn("Underwater");
				rtxSetConfig("underwater_fast", 0.0);
				//rtxSetConfig("underwater_slow", 4.0);
				self.underwater = true;
			}
			else if (self.underwater && level.water_plane.origin[2] <= eye[2])
			{
				self iPrintLn("Abovewater");
				rtxResetVarsToLevel(1.0);
				//rtxSetConfig("abovewater_fast", 1.0);
				//rtxSetConfig("abovewater_slow", 4.0);
				self.underwater = false;
			}
		}

		waittillframeend;
	}
}

watch_buttons()
{
	self endon("death");
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	button_state = false;

	while(1)
	{
		wait 0.05;

		if (level.dev)
		{
			if (self fragButtonPressed())
			{
				thread poles_reset();
				thread do_meteor();
				thread spawn_control_buttons();
				wait 10;
			}

			if (self MeleeButtonPressed())
			{
				rtxTransitionDay();
				wait 10;
			}
		}

		/* if (self UseButtonPressed())
		{
			earthquake(0.3, 2.3, self.origin, 3000);
			level spawn_poles_dyn();
			wait 1;
		} */

		/* if (self MeleeButtonPressed())
		{
			//self spawn_pop();
			rtxTransitionDay();
			wait 1;
		} */

		/* if (self fragButtonPressed())
		{
			thread poles_reset();
			thread do_meteor();
			wait 1;
		} */

		/* if (self fragButtonPressed())
		{
			thread water_rise(60, 0);
			wait 1;
		} */

		if (self UseButtonPressed() && !flag("global_hint_in_use"))
		{
			if (!button_state)
			{
				self rtxToggleFlashlight();
				self playSound("rtxflashlight");
				button_state = true;
			}
		}
		else {
			button_state = false;
		}
	}
}

movementSounds()
{
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	while( true )
	{
		wait 0.05;

		if (self checkJump()) // active for 333 frames after PM_Jump_Check = true
		{
			self playSound("move_rtx_jump");
			wait 0.1;
		}
	}
} 

weaponSetup()
{
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	weaponArray = [];
	//weaponArray[0] ="ak47_mp";
	weaponArray[0] ="deserteagle_nv_mp";
	weaponArray[1] ="ak47_octo_mp";

    self TakeAllWeapons();
	wait 0.05;

	for (x = 0; x < weaponArray.size; x++)
	{
		self giveWeapon(weaponArray[x]);
		wait 0.05;
	}

	self GiveWeapon( "ammo_mp" );
	self SwitchToWeapon(weaponArray[1]);
}

removePerks()
{
	self clearPerks();
}

unlimitedHealth()
{
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	self.maxhealth = 9999;
    self.health = self.maxhealth;

    while( true )
    {
		if( self.health < self.maxhealth * 0.75 )
        	self.health = self.maxhealth;
		
		wait 1;
    }
}

// -------
// general hud

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

	self thread maps\mp\gametypes\_hud_message_custom::hintMessage("Shoot all 20 Suzanne's before the time is up!", 6);
	self thread maps\mp\gametypes\_hud_message::resetOnDeath();
	self.black_transition FadeOverTime( 1.5 );
	self.black_transition.alpha = 0;
}


// -------
// suzanne

suzanne_hud_init()
{
	if (!isDefined(self.suzanne_hud))
	{
		self.suzanne_hud = CreateFontString( "default", 1.4 );
		self.suzanne_hud setPoint( "LEFT", "TOP" , -200, 100 );
		self.suzanne_hud.hideWhenInMenu = false;
		self.suzanne_hud.label = &"&&1 / 20 Suzanne's";
	} 
	self.suzanne_hud setValue( 0 );
	self.suzanne_hud.alpha = 0;
	self.suzanne_hud.glowColor = (0.2, 0.3, 0.7);
	self.suzanne_hud.glowAlpha = 0.5;

	if (!isDefined(self.suzanne_hud_timer_min))
	{
		self.suzanne_hud_timer_min = CreateFontString( "default", 1.4 );
		self.suzanne_hud_timer_min setPoint( "LEFT", "TOP" , -200, 120 );
		self.suzanne_hud_timer_min.hideWhenInMenu = false;
		self.suzanne_hud_timer_min.label = &"Time Remaining: &&1";
	} 
	self.suzanne_hud_timer_min setValue( 0 );
	self.suzanne_hud_timer_min.alpha = 0;
	self.suzanne_hud_timer_min.glowColor = (0.2, 0.3, 0.7);
	self.suzanne_hud_timer_min.glowAlpha = 0.5;
	

	if (!isDefined(self.suzanne_hud_timer_sec))
	{
		self.suzanne_hud_timer_sec = CreateFontString( "default", 1.4 );
		self.suzanne_hud_timer_sec setPoint( "LEFT", "TOP" , -100, 120 );
		self.suzanne_hud_timer_sec.hideWhenInMenu = false;
		self.suzanne_hud_timer_sec.label = &" : &&1";
	} 
	self.suzanne_hud_timer_sec setValue( 0 );
	self.suzanne_hud_timer_sec.alpha = 0;
	self.suzanne_hud_timer_sec.glowColor = (0.2, 0.3, 0.7);
	self.suzanne_hud_timer_sec.glowAlpha = 0.5;
	
}

suzanne_hud_glow_color(r, g, b)
{
	if (isDefined(self.suzanne_hud)) {
		self.suzanne_hud.glowColor = (r, g, b);
	}

	if (isDefined(self.suzanne_hud_timer_min)) {
		self.suzanne_hud_timer_min.glowColor = (r, g, b);
	}
	
	if (isDefined(self.suzanne_hud_timer_sec)) {
		self.suzanne_hud_timer_sec.glowColor = (r, g, b);
	}
}

suzanne_hud_fade(goal)
{
	if (isDefined(self.suzanne_hud))
	{
		self.suzanne_hud FadeOverTime( 1.0 );
		self.suzanne_hud.alpha = goal;
	}
}

suzanne_hud_timer_fade(goal)
{
	if (isDefined(self.suzanne_hud_timer_min))
	{
		self.suzanne_hud_timer_min FadeOverTime( 1.0 );
		self.suzanne_hud_timer_min.alpha = goal;
	}
	
	if (isDefined(self.suzanne_hud_timer_sec))
	{
		self.suzanne_hud_timer_sec FadeOverTime( 1.0 );
		self.suzanne_hud_timer_sec.alpha = goal;
	}
}

// on player
suzanne_on_finish()
{
	self endon("death");
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	while (1)
	{
		if (level.suzanne_finished)
		{
			self stopLocalSound("suzanne_ambient");

			self playSound("suzanne_finish");
			wait 1;
			self playSound("suzanne_finish_clap");
			wait 1;

			self takeWeapon("deserteagle_nv_mp");
			wait 0.05;
			self giveWeapon("deserteagle_rtx_mp");
			self giveWeapon( "ammo_mp" );
			wait 0.05;

			self SwitchToWeapon("deserteagle_rtx_mp");
			break;
		}
		
		wait 0.1;
	}
}

// on player
suzanne_on_fail(no_kill_player)
{
	self endon("death");
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	//self IPrintLnBold("FAILED - DIE!");
	self playSound("suzanne_game_over");
	self suzanne_hud_glow_color(1.0, 0.2, 0.0);
	wait 1;

	self suzanne_hud_fade(0.0);
	self suzanne_hud_timer_fade(0.0);
	wait 1;

	level.suzanne_failed = true;
	suzanne_cleanup();

	if (!isDefined(level.suzanne_tries)) {
		level.suzanne_tries = 0;
	}

	level.suzanne_tries++;

	level notify("suzanne_failed");

	if (!isDefined(no_kill_player) || !no_kill_player) {
		self suicide(); // has to be last	
	}
}

suzanne_check_timeout()
{
	self endon("death");
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	level endon("suzanne_over");
	
	if (isDefined(level.suzanne_timer)) {
		wait level.suzanne_timer;
	} else {
		wait 100;
	}

	if (!level.suzanne_finished) {
		self thread suzanne_on_fail();
	}

	//IPrintLnBold("check_timeout was active even tho suzanne_over was notified");
}

suzanne_hud_update_timer()
{
	self endon("death");
	self endon("disconnect");
	self endon("joined_spectators");
	self endon("killed_player");

	//level endon("suzanne_over");

	if (isDefined(self.suzanne_hud_timer_min) && isDefined(self.suzanne_hud_timer_sec))
	{
		blink = false;
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

			if (minutes == 0 && seconds < 10) 
			{
				if (!blink)
				{
					self suzanne_hud_glow_color(0.7, 0.4, 0.1);
					self.suzanne_hud_timer_min.glowAlpha = 0.5;
					self.suzanne_hud_timer_sec.glowAlpha = 0.5;
					blink = true;
				}
				else
				{
					self suzanne_hud_glow_color(1.0, 0.2, 0.0);
					self.suzanne_hud_timer_min.glowAlpha = 1.0;
					self.suzanne_hud_timer_sec.glowAlpha = 1.0;
					blink = false;
				}
			}

    	    // Update HUD with MM:SS format
    	    self.suzanne_hud_timer_min setValue(minutes); //setText(minutesStr + ":" + secondsStr);
			self.suzanne_hud_timer_sec setValue(seconds);
    	    wait 1; // Wait 1 second
		}

		if (level.suzanne_finished)
		{
			self thread suzanne_hud_timer_fade(0.0);
			wait 1;

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
	self endon("joined_spectators");
	self endon("killed_player");

	while (1)
	{
		if (!isDefined(self.suzanne_hud_current) || self.suzanne_hud_current != level.suzanne_shot_count)
		{
			self.suzanne_hud_current = level.suzanne_shot_count;
			self.suzanne_hud SetValue(self.suzanne_hud_current);
		}

		if (level.suzanne_finished)
		{
			self thread suzanne_hud_fade(0.0);
			wait 1;

			self.suzanne_hud destroy();
			self.suzanne_hud = undefined;
			return;
		}

		wait 0.05;
	}
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
		//IPrintLn("Suzanne was damaged caused by player");

		if (!isDefined(self)) {
			return;
		}

		self notify("suz_break");
		self suzanne_break_fx();
		self hide();

		wait 0.5;

		if (!isDefined(self)) {
			return;
		}

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
	//iPrintLn("^1 Suzanne Cleanup");
	self notify("suzanne_over");

	if (isDefined(level.suzanne_objects)) 
	{
		for (i = 0; i < level.suzanne_parallel_objects; i++)
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
suzanne_respawn()
{
	//level endon("suzanne_over");
	level endon("suzanne_failed");

	if (!isDefined(self.suzanne_objects)) {
		level.suzanne_objects = [];
	}

	while (1)
	{
		//iPrintLn("respawn active @ try: " + level.suzanne_tries);

		for (i = 0; i < level.suzanne_parallel_objects; i++)
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
			self notify("suzanne_over");
			thread poles_reset();

			// do daycyle only on very first meteor + spawn buttons
			if (!level.suzanne_finished_once) 
			{
				thread do_meteor();
				thread spawn_control_buttons();
			}
			else {
				thread do_meteor_no_daycycle();
			}
			
			level.suzanne_finished_once = true;
			return;
		}
		
		wait 1;
	}
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
		//IPrintLn("Waiting for hit ..");
		obj thread suzanne_animate_move();
		obj thread suzanne_animate_rotation();
		obj thread suzanne_wait_for_hit();

		return obj;
	} 
	else {
		//IPrintLnBold("Spawn: Failed to Spawn suzanne - undefined");
	}
}

suzanne_break_fx()
{
	if (isDefined(level.suzanne_break_fx)) {
		level.suzanne_break_fx delete();
	}

	level.suzanne_break_fx = spawnFx( level.rtx_pop, self.origin + (0, 0, 20) );
	triggerFx( level.suzanne_break_fx );
}

// on level
suzanne_init(total)
{
	suzanne_cleanup();

	if (!isDefined(level.suzanne_tries)) {
		level.suzanne_tries = 0;
	}

	if (!isDefined(level.suzanne_finished_once)) {
		level.suzanne_finished_once = false;
	}

	level.suzanne_parallel_objects = 6;
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

	level.suzanne_data[counter][0] = (-596, -81, 255 + zofs);
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

	level.suzanne_data[counter][0] = (894, -185, 420 + zofs);
	level.suzanne_data[counter][1] = (0, 90 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (982, 606, 326 + zofs);
	level.suzanne_data[counter][1] = (0, 90 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (-2, 846, 138 + zofs);
	level.suzanne_data[counter][1] = (0, 90 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (26, -776, 136 + zofs);
	level.suzanne_data[counter][1] = (0, 90 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (72, -1109, 370 + zofs);
	level.suzanne_data[counter][1] = (0, 90 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level.suzanne_data[counter][0] = (551, 1350, 468 + zofs);
	level.suzanne_data[counter][1] = (0, 90 + yawoffs, 0);
	level.suzanne_data[counter][2] = false;
	counter++;

	level thread suzanne_respawn();
}


// ------
// poles

poles_reset()
{
	for (i = 0; i < level.poles.size; i++) 
	{
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

spawn_poles()
{
	counter = 0;

	// array of pole data: origin, angles
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

	level.poles = [];

	for (i = 0; i < level.pole_data.size; i++)
	{
		pole = spawn("script_model", level.pole_data[i][0]);
		pole SetModel("rtx_lamp_post_proxy");
		pole.angles = level.pole_data[i][1];
		level.poles[i] = pole;
	}
}


// ------
// meteor

do_meteor()
{
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
	//IPrintLnBold("sound impact!");

	if (isDefined(level.suzanne_break_fx)) {
		level.suzanne_break_fx delete();
	}

	wait 1.25;
	nvexpl_fx = spawnFx(level.rtx_nvlogo, (621, 498, 157));
	//IPrintLnBold("sound expl!");
	
	wait 1.5;
	nvexpl_fx playSound("meteor_explode");
	level.meteor_fx delete();
	level.meteor_fx = undefined;

	earthquake(0.5, 1.8, (621, 498, 157), 5000);
	poles_make_dynamic();

	//IPrintLnBold("trig expl!");
	triggerFx(nvexpl_fx);

	wait 3;
	nvexpl_fx playSound("nvintro");

	wait 1;
	//nvexpl_fx playSound("nvintro");
	rtxResetVarsToLevel(5.0);

	wait 6;
	rtxTransitionDay();

	level.meteor_snd delete();
	level.meteor_snd = undefined;

	thread water_set_height(200, 120, 80);

	level notify("meteor_done");
}

do_meteor_no_daycycle()
{
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
	//IPrintLnBold("sound impact!");

	if (isDefined(level.suzanne_break_fx)) {
		level.suzanne_break_fx delete();
	}

	wait 1.25;
	nvexpl_fx = spawnFx(level.rtx_nvlogo, (621, 498, 157));
	//IPrintLnBold("sound expl!");
	
	wait 1.5;
	nvexpl_fx playSound("meteor_explode");
	level.meteor_fx delete();
	level.meteor_fx = undefined;

	earthquake(0.5, 1.8, (621, 498, 157), 5000);
	poles_make_dynamic();

	//IPrintLnBold("trig expl!");
	triggerFx(nvexpl_fx);

	wait 3;
	nvexpl_fx playSound("nvintro");

	wait 1;
	rtxResetVarsToLevel(5.0);

	wait 5.0;
	level.meteor_snd delete();
	level.meteor_snd = undefined;
}

spawn_control_buttons()
{
	if (!isDefined(level.spawned_buttons) || !level.spawned_buttons)
	{
		level.spawned_buttons = true;

		btn1_pos = (1490, 740, 600);
		level.btn1 = spawn("script_model", btn1_pos);
		level.btn1 SetModel("rtx_marker_81");
		level.btn1.angles = (0, 180 + 90, 0);

		level.btn1_trig = spawn("trigger_radius", btn1_pos, 0, 20, 50);
		level.btn1_trig thread btn1_logic();

		// --

		btn2_pos = (1490, 670, 600);
		level.btn2 = spawn("script_model", btn2_pos);
		level.btn2 SetModel("rtx_marker_82");
		level.btn2.angles = (0, 180 + 90, 0);

		level.btn2_trig = spawn("trigger_radius", btn2_pos, 0, 20, 50);
		level.btn2_trig thread btn2_logic();

		// --

		btn3_pos = (1490, 600, 600);
		level.btn3 = spawn("script_model", btn3_pos);
		level.btn3 SetModel("rtx_marker_83");
		level.btn3.angles = (0, 180 + 90, 0);

		level.btn3_trig = spawn("trigger_radius", btn3_pos, 0, 20, 50);
		level.btn3_trig thread btn3_logic();

		// --

		level.btn4_trig = spawn("trigger_radius", (1795, 440, 587), 0, 20, 50);
		level.btn4_trig thread btn4_logic();


		level.berry_proxy = spawn("script_model", (1490, 600, 800));
		level.berry_proxy SetModel("rtx_marker_84");
	}
}

btn1_logic()
{
	add_hint_string("BTN1", "Press ^9[[{+frag}]]^7 to spawn a meteor");
	while(1)
	{
		self waittill("trigger", player);

		if (!isDefined(level.meteor_fx) && !isDefined(level.meteor_snd)) 
		{
			if (!flag("global_hint_in_use")) {
				player thread display_hint("BTN1");
			}

			if (player fragButtonPressed())
			{
				//rtxResetVarsToLevel(0.0);
				//rtxReloadMapSettings();
				thread poles_reset();
				thread do_meteor_no_daycycle();
			}
		}

		wait 0.05;
	}
}

btn2_logic()
{
	add_hint_string("BTN2", "Press ^9[[{+leanleft}]]^7 to lower the water level and ^9[[{+leanright}]]^7 to raise it.");
	while(1)
	{
		self waittill("trigger", player);

		if (!flag("global_hint_in_use")) {
			player thread display_hint("BTN2");
		}

		if (player leanLeftButtonPressed())
		{
			if (isDefined(level.water_plane_init) && level.water_plane_init) {
				level.water_plane MoveZ( -3, 0.05, 0, 0 );
			}
		}
		else if (player leanRightButtonPressed())
		{
			if (isDefined(level.water_plane_init) && level.water_plane_init) {
				level.water_plane MoveZ( 3, 0.05, 0, 0 );
			}
		}

		wait 0.05;
	}
}

btn3_logic()
{
	transition_time = 30;

	time_of_day_current_index = 0;
	time_of_day_strings = [];
	time_of_day_strings[0] = "sunrise";
	time_of_day_strings[1] = "day";
	time_of_day_strings[2] = "sunset";
	time_of_day_strings[3] = "night";
	time_of_day_strings[4] = "overcast";
	button_state_frag = false;
	button_state_use = false;

	add_hint_string("BTN3", "^9[[{+frag}]]^7 to select a time of day\n^9[[{+activate}]]^7 to start the transition\nUse ^9[[{+leanleft}]]^7 to decrease and ^9[[{+leanright}]]^7 to increase the transition time");
	while(1)
	{
		self waittill("trigger", player);
		triggered_transition = false;

		if (!flag("global_hint_in_use")) {
			player thread display_hint("BTN3");
		}

		if (player fragButtonPressed())
		{
			if (!button_state_frag)
			{
				if (time_of_day_current_index < time_of_day_strings.size - 1) {
					time_of_day_current_index++;
				} else {
					time_of_day_current_index = 0;
				}

				player iPrintLn("Time of Day: " + time_of_day_strings[time_of_day_current_index]);
				button_state_frag = true;
			}
		} else { button_state_frag = false; }

		if (player useButtonPressed())
		{
			if (!button_state_use)
			{
				rtxTimeCycle(time_of_day_strings[time_of_day_current_index], transition_time);
				player iPrintLn("Started transition");
				triggered_transition = true;
				button_state_use = true;
			}
		} else { button_state_use = false; }

		if (player leanLeftButtonPressed())
		{
			if (transition_time >= 2) 
			{
				transition_time -= 1;
				player iPrintLn("Transition Time: " + transition_time);
			}
		}
		else if (player leanRightButtonPressed())
		{
			transition_time += 1;
			player iPrintLn("Transition Time: " + transition_time);
		}

		if (triggered_transition) {
			wait transition_time;
		}

		wait 0.05;
	}
}

btn4_logic()
{
	add_hint_string("BTN4", "^9[[{+frag}]]^7 for a special weapon.");
	while(1)
	{
		self waittill("trigger", player);

		if (!flag("global_hint_in_use")) {
			player thread display_hint("BTN4");
		}

		if (player fragButtonPressed())
		{
			player giveWeapon("rain_blaster_mp");
			player giveWeapon( "ammo_mp" );
			wait 0.05;

			player SwitchToWeapon("rain_blaster_mp");
		}

		wait 0.05;
	}
}

// ------
// util

add_hint_string( name, string )
{
	//assertex( isdefined( level.trigger_hint_string ), "Tried to add a hint string before _load was called." );
	assertEx( isdefined( name ), "Set a name for the hint string. This should be the same as the script_hint on the trigger_hint." );
	assertEx( isdefined( string ), "Set a string for the hint string. This is the string you want to appear when the trigger is hit." );
		
	if (!isDefined(level.trigger_hint_string)) {
		level.trigger_hint_string = [];
	}

	level.trigger_hint_string[name] = string;
}

display_hint( hint )
{
	if (isDefined(level.trigger_hint_string[hint])) {
		self HintPrint(level.trigger_hint_string[hint]);
	}
	else {
		iPrintLnBold("Hintstring: " + hint + " was not defined");
	}
	
}

HintPrint( string )
{
	MYFADEINTIME = 0.55;
	MYFLASHTIME = 0.55;
	MYALPHAHIGH = 1.0;
	MYALPHALOW = 0.4;
	
	flag_waitopen( "global_hint_in_use" );
	flag_set( "global_hint_in_use" );

	self.Hint = createFontString( "default", 1.4 );
	self.Hint.alpha = 0.0;
	self.Hint.x = 10;
	self.Hint.y = 10;
	self.Hint.alignx = "left";
	self.Hint.aligny = "top";
	self.Hint.horzAlign = "left";
	self.Hint.vertAlign = "top";
	self.Hint setText( string );

	self.Hint FadeOverTime( MYFADEINTIME );
	self.Hint.alpha = MYALPHAHIGH;
	HintPrintWait( MYFADEINTIME );

	wait 0.3;

	for ( i = 0; i < 1; i++ )
	{
		self.Hint FadeOverTime( MYFLASHTIME );
		self.Hint.alpha = 0.0;
		HintPrintWait( MYFLASHTIME );

		/* self.Hint FadeOverTime( MYFLASHTIME );
		self.Hint.alpha = MYALPHAHIGH;
		HintPrintWait( MYFLASHTIME ); */
	}
		
	self.Hint Destroy();
	flag_clear( "global_hint_in_use" );
}

HintPrintWait( length )
{
	wait( length );
	return;
}