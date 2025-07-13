main() 
{
	maps\mp\gametypes\_globallogic::init();
	maps\mp\gametypes\_callbacksetup::SetupCallbacks();
	maps\mp\gametypes\_globallogic::SetupCallbacks();

	level.callbackPlayerConnect = ::Callback_PlayerConnect;
	level.callbackPlayerKilled = ::Callback_PlayerKilled;

	level.inter_amb_sound = spawn("script_model", (619, 527, 150));
	level.inter_amb_sound hide();
	level.inter_amb_sound playLoopSound("inter_amb");

	rtx\_menus::main();
	rtx\_rtx::main();

	maps\mp\gametypes\_globallogic::registerTimeLimitDvar	( level.gameType, 60, 0, 1440 );
	maps\mp\gametypes\_globallogic::registerScoreLimitDvar	( level.gameType, 20, 0, 5000 );
	maps\mp\gametypes\_globallogic::registerRoundLimitDvar	( level.gameType, 1, 0, 10 );
	maps\mp\gametypes\_globallogic::registerNumLivesDvar	( level.gameType, 0, 0, 10 );

	level.onStartGameType 	= ::onStartGameType;
	level.onSpawnPlayer 	= ::onSpawnPlayer;

	level.allies 	= ::allies;
	level.axis 		= ::axis;
	level.spectator = ::spectator;

	game["dialog"]["gametype"] = "freeforall";
}

giveLoadout()
{
	//assert( isdefined( level.oldschoolLoadout ) );
	//loadout = level.oldschoolLoadout;
	primaryWeapon = "ak47_mp";

	primaryTokens = strtok( primaryWeapon, "_" );
	self.pers["primaryWeapon"] = primaryTokens[0];
	
	self maps\mp\gametypes\_teams::playerModelForWeapon( self.pers["primaryWeapon"] );		
	
	self GiveWeapon( primaryWeapon );
	self giveStartAmmo( primaryWeapon );
	self setSpawnWeapon( primaryWeapon );
	
	// give secondary weapon
	//self GiveWeapon( loadout.secondaryWeapon );
	//self giveStartAmmo( loadout.secondaryWeapon );
	
	self SetActionSlot( 1, "nightvision" );

	{
		self SetActionSlot( 3, "altMode" );
		self SetActionSlot( 4, "" );
	}
}

spawnPlayer() 
{
	self endon("disconnect");
	self endon("joined_spectators");
	self notify("spawned");
	self notify("end_respawn");

	self.spawnTime = getTime();

	//self iPrintLnBold("spawnPlayer");

	resetTimeout();
	self stopShellShock();

	self.pers[ "team" ] = "allies";
	self.team = self.pers[ "team" ];
	self.sessionteam = "allies";
	self.sessionstate = "playing";
	self.spectatorclient = -1;
	self.statusicon = "";

	self.friendlydamage = undefined;
	self.hasSpawned = true;

	spawnpoints = getEntArray( "mp_dm_spawn", "classname" );
	spawnpoint = spawnpoints[ randomInt( spawnpoints.size ) ];

	if( isDefined( spawnpoint ) )
		self spawn( spawnpoint.origin, spawnpoint.angles );
	else
		maps\mp\_utility::error( "NO mp_dm_spawn SPAWNPOINTS IN MAP" );

	self.maxhealth = 100;
	self.health = self.maxhealth;
	self setMoveSpeedScale( 1 );

	[[level.onSpawnPlayer]]();
	
	self maps\mp\gametypes\_missions::playerSpawned();

	//assert( isValidClass( self.class ) );
	//self maps\mp\gametypes\_oldschool::giveLoadout();
	giveLoadout();
	self maps\mp\gametypes\_class::setClass( level.defaultClass );
	
	waittillframeend;
	
	self notify( "spawned_player" );
	level notify( "player_spawn", self );
}

allies() {
	self setTeam( "allies" );
}

axis() {
	self setTeam( "axis" );
}

spectator() {
	self setTeam( "spectator" );
}

setTeam( team ) 
{
	if( self.pers[ "team" ] == team )
		return;

	if( isAlive( self ) )
		self suicide();
	
	self.pers[ "team" ] = team;
	self.team = team;
	self.sessionteam = team;

	self setClientDvars( "g_scriptMainMenu", game[ "menu_ingame_main" ] );
}

dummy() 
{
	waittillframeend;

	if( isDefined( self ) )
		level notify( "connecting", self );
}


initPersStat( dataName )
{
	if( !isDefined( self.pers[dataName] ) )
		self.pers[dataName] = 0;
}

getPersStat( dataName )
{
	return self.pers[dataName];
}

Callback_PlayerConnect() 
{
	thread dummy();

	self.statusicon = "hud_status_connecting";
	self waittill( "begin" );
	self.statusicon = "";
	
	level notify( "connected", self );
	//self iPrintLnBold("Callback_PlayerConnect");
	
	self.killedPlayers = [];
	self.killedPlayersCurrent = [];
	self.killedBy = [];
	self.cur_kill_streak = 0;
	self.cur_death_streak = 0;
	self.hasSpawned = false;
	self.waitingToSpawn = false;
	self.deathCount = 0;

	self.cur_kill_streak = 0;
	self.cur_death_streak = 0;
	self.death_streak = self maps\mp\gametypes\_persistence::statGet( "death_streak" );
	self.kill_streak = self maps\mp\gametypes\_persistence::statGet( "kill_streak" );
	self.lastGrenadeSuicideTime = -1;

	self initPersStat( "score" );
	self.score = self.pers["score"];

	self initPersStat( "deaths" );
	self.deaths = self getPersStat( "deaths" );

	self initPersStat( "suicides" );
	self.suicides = self getPersStat( "suicides" );

	self initPersStat( "kills" );
	self.kills = self getPersStat( "kills" );

	self initPersStat( "headshots" );
	self.headshots = self getPersStat( "headshots" );

	self initPersStat( "assists" );
	self.assists = self getPersStat( "assists" );

	if( !level.splitscreen )
		iPrintLn( &"MP_CONNECTED", self );

	if( game[ "state" ] == "intermission" ) 
	{
		self spawnIntermission();
		return;
	}

	level endon( "intermission" );

	self.pers[ "team" ] = "spectator";
	self.sessionteam = "spectator";

	self.score = 0;
	self.kills = 0;
	self.assists = 0;
	self.deaths = self getEntityNumber();

	//self thread maps\mp\gametypes\_hud_message::initNotifyMessage();
	//level notify( "update_xp_event_state" );
	
	self.joined = true;
	self spawnSpectator();
	self openMenu( game[ "menu_ingame_main" ] );
}

respawn() 
{
	self thread spawnPlayer();
}

waitForTimeOrNotifies( desiredDelay )
{
	startedWaiting = getTime();

	waitedTime = (getTime() - startedWaiting)/1000;
	
	if ( waitedTime < desiredDelay )
	{
		wait desiredDelay - waitedTime;
		return desiredDelay;
	}
	else
	{
		return waitedTime;
	}
}

Callback_PlayerKilled( eInflictor, attacker, iDamage, sMeansOfDeath, sWeapon, vDir, sHitLoc, psOffsetTime, deathAnimDuration ) 
{
	self endon( "spawned" );
	self notify( "death" );
	self notify( "killed_player" );
	
	//self iPrintLnBold("Callback_PlayerKilled");

	if( self.sessionteam == "spectator" )
		return;
		
	if( sHitLoc == "head" && sMeansOfDeath != "MOD_MELEE" )
		sMeansOfDeath = "MOD_HEAD_SHOT";

	maps\mp\gametypes\_spawnlogic::deathOccured(self, attacker);

	if ( !level.inGracePeriod )
	{
		self maps\mp\gametypes\_weapons::dropWeaponForDeath( attacker );
		self maps\mp\gametypes\_weapons::dropOffhand();
	}

	self.sessionstate = "dead";
	self.statusicon = "hud_status_dead";
	
	self thread rtx\_rtx::on_player_killed();

	self allowSpectateTeam( "allies", false );
	self allowSpectateTeam( "axis", false );
	self allowSpectateTeam( "freelook", false );
	self allowSpectateTeam( "none", true );

	self maps\mp\gametypes\_gameobjects::detachUseModels(); // want them detached before we create our corpse
	
	body = self clonePlayer( deathAnimDuration );
	if ( self isOnLadder() || self isMantling() )
		body startRagDoll();
	
	thread maps\mp\gametypes\_globallogic::delayStartRagdoll( body, sHitLoc, vDir, sWeapon, eInflictor, sMeansOfDeath );

	forward = anglesToForward(self.angles);
	forward_offset = (forward * 80.0);
	self setOrigin(self.origin + (0, 0, 50) - (forward_offset));

	self.body = body;
	if ( !isDefined( self.switching_teams ) )
		thread maps\mp\gametypes\_deathicons::addDeathicon( body, self, self.pers["team"], 5.0 );
	
	self.switching_teams = undefined;
	self.joining_team = undefined;
	self.leaving_team = undefined;

	self thread [[level.onPlayerKilled]](eInflictor, attacker, iDamage, sMeansOfDeath, sWeapon, vDir, sHitLoc, psOffsetTime, deathAnimDuration);

	wait ( 0.25 );
	postDeathDelay = waitForTimeOrNotifies( 1.75 );
	self notify ( "death_delay_finished" );
	
	wait .1;
	self respawn();
}

setSpectatorPermissions() 
{
	self allowSpectateTeam( "allies", true );
	self allowSpectateTeam( "axis", true );
	self allowSpectateTeam( "freelook", true );
	self allowSpectateTeam( "none", true );
}

spawnSpectator( origin, angles ) 
{
	self notify( "spawned" );
	self notify( "spectator_spawn" );

	resetTimeout();

	self stopShellShock();

	self.pers[ "team" ] = "spectator";
	self.sessionteam = "spectator";
	self.sessionstate = "spectator";
	self.spectatorclient = -1;
	self.statusicon = "";

	self setSpectatorPermissions();

	if( isDefined( origin ) && isDefined( angles ) )
		self spawn( origin, angles );
	else 
	{
		spawnpoints = getEntArray( "mp_global_intermission", "classname" );
		if( spawnpoints.size > 0 ) 
		{
			spawnpoint = spawnpoints[ randomInt( spawnpoints.size ) ];
			self spawn( spawnpoint.origin, spawnpoint.angles );
		} else {
			self spawn( ( 0, 0, 0 ), ( 0, 0, 0 ) );
		}
	}

	level notify( "spectator_spawn", self );
}

spawnIntermission() 
{
	self notify( "spawned" );

	resetTimeout();

	self stopShellShock();

	self.sessionstate = "intermission";
	self.spectatorclient = -1;

	

	spawnpoints = getEntArray( "mp_global_intermission", "classname" );
	if( spawnpoints.size > 0 ) 
	{
		spawnpoint = spawnpoints[ randomInt( spawnpoints.size ) ];
		self spawn( spawnpoint.origin, spawnpoint.angles );
	} else {
		 self spawn( ( 0, 0, 0 ), ( 0, 0, 0 ) );
	}
}

onStartGameType()
{
	setClientNameMode("auto_change");

	precachestring(&"RTX_OBJ");
	
	maps\mp\gametypes\_globallogic::setObjectiveText( "allies", &"RTX_OBJ" );
	maps\mp\gametypes\_globallogic::setObjectiveText( "axis", &"RTX_OBJ" );

	maps\mp\gametypes\_globallogic::setObjectiveScoreText( "allies", &"RTX_OBJ" );
	maps\mp\gametypes\_globallogic::setObjectiveScoreText( "axis", &"RTX_OBJ" );

	maps\mp\gametypes\_globallogic::setObjectiveHintText( "allies", &"NULL" );
	maps\mp\gametypes\_globallogic::setObjectiveHintText( "axis", &"NULL" );

	level.spawnMins = ( 0, 0, 0 );
	level.spawnMaxs = ( 0, 0, 0 );

	maps\mp\gametypes\_spawnlogic::addSpawnPoints( "allies", "mp_dm_spawn" );
	maps\mp\gametypes\_spawnlogic::addSpawnPoints( "axis", "mp_dm_spawn" );

	level.mapCenter = maps\mp\gametypes\_spawnlogic::findBoxCenter( level.spawnMins, level.spawnMaxs );
	setMapCenter( level.mapCenter );

	allowed[0] = "dm";
	maps\mp\gametypes\_gameobjects::main(allowed);

	level.displayRoundEndText = false;
	level.QuickMessageToAll = true;
}

onSpawnPlayer()
{
	spawnPoints = maps\mp\gametypes\_spawnlogic::getTeamSpawnPoints( self.pers["team"] );
	spawnPoint 	= maps\mp\gametypes\_spawnlogic::getSpawnpoint_DM( spawnPoints );

	self spawn( spawnPoint.origin, spawnPoint.angles );
}


onEndGame()
{

}