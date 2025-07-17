precacheAddons()
{
    _precacheItems();
    _precacheModels();
	_loadFx();
}

_precacheItems()
{
    preCacheItem("deserteagle_rtx_mp");
	preCacheItem("rain_blaster_mp");
	preCacheItem("deserteagle_nv_mp");
	preCacheItem("ak47_octo_mp");
}

_precacheModels()
{
    preCacheModel ( "rtx_lamp_post_proxy" );
	preCacheModel ( "rtx_suzanne" );
	preCacheModel ( "rtx_water_plane" );

	preCacheModel ( "rtx_cube" );
	preCacheModel ( "rtx_icosphere" );
	preCacheModel ( "rtx_sphere" );

	preCacheModel ( "rtx_r" );
	preCacheModel ( "rtx_t" );
	preCacheModel ( "rtx_x" );
}

_loadFx()
{
	level.rtx_pop = loadFx( "rtxcomp/balls_release" );
	level.rtx_pole = loadFx( "rtxcomp/lamp_post" );
	level.rtx_nvlogo = loadFx( "rtxcomp/nvlogo_expl" );
	level.rtx_meteor = loadFx( "rtxcomp/meteor" );
	level.rtx_rain = loadFx( "rtxcomp/rain" );
	level.water_bubbles = loadFx( "rtxcomp/water_bubbles" );
	level.water_life = loadFx( "rtxcomp/water_life" );
	level.water_transition_out = loadFx( "rtxcomp/water_transition_out" );
	level.water_transition_in = loadFx( "rtxcomp/water_transition_in" );
}

addDvar( scriptName, varName, varDefault, min, max, type ) 
{
	if( getDvar( varName ) != "" ) {
		switch( type ) {
		case "int":
			definition = getDvarInt( varName );
			break;

		case "float":
			definition = getDvarFloat( varName );
			break;

		default:
			definition = getDvar( varName );
			break;
		}
	}
	else
		definition = varDefault;

	if( type == "int" || type == "float" ) 
    {
		if( min != 0 && definition < min )
			definition = min;
		if( max != 0 && definition > max )
			definition = max;
	}

	level.dvar[ scriptName ] = definition;
}

serverDvars() 
{
	level.dvar = [];

    // Server :: iw3x dvars ::
    //setDvar( "pm_movementType",         "1" );
    //setDvar( "pm_bhop_auto",            true );
    setDvar( "pm_bhop_slowdown",        false );
    //setDvar( "pm_disableSprint",        true );
    //setDvar( "pm_cpm_useQuakeDamage",   true );
    //setDvar( "pm_rocketJumpHeight",     "0.0" );

	//setDvar( "pm_q3",     				"1" );
	//setDvar( "pm_q3_cpma",    			"1" );

    // Server :: stock dvars ::
    setDvar( "bg_falldamageminheight",  "99998.0" );
    setDvar( "bg_falldamagemaxheight",  "99999.0" );
    setDvar( "friction",                "8.0" );
	//setDvar( "bg_bobMax",               "0.0" );
    //setDvar( "g_speed",                 "416" );
	//setDvar( "jump_height",     		"46.0" );
	setDvar( "jump_slowdownenable",     false );

	setDvar( "bg_bobMax",  "0.0" );
   
	//setDvar( "sv_fps", "20" );
	//setDvar( "g_deadChat", 1 );
	
	setDvar( "g_TeamColor_Allies", "1 1 1 1" );
	setDvar( "g_TeamName_Allies", "RTX" );
	setDvar( "g_TeamName_Axis", "" );
	setDvar( "g_TeamIcon_Allies", "" );
	setDvar( "g_TeamIcon_Axis", "" );

	addDvar( "rtx_timelimit", "rtx_timelimit", 30, 1, 12000, "int" );

    setDvar( "mod_author", "#xoxor4d" );
	makeDvarServerInfo( "mod_author", "#xoxor4d" );
}

clientDvars()
{
	if( !isDefined(self) )
	{
        if( level.dbgPrints )
            println( " ++ ^1Q3^7::setup::clientDvars ++ -> self not defined!" );
        
        return;
    }

	// Client :: iw3x dvars ::
    //self setClientDvar( "pm_movementType",         	"1" );
    self setClientDvar( "pm_bhop_auto",            	true );
    self setClientDvar( "pm_bhop_slowdown",        	false );
    //self setClientDvar( "pm_disableSprint",        	true );
    //self setClientDvar( "pm_cpm_useQuakeDamage",   	true );
    //self setClientDvar( "pm_rocketJumpHeight",     	"0.0" );
    self setClientDvar( "jump_slowdownenable",     	false );
	//self setClientDvar( "bg_bobMax",     			"0.0" );

	//self setClientDvar( "pm_q3",     				"1" );
	//self setClientDvar( "pm_q3_cpma",    			"1" );

    // Client :: reset all movement related dvars back to stock; server already uses stock ones
    //self setClientDvar( "pm_cpm_airstopAccelerate",  "3.0" );
    //self setClientDvar( "pm_cpm_strafeAccelerate",   "70.0" );
    //self setClientDvar( "pm_cpm_airControl",         "150.0" );
    //self setClientDvar( "pm_cpm_airAccelerate",      "1.0" );
    //self setClientDvar( "pm_bhop_rampjump",          false );
    //self setClientDvar( "pm_crashland",              true );
    //self setClientDvar( "pm_cpm_damageKnockback",    "1.0" );

	self setClientDvars( "ui_hud_hardcore", 1);
}