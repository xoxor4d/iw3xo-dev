#include maps\mp\gametypes\_hud_util;

init()
{
	precacheString( &"MP_FIRSTPLACE_NAME" );
	precacheString( &"MP_SECONDPLACE_NAME" );
	precacheString( &"MP_THIRDPLACE_NAME" );
	precacheString( &"MP_MATCH_BONUS_IS" );

	game["strings"]["draw"] = &"MP_DRAW";
	game["strings"]["round_draw"] = &"MP_ROUND_DRAW";
	game["strings"]["round_win"] = &"MP_ROUND_WIN";
	game["strings"]["round_loss"] = &"MP_ROUND_LOSS";
	game["strings"]["victory"] = &"MP_VICTORY";
	game["strings"]["defeat"] = &"MP_DEFEAT";
	game["strings"]["halftime"] = &"MP_HALFTIME";
	game["strings"]["overtime"] = &"MP_OVERTIME";
	game["strings"]["roundend"] = &"MP_ROUNDEND";
	game["strings"]["intermission"] = &"MP_INTERMISSION";
	game["strings"]["side_switch"] = &"MP_SWITCHING_SIDES";
	game["strings"]["match_bonus"] = &"MP_MATCH_BONUS_IS";
	
	level thread onPlayerConnect();
}


onPlayerConnect()
{
	for(;;)
	{
		level waittill( "connecting", player );

		player thread hintMessageDeathThink();
		player thread lowerMessageThink();
		
		player thread initNotifyMessage();
	}
}


hintMessage( hintText, duration )
{
	notifyData = spawnstruct();
	
	notifyData.notifyText = hintText;
	notifyData.glowColor = (0.3, 0.6, 0.3);
	notifyData.duration = duration;

	notifyMessage( notifyData );
}

hintMessageWithTitle( titleText, hintText, duration )
{
	notifyData = spawnstruct();
	
	notifyData.titleText = titleText;
	notifyData.notifyText = hintText;
	notifyData.glowColor = (0.3, 0.6, 0.3);
	notifyData.duration = duration;
	
	notifyMessage( notifyData );
}


initNotifyMessage()
{
	if ( level.splitscreen )
	{
		titleSize = 2.0;
		textSize = 1.5;
		iconSize = 24;
		font = "default";
		point = "TOP";
		relativePoint = "BOTTOM";
		yOffset = 30;
		xOffset = 0;
	}
	else
	{
		titleSize = 2.5;
		textSize = 1.75;
		iconSize = 30;
		font = "objective";
		point = "TOP";
		relativePoint = "BOTTOM";
		yOffset = 30;
		xOffset = 0;
	}
	
	self.notifyTitle = createFontString( font, titleSize );
	self.notifyTitle setPoint( point, undefined, xOffset, yOffset );
	self.notifyTitle.glowColor = (0.2, 0.3, 0.7);
	self.notifyTitle.glowAlpha = 1;
	self.notifyTitle.hideWhenInMenu = true;
	self.notifyTitle.archived = false;
	self.notifyTitle.alpha = 0;

	self.notifyText = createFontString( font, textSize );
	self.notifyText setParent( self.notifyTitle );
	self.notifyText setPoint( point, relativePoint, 0, 0 );
	self.notifyText.glowColor = (0.2, 0.3, 0.7);
	self.notifyText.glowAlpha = 1;
	self.notifyText.hideWhenInMenu = true;
	self.notifyText.archived = false;
	self.notifyText.alpha = 0;

	self.notifyText2 = createFontString( font, textSize );
	self.notifyText2 setParent( self.notifyTitle );
	self.notifyText2 setPoint( point, relativePoint, 0, 0 );
	self.notifyText2.glowColor = (0.2, 0.3, 0.7);
	self.notifyText2.glowAlpha = 1;
	self.notifyText2.hideWhenInMenu = true;
	self.notifyText2.archived = false;
	self.notifyText2.alpha = 0;

	self.notifyIcon = createIcon( "white", iconSize, iconSize );
	self.notifyIcon setParent( self.notifyText2 );
	self.notifyIcon setPoint( point, relativePoint, 0, 0 );
	self.notifyIcon.hideWhenInMenu = true;
	self.notifyIcon.archived = false;
	self.notifyIcon.alpha = 0;

	self.doingNotify = false;
	self.notifyQueue = [];
}

oldNotifyMessage( titleText, notifyText, iconName, glowColor, sound, duration )
{
	notifyData = spawnstruct();
	
	notifyData.titleText = titleText;
	notifyData.notifyText = notifyText;
	notifyData.iconName = iconName;
	notifyData.glowColor = glowColor;
	notifyData.sound = sound;
	notifyData.duration = duration;
	
	notifyMessage( notifyData );
}

notifyMessage( notifyData )
{
	self endon ( "death" );
	self endon ( "disconnect" );
	
	if ( !self.doingNotify )
	{
		self thread showNotifyMessage( notifyData );
		return;
	}
	
	self.notifyQueue[ self.notifyQueue.size ] = notifyData;
}


showNotifyMessage( notifyData )
{
	self endon("disconnect");
	
	self.doingNotify = true;

	waitRequireVisibility( 0 );

	if ( isDefined( notifyData.duration ) )
		duration = notifyData.duration;
	else if ( level.gameEnded )
		duration = 2.0;
	else
		duration = 4.0;
	
	self thread resetOnCancel();

	if ( isDefined( notifyData.sound ) )
		self playLocalSound( notifyData.sound );

	if ( isDefined( notifyData.leaderSound ) )
		self maps\mp\gametypes\_globallogic::leaderDialogOnPlayer( notifyData.leaderSound );
	
	if ( isDefined( notifyData.glowColor ) )
		glowColor = notifyData.glowColor;
	else
		glowColor = (0.3, 0.6, 0.3);

	anchorElem = self.notifyTitle;

	if ( isDefined( notifyData.titleText ) )
	{
		if ( level.splitScreen )
		{
			if ( isDefined( notifyData.titleLabel ) )
				self iPrintLnBold( notifyData.titleLabel, notifyData.titleText );
			else
				self iPrintLnBold( notifyData.titleText );
		}
		else
		{
			if ( isDefined( notifyData.titleLabel ) )
				self.notifyTitle.label = notifyData.titleLabel;
			else
				self.notifyTitle.label = &"";

			if ( isDefined( notifyData.titleLabel ) && !isDefined( notifyData.titleIsString ) )
				self.notifyTitle setValue( notifyData.titleText );
			else
				self.notifyTitle setText( notifyData.titleText );
			self.notifyTitle setPulseFX( 100, int(duration*1000), 1000 );
			self.notifyTitle.glowColor = glowColor;	
			self.notifyTitle.alpha = 1;
		}
	}

	if ( isDefined( notifyData.notifyText ) )
	{
		if ( level.splitScreen )
		{
			if ( isDefined( notifyData.textLabel ) )
				self iPrintLnBold( notifyData.textLabel, notifyData.notifyText );
			else
				self iPrintLnBold( notifyData.notifyText );
		}
		else
		{
			if ( isDefined( notifyData.textLabel ) )
				self.notifyText.label = notifyData.textLabel;
			else
				self.notifyText.label = &"";
	
			if ( isDefined( notifyData.textLabel ) && !isDefined( notifyData.textIsString ) )
				self.notifyText setValue( notifyData.notifyText );
			else
				self.notifyText setText( notifyData.notifyText );
			self.notifyText setPulseFX( 100, int(duration*1000), 1000 );
			self.notifyText.glowColor = glowColor;	
			self.notifyText.alpha = 1;
			anchorElem = self.notifyText;
		}
	}

	if ( isDefined( notifyData.notifyText2 ) )
	{
		if ( level.splitScreen )
		{
			if ( isDefined( notifyData.text2Label ) )
				self iPrintLnBold( notifyData.text2Label, notifyData.notifyText2 );
			else
				self iPrintLnBold( notifyData.notifyText2 );
		}
		else
		{
			self.notifyText2 setParent( anchorElem );
			
			if ( isDefined( notifyData.text2Label ) )
				self.notifyText2.label = notifyData.text2Label;
			else
				self.notifyText2.label = &"";
	
			self.notifyText2 setText( notifyData.notifyText2 );
			self.notifyText2 setPulseFX( 100, int(duration*1000), 1000 );
			self.notifyText2.glowColor = glowColor;	
			self.notifyText2.alpha = 1;
			anchorElem = self.notifyText2;
		}
	}

	if ( isDefined( notifyData.iconName ) && !level.splitScreen )
	{
		self.notifyIcon setParent( anchorElem );
		self.notifyIcon setShader( notifyData.iconName, 60, 60 );
		self.notifyIcon.alpha = 0;
		self.notifyIcon fadeOverTime( 1.0 );
		self.notifyIcon.alpha = 1;
		
		waitRequireVisibility( duration );

		self.notifyIcon fadeOverTime( 0.75 );
		self.notifyIcon.alpha = 0;
	}
	else
	{
		waitRequireVisibility( duration );
	}

	self notify ( "notifyMessageDone" );
	self.doingNotify = false;

	if ( self.notifyQueue.size > 0 )
	{
		nextNotifyData = self.notifyQueue[0];
		
		newQueue = [];
		for ( i = 1; i < self.notifyQueue.size; i++ )
			self.notifyQueue[i-1] = self.notifyQueue[i];
		self.notifyQueue[i-1] = undefined;
		
		self thread showNotifyMessage( nextNotifyData );
	}
}

// waits for waitTime, plus any time required to let flashbangs go away.
waitRequireVisibility( waitTime )
{
	interval = .05;
	
	while ( !self canReadText() )
		wait interval;
	
	while ( waitTime > 0 )
	{
		wait interval;
		if ( self canReadText() )
			waitTime -= interval;
	}
}


canReadText()
{
	if ( self maps\mp\_flashgrenades::isFlashbanged() )
		return false;
	
	return true;
}


resetOnDeath()
{
	self endon ( "notifyMessageDone" );
	self endon ( "disconnect" );
	level endon ( "game_ended" );
	self waittill ( "death" );

	resetNotify();
}


resetOnCancel()
{
	self notify ( "resetOnCancel" );
	self endon ( "resetOnCancel" );
	self endon ( "notifyMessageDone" );
	self endon ( "disconnect" );

	level waittill ( "cancel_notify" );
	
	resetNotify();
}


resetNotify()
{
	self.notifyTitle.alpha = 0;
	self.notifyText.alpha = 0;
	self.notifyIcon.alpha = 0;
	self.doingNotify = false;
}


hintMessageDeathThink()
{
	self endon ( "disconnect" );

	for ( ;; )
	{
		self waittill ( "death" );
		
		if ( isDefined( self.hintMessage ) )
			self.hintMessage destroyElem();
	}
}

lowerMessageThink()
{
	self endon ( "disconnect" );
	
	self.lowerMessage = createFontString( "default", level.lowerTextFontSize );
	self.lowerMessage setPoint( "CENTER", level.lowerTextYAlign, 0, level.lowerTextY );
	self.lowerMessage setText( "" );
	self.lowerMessage.archived = false;
	
	timerFontSize = 1.5;
	if ( level.splitscreen )
		timerFontSize = 1.4;
	
	self.lowerTimer = createFontString( "default", timerFontSize );
	self.lowerTimer setParent( self.lowerMessage );
	self.lowerTimer setPoint( "TOP", "BOTTOM", 0, 0 );
	self.lowerTimer setText( "" );
	self.lowerTimer.archived = false;
}

teamOutcomeNotify( winner, isRound, endReasonText )
{
	self endon ( "disconnect" );
	self notify ( "reset_outcome" );

	team = self.pers["team"];
	if ( !isDefined( team ) || (team != "allies" && team != "axis") )
		team = "allies";

	// wait for notifies to finish
	while ( self.doingNotify )
		wait 0.05;

	self endon ( "reset_outcome" );
	
	if ( level.splitscreen )
	{
		titleSize = 2.0;
		textSize = 1.5;
		iconSize = 30;
		spacing = 10;
		font = "default";
	}
	else
	{
		titleSize = 3.0;
		textSize = 2.0;
		iconSize = 70;
		spacing = 30;
		font = "objective";
	}

	duration = 60000;

	outcomeTitle = createFontString( font, titleSize );
	outcomeTitle setPoint( "TOP", undefined, 0, 30 );
	outcomeTitle.glowAlpha = 1;
	outcomeTitle.hideWhenInMenu = false;
	outcomeTitle.archived = false;

	outcomeText = createFontString( font, 2.0 );
	outcomeText setParent( outcomeTitle );
	outcomeText setPoint( "TOP", "BOTTOM", 0, 0 );
	outcomeText.glowAlpha = 1;
	outcomeText.hideWhenInMenu = false;
	outcomeText.archived = false;
	
	if ( winner == "halftime" )
	{
		outcomeTitle.glowColor = (0.2, 0.3, 0.7);
		outcomeTitle setText( game["strings"]["halftime"] );
		outcomeTitle.color = (1, 1, 1);
		
		winner = "allies";
	}
	else if ( winner == "intermission" )
	{
		outcomeTitle.glowColor = (0.2, 0.3, 0.7);
		outcomeTitle setText( game["strings"]["intermission"] );
		outcomeTitle.color = (1, 1, 1);
		
		winner = "allies";
	}
	else if ( winner == "roundend" )
	{
		outcomeTitle.glowColor = (0.2, 0.3, 0.7);
		outcomeTitle setText( game["strings"]["roundend"] );
		outcomeTitle.color = (1, 1, 1);
		
		winner = "allies";
	}
	else if ( winner == "overtime" )
	{
		outcomeTitle.glowColor = (0.2, 0.3, 0.7);
		outcomeTitle setText( game["strings"]["overtime"] );
		outcomeTitle.color = (1, 1, 1);
		
		winner = "allies";
	}
	else if ( winner == "tie" )
	{
		outcomeTitle.glowColor = (0.2, 0.3, 0.7);
		if ( isRound )
			outcomeTitle setText( game["strings"]["round_draw"] );
		else
			outcomeTitle setText( game["strings"]["draw"] );
		outcomeTitle.color = (1, 1, 1);
		
		winner = "allies";
	}
	else if ( isDefined( self.pers["team"] ) && winner == team )
	{
		outcomeTitle.glowColor = (0, 0, 0);
		if ( isRound )
			outcomeTitle setText( game["strings"]["round_win"] );
		else
			outcomeTitle setText( game["strings"]["victory"] );
		outcomeTitle.color = (0.6, 0.9, 0.6);
	}
	else
	{
		outcomeTitle.glowColor = (0, 0, 0);
		if ( isRound )
			outcomeTitle setText( game["strings"]["round_loss"] );
		else
			outcomeTitle setText( game["strings"]["defeat"] );
		outcomeTitle.color = (0.7, 0.3, 0.2);
	}
	
	outcomeText.glowColor = (0.2, 0.3, 0.7);
	outcomeText setText( endReasonText );
	
	outcomeTitle setPulseFX( 100, duration, 1000 );
	outcomeText setPulseFX( 100, duration, 1000 );
	
	leftIcon = createIcon( game["icons"][team], iconSize, iconSize );
	leftIcon setParent( outcomeText );
	leftIcon setPoint( "TOP", "BOTTOM", -60, spacing );
	leftIcon.hideWhenInMenu = false;
	leftIcon.archived = false;
	leftIcon.alpha = 0;
	leftIcon fadeOverTime( 0.5 );
	leftIcon.alpha = 1;

	rightIcon = createIcon( game["icons"][level.otherTeam[team]], iconSize, iconSize );
	rightIcon setParent( outcomeText );
	rightIcon setPoint( "TOP", "BOTTOM", 60, spacing );
	rightIcon.hideWhenInMenu = false;
	rightIcon.archived = false;
	rightIcon.alpha = 0;
	rightIcon fadeOverTime( 0.5 );
	rightIcon.alpha = 1;

	leftScore = createFontString( font, titleSize );
	leftScore setParent( leftIcon );
	leftScore setPoint( "TOP", "BOTTOM", 0, spacing );
	leftScore.glowColor = game["colors"][team];
	leftScore.glowAlpha = 1;
	leftScore setValue( getTeamScore( team ) );
	leftScore.hideWhenInMenu = false;
	leftScore.archived = false;
	leftScore setPulseFX( 100, duration, 1000 );

	rightScore = createFontString( font, titleSize );
	rightScore setParent( rightIcon );
	rightScore setPoint( "TOP", "BOTTOM", 0, spacing );
	rightScore.glowColor = game["colors"][level.otherTeam[team]];
	rightScore.glowAlpha = 1;
	rightScore setValue( getTeamScore( level.otherTeam[team] ) );
	rightScore.hideWhenInMenu = false;
	rightScore.archived = false;
	rightScore setPulseFX( 100, duration, 1000 );

	matchBonus = undefined;
	if ( isDefined( self.matchBonus ) )
	{
		matchBonus = createFontString( font, 2.0 );
		matchBonus setParent( outcomeText );
		matchBonus setPoint( "TOP", "BOTTOM", 0, iconSize + (spacing * 3) + leftScore.height );
		matchBonus.glowAlpha = 1;
		matchBonus.hideWhenInMenu = false;
		matchBonus.archived = false;
		matchBonus.label = game["strings"]["match_bonus"];
		matchBonus setValue( self.matchBonus );
	}
	
	self thread resetTeamOutcomeNotify( outcomeTitle, outcomeText, leftIcon, rightIcon, leftScore, rightScore, matchBonus );
}


outcomeNotify( winner, endReasonText )
{
	self endon ( "disconnect" );
	self notify ( "reset_outcome" );

	// wait for notifies to finish
	while ( self.doingNotify )
		wait 0.05;

	self endon ( "reset_outcome" );

	if ( level.splitscreen )
	{
		titleSize = 2.0;
		winnerSize = 1.5;
		otherSize = 1.5;
		iconSize = 30;
		spacing = 2;
		font = "default";
	}
	else
	{
		titleSize = 3.0;
		winnerSize = 2.0;
		otherSize = 1.5;
		iconSize = 30;
		spacing = 20;
		font = "objective";
	}

	duration = 60000;

	players = level.placement["all"];
		
	outcomeTitle = createFontString( font, titleSize );
	outcomeTitle setPoint( "TOP", undefined, 0, spacing );
	if ( isDefined( players[1] ) && players[0].score == players[1].score && players[0].deaths == players[1].deaths && (self == players[0] || self == players[1]) )
	{
		outcomeTitle setText( game["strings"]["tie"] );
		outcomeTitle.glowColor = (0.2, 0.3, 0.7);
	}
	else if ( isDefined( players[2] ) && players[0].score == players[2].score && players[0].deaths == players[2].deaths && self == players[2] )
	{
		outcomeTitle setText( game["strings"]["tie"] );
		outcomeTitle.glowColor = (0.2, 0.3, 0.7);
	}
	else if ( isDefined( players[0] ) && self == players[0] )
	{
		outcomeTitle setText( game["strings"]["victory"] );
		outcomeTitle.glowColor = (0.2, 0.3, 0.7);
	}
	else
	{
		outcomeTitle setText( game["strings"]["defeat"] );
		outcomeTitle.glowColor = (0.7, 0.3, 0.2);
	}
	outcomeTitle.glowAlpha = 1;
	outcomeTitle.hideWhenInMenu = false;
	outcomeTitle.archived = false;
	outcomeTitle setPulseFX( 100, duration, 1000 );

	outcomeText = createFontString( font, 2.0 );
	outcomeText setParent( outcomeTitle );
	outcomeText setPoint( "TOP", "BOTTOM", 0, 0 );
	outcomeText.glowAlpha = 1;
	outcomeText.hideWhenInMenu = false;
	outcomeText.archived = false;
	outcomeText.glowColor = (0.2, 0.3, 0.7);
	outcomeText setText( endReasonText );

	firstTitle = createFontString( font, winnerSize );
	firstTitle setParent( outcomeText );
	firstTitle setPoint( "TOP", "BOTTOM", 0, spacing );
	firstTitle.glowColor = (0.3, 0.7, 0.2);
	firstTitle.glowAlpha = 1;
	firstTitle.hideWhenInMenu = false;
	firstTitle.archived = false;
	if ( isDefined( players[0] ) )
	{
		firstTitle.label = &"MP_FIRSTPLACE_NAME";
		firstTitle setPlayerNameString( players[0] );
		firstTitle setPulseFX( 100, duration, 1000 );
	}

	secondTitle = createFontString( font, otherSize );
	secondTitle setParent( firstTitle );
	secondTitle setPoint( "TOP", "BOTTOM", 0, spacing );
	secondTitle.glowColor = (0.2, 0.3, 0.7);
	secondTitle.glowAlpha = 1;
	secondTitle.hideWhenInMenu = false;
	secondTitle.archived = false;
	if ( isDefined( players[1] ) )
	{
		secondTitle.label = &"MP_SECONDPLACE_NAME";
		secondTitle setPlayerNameString( players[1] );
		secondTitle setPulseFX( 100, duration, 1000 );
	}
	
	thirdTitle = createFontString( font, otherSize );
	thirdTitle setParent( secondTitle );
	thirdTitle setPoint( "TOP", "BOTTOM", 0, spacing );
	thirdTitle setParent( secondTitle );
	thirdTitle.glowColor = (0.2, 0.3, 0.7);
	thirdTitle.glowAlpha = 1;
	thirdTitle.hideWhenInMenu = false;
	thirdTitle.archived = false;
	if ( isDefined( players[2] ) )
	{
		thirdTitle.label = &"MP_THIRDPLACE_NAME";
		thirdTitle setPlayerNameString( players[2] );
		thirdTitle setPulseFX( 100, duration, 1000 );
	}

	matchBonus = createFontString( font, 2.0 );
	matchBonus setParent( thirdTitle );
	matchBonus setPoint( "TOP", "BOTTOM", 0, spacing );
	matchBonus.glowAlpha = 1;
	matchBonus.hideWhenInMenu = false;
	matchBonus.archived = false;
	if ( isDefined( self.matchBonus ) )
	{
		matchBonus.label = game["strings"]["match_bonus"];
		matchBonus setValue( self.matchBonus );
	}

	self thread updateOutcome( firstTitle, secondTitle, thirdTitle );
	self thread resetOutcomeNotify( outcomeTitle, outcomeText, firstTitle, secondTitle, thirdTitle, matchBonus );
}


resetOutcomeNotify( outcomeTitle, outcomeText, firstTitle, secondTitle, thirdTitle, matchBonus )
{
	self endon ( "disconnect" );
	self waittill ( "reset_outcome" );
	
	if ( isDefined( outcomeTitle ) )
		outcomeTitle destroyElem();
	if ( isDefined( outcomeText ) )
		outcomeText destroyElem();
	if ( isDefined( firstTitle ) )
		firstTitle destroyElem();
	if ( isDefined( secondTitle ) )
		secondTitle destroyElem();
	if ( isDefined( thirdTitle ) )
		thirdTitle destroyElem();
	if ( isDefined( matchBonus ) )
		matchBonus destroyElem();
}

resetTeamOutcomeNotify( outcomeTitle, outcomeText, leftIcon, rightIcon, LeftScore, rightScore, matchBonus )
{
	self endon ( "disconnect" );
	self waittill ( "reset_outcome" );

	if ( isDefined( outcomeTitle ) )
		outcomeTitle destroyElem();
	if ( isDefined( outcomeText ) )
		outcomeText destroyElem();
	if ( isDefined( leftIcon ) )
		leftIcon destroyElem();
	if ( isDefined( rightIcon ) )
		rightIcon destroyElem();
	if ( isDefined( leftScore ) )
		leftScore destroyElem();
	if ( isDefined( rightScore ) )
		rightScore destroyElem();
	if ( isDefined( matchBonus ) )
		matchBonus destroyElem();
}


updateOutcome( firstTitle, secondTitle, thirdTitle )
{
	self endon( "disconnect" );
	self endon( "reset_outcome" );
	
	while( true )
	{
		self waittill( "update_outcome" );

		players = level.placement["all"];

		if ( isDefined( firstTitle ) && isDefined( players[0] ) )
			firstTitle setPlayerNameString( players[0] );
		else if ( isDefined( firstTitle ) )
			firstTitle.alpha = 0;
		
		if ( isDefined( secondTitle ) && isDefined( players[1] ) )
			secondTitle setPlayerNameString( players[1] );
		else if ( isDefined( secondTitle ) )
			secondTitle.alpha = 0;
		
		if ( isDefined( thirdTitle ) && isDefined( players[2] ) )
			thirdTitle setPlayerNameString( players[2] );
		else if ( isDefined( thirdTitle ) )
			thirdTitle.alpha = 0;
	}	
}
