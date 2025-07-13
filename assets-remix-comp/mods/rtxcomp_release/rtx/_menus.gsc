main() 
{
	game[ "menu_ingame_main" ] = "ingame_main";

	precacheMenu( game[ "menu_ingame_main" ] );
	//precacheMenu( "clientcmd" );

	precacheShader( "black" );
	precacheShader( "white" );

	level thread onPlayerConnect();
}

onPlayerConnect() 
{
	level endon( "game_ended" );

	while (1) 
    {
		level waittill( "connected", player );
		player setClientDvar( "g_scriptMainMenu", game[ "menu_ingame_main" ] );
		player thread onMenuResponse();

        //player playLoopSound("inter_amb");
	}
}

wait_respawn(timeout)
{
    self endon("spawned_player");

    wait timeout;
    thread maps\mp\gametypes\rtx::spawnPlayer();
}

onMenuResponse() 
{
	self endon( "disconnect" );

	while (1)  
    {
		self waittill( "menuresponse", menu, response );

		if( response == "back" ) 
        {
			self closeMenu();
			self closeInGameMenu();
			continue;
		}

		if( menu == game[ "menu_ingame_main" ] ) 
        {
			switch( response ) 
            {
			case "allies":
			case "axis":
			case "autoassign":
				self closeMenu();
				self closeInGameMenu();

                //level.inter_amb_sound stopLoopSound();

                if (isAlive(self)) 
                {
                    self suicide();
                    self thread wait_respawn(2);
                } else {
                    //self thread maps\mp\gametypes\_globallogic::spawnPlayer();
                    thread maps\mp\gametypes\rtx::spawnPlayer();
                }
				
				break;

			case "spectator":
				self closeMenu();
				self closeInGameMenu();

				self thread maps\mp\gametypes\rtx::spawnSpectator();
				break;
			}
		} else if ( !level.console ) {
			if( menu == game[ "menu_quickcommands" ] )
				maps\mp\gametypes\_quickmessages::quickcommands( response );
			else if( menu == game[ "menu_quickstatements" ] )
				maps\mp\gametypes\_quickmessages::quickstatements( response );
			else if( menu == game[ "menu_quickresponses" ] )
				maps\mp\gametypes\_quickmessages::quickresponses( response );
		}
	}
}