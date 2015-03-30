#pragma config(Sensor, port1,  userSensor,     sensorNone)
#pragma config(Sensor, port4,  touch2,         sensorVexIQ_LED)
#pragma config(Sensor, port5,  touch3,         sensorVexIQ_LED)
#pragma config(Sensor, port9,  touch1,         sensorVexIQ_LED)
#pragma config(Sensor, port10, touch4,         sensorVexIQ_LED)
#pragma config(Motor,  motor8, motorL,         tmotorVexIQ, PIDControl, encoder)
#pragma config(Motor,  motor11,motorR,         tmotorVexIQ, PIDControl, reversed, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2015                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     simon.c                                                      */
/*    Author:     James Pearman                                                */
/*    Created:    27 March 2015                                                */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     27 March 2015 - Initial release                    */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    The author is supplying this software for use with the VEX IQ            */
/*    control system. This file can be freely distributed and teams are        */
/*    authorized to freely use this program , however, it is requested that    */
/*    improvements or additions be shared with the Vex community via the vex   */
/*    forum.  Please acknowledge the work of the authors when appropriate.     */
/*    Thanks.                                                                  */
/*                                                                             */
/*    Licensed under the Apache License, Version 2.0 (the "License");          */
/*    you may not use this file except in compliance with the License.         */
/*    You may obtain a copy of the License at                                  */
/*                                                                             */
/*      http://www.apache.org/licenses/LICENSE-2.0                             */
/*                                                                             */
/*    Unless required by applicable law or agreed to in writing, software      */
/*    distributed under the License is distributed on an "AS IS" BASIS,        */
/*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/*    See the License for the specific language governing permissions and      */
/*    limitations under the License.                                           */
/*                                                                             */
/*    The author can be contacted on the vex forums as jpearman                */
/*    or electronic mail using jbpearman_at_mac_dot_com                        */
/*    Mentor for team 8888 RoboLancers, Pasadena CA.                           */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Simple implementation of the Simon game for Vex worlds 2015              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/

#include "generic_i2c.c"
#include "matrix.c"
#include "wdMatrixUtil.c"

#define wdMotorFlagLeft     motorL
#define wdMotorFlagRight    motorR

#include "wdMotorLib.c"

typedef enum t_GameAction {
    kActionNone = 0,
    kActionRed,
    kActionGreen,
    kActionBlue,
    kActionYellow,

    kActionInit,
    kActionWin,
    kActionLoss
} t_GameAction;

// maximum Length of game
#define MAX_GAME_LENGTH     20
// Array to hold game sequence
t_GameAction gameArray[ MAX_GAME_LENGTH ];

// Game states
typedef enum t_GameStates {
    kGameStatePre = 0,
    kGameStateInit,
    kGameStateComputer,
    kGameStateUser,
    kGameStateLoss,
    kGameStateWin,
    kGameStateGameOver
} t_GameStates;

typedef enum t_GameLevel {
    kGameEasy = 0,
    kGameNormal,
    kGameHard,
    kGameTest
} t_GameLevel;

// Global we use to start and stop sound
t_GameAction gAction = kActionNone;

// Game difficulty level
t_GameLevel  gLevel;
// Game length based on difficulty
int          gLength;

// Port for the user sensor that controls the LED matrix
portName userPort;

/*-----------------------------------------------------------------------------*/
// Map sensor ports
#define touchA  touch1
#define touchB  touch2
#define touchC  touch3
#define touchD  touch4

/*-----------------------------------------------------------------------------*/
// Play the fancy win sound
void
simonPlayWinSound()
{
    int     note;
    int     octave;

    for(note=1;note<9;note+=2) {
        for(octave=1;octave<4;octave++) {
            playNote( note, octave, 10 );

            wait1Msec(25);
            }
        }
    // Wait for sound queue to empty
    while( bSoundActive ) wait1Msec(10);

    for(note=8;note>0;note-=2) {
        for(octave=3;octave>0;octave--) {
            playNote( note, octave, 10 );

            wait1Msec(25);
            }
        }
    // Wait for sound queue to empty
    while( bSoundActive ) wait1Msec(10);
}

/*-----------------------------------------------------------------------------*/
// Play note associated with a touch LED
void
simonSoundTouchLedPlay( t_GameAction l )
{
    // Duration in 10mS increments
    // Sound will be terminated by clearSounds
    const int soundDuration = 500;

    switch(l)
        {
        case    kActionRed:
            playNote(4, 3, soundDuration ); // Red
            break;
        case    kActionGreen:
            playNote(5, 1, soundDuration );  // Green
            break;
        case    kActionBlue:
            playNote(4, 0, soundDuration ); // Blue
            break;
        case    kActionYellow:
            playNote(4, 1, soundDuration ); // Yellow
            break;
        default:
            break;
        }
}

/*-----------------------------------------------------------------------------*/
// Task that starts and stops sounds when the touch LED are pressed
task
simonSoundTask()
{
    while(1)
        {
        if( gAction == kActionWin ) {
            simonPlayWinSound();
            gAction = kActionNone;
            }
        else
        if( gAction == kActionLoss ) {
            playNote( 1, 1, 100 );
            gAction = kActionNone;
            }
        else
        if( gAction != kActionNone ) {
            // User has touched an LED so play sound
            simonSoundTouchLedPlay( gAction );
            // Wait for touch LED release
            while( gAction != kActionNone )
                wait1Msec(10);
            // Now clear the sound playback
            clearSounds();
            }

        // Don't hog the cpu
        wait1Msec(25);
        }
}

/*-----------------------------------------------------------------------------*/
// Turn on Touch LED
void
simonTouchButtonLedSet( t_GameAction l )
{
    static  t_GameAction cl = kActionInit;

    if( l == cl )
        return;
    cl = l;

    switch(l)
        {
        case    kActionRed:
            setTouchLEDRGB( touchA, 255, 0, 0);    // Red
            break;
        case    kActionGreen:
            setTouchLEDRGB( touchB, 0, 255, 0);    // Green
            break;
        case    kActionBlue:
            setTouchLEDRGB( touchC, 0, 0, 255);    // Blue
            break;
        case    kActionYellow:
            setTouchLEDRGB( touchD, 255, 255, 0); // Yellow
            break;

        default:
            setTouchLEDRGB( touchA, 0, 0, 0);
            setTouchLEDRGB( touchB, 0, 0, 0);
            setTouchLEDRGB( touchC, 0, 0, 0);
            setTouchLEDRGB( touchD, 0, 0, 0);
            break;
        }
}

/*-----------------------------------------------------------------------------*/
// Set the global that allow sound playback for this led
void
simonTouchLedSoundSet( t_GameAction l )
{
    gAction = l;
}

/*-----------------------------------------------------------------------------*/
// Blink a touch LED ans play sound
void
simonBlinkLed( t_GameAction l )
{
    // On
    simonTouchButtonLedSet( l );
    simonTouchLedSoundSet( l );

    // Wait
    wait1Msec(250);

    // Off
    simonTouchButtonLedSet( kActionNone );
    simonTouchLedSoundSet( kActionNone );
}

/*-----------------------------------------------------------------------------*/
// Check the four touch buttins and return state
t_GameAction
simonTouchButtonGet()
{
    t_GameAction pressed = kActionNone;

    if( SensorValue[ touchA ] == 1 ) {
        pressed = kActionRed;
    }

    if( SensorValue[ touchB ] == 1 ) {
        pressed = kActionGreen;
    }

    if( SensorValue[ touchC ] == 1 ) {
        pressed = kActionBlue;
    }

    if( SensorValue[ touchD ] == 1 ) {
        pressed = kActionYellow;
    }

    return( pressed );
}

/*-----------------------------------------------------------------------------*/
// Get the state of a touch LED during the user turn
t_GameAction
simonUserButtonGet()
{
    t_GameAction l = kActionNone;

    unsigned long timeout = nSysTime + 5000; // 5 second timeout

    while( nSysTime < timeout )
        {
        l = simonTouchButtonGet();

        if( l != kActionNone ){
            // illuminate and play sound while pressed
            simonTouchButtonLedSet( l );
            simonTouchLedSoundSet( l );

            // Wait for release
            while( simonTouchButtonGet() != kActionNone )
                wait1Msec(25);

            // led and sound off
            simonTouchButtonLedSet( kActionNone );
            simonTouchLedSoundSet( kActionNone );

            return( l );
            }

        wait1Msec(25);
        }

    return( kActionNone );
}

/*-----------------------------------------------------------------------------*/
// run the pre game display

#define BANNER_TEXT " Play a game! "

// forward ref
void    simonDisplayGameLevel();

void
simonPreGame()
{
    t_GameAction l1, l2;
    int     i;
    int     loops = 0;
    int     scrollDir = MATRIX_MODE_SCROLL_LEFT;

    eraseDisplay();
    displayBigTextLine(0, "Touch LED");
    displayBigTextLine(2, "to start");
    simonDisplayGameLevel();

    // Start banner text
    // We display as static text first so as to reset the scroll counter
    // in the display.
    ledMatrixWriteString( userPort, BANNER_TEXT, );
    wait1Msec(1000);
    ledMatrixWriteString( userPort, BANNER_TEXT, MATRIX_MODE_SPEED_SLOW3 | scrollDir );

    while(1)
        {
        for(l1=kActionRed;l1<=kActionYellow;l1++)
            {
            // touch led on
            simonTouchButtonLedSet( l1 );

            // check for user input that starts the game
            for(i=0;i<8;i++) {
                wait1Msec(20);

                // Check for user pressing one of the touch leds
                l2 = simonTouchButtonGet();
                if( l2 != kActionNone )
                    {
                    wdMotorFlagOpen();
                    simonTouchButtonLedSet(kActionNone);

                    simonTouchButtonLedSet( l2 );
                    simonTouchLedSoundSet( l2 );
                    while( simonTouchButtonGet() != kActionNone )
                        wait1Msec(25);
                    simonTouchButtonLedSet( kActionNone );
                    simonTouchLedSoundSet( kActionNone );
                    return;
                    }

                // Check up and down arrows for difficulty change
                if( nLCDButtons == (kButtonUp | kButtonDown) )
                    {
                    // Test level - quick games
                    gLevel = kGameTest;
                    simonDisplayGameLevel();
                    while(nLCDButtons != kButtonNone) wait1Msec(10);
                    }
                else
                if( nLCDButtons == kButtonDown )
                    {
                    switch(gLevel) {
                        case kGameEasy   :  gLevel = kGameNormal; break;
                        case kGameNormal :  gLevel = kGameHard;   break;
                        case kGameHard   :  gLevel = kGameEasy;   break;
                        default:            gLevel = kGameNormal; break;
                        }
                    simonDisplayGameLevel();
                    while(nLCDButtons != kButtonNone) wait1Msec(10);
                    }
                else
                if( nLCDButtons == kButtonUp )
                    {
                    switch(gLevel) {
                        case kGameEasy   :  gLevel = kGameHard;   break;
                        case kGameNormal :  gLevel = kGameEasy;   break;
                        case kGameHard   :  gLevel = kGameNormal; break;
                        default:            gLevel = kGameNormal; break;
                        }
                    simonDisplayGameLevel();
                    while(nLCDButtons != kButtonNone) wait1Msec(10);
                    }
                }

            // touch led off
            simonTouchButtonLedSet( kActionNone );

            // Wave the flags occasionally
            ++loops;
            if(loops == 60)
                wdMotorFlagClose();
            if(loops == 72)
                {
                scrollDir = (scrollDir == MATRIX_MODE_SCROLL_LEFT ) ?
                                           MATRIX_MODE_SCROLL_RIGHT :
                                           MATRIX_MODE_SCROLL_LEFT;

                ledMatrixWriteMode( userPort, MATRIX_MODE_SPEED_SLOW3 | scrollDir );

                wdMotorFlagOpen();
                loops = 0;
                }
            }
        }
}

/*-----------------------------------------------------------------------------*/
// Fill the game array
void
simonNewGame()
{
    int     i;

    srand( (short) nSysTime & 0xFFFF );

    writeDebugStream("New Game: ");

    for(i=0;i<MAX_GAME_LENGTH;i++)
        {
        gameArray[i] = (t_GameAction)( rand() % 4 + 1 );
        writeDebugStream("%d ", gameArray[i]);
        }

    switch(gLevel)
        {
        case    kGameEasy:   gLength = 4;  break;
        case    kGameNormal: gLength = 8;  break;
        case    kGameHard:   gLength = 12; break;
        case    kGameTest:   gLength = 2;  break;
        default:             gLength = 8;  break;
        }

    writeDebugStreamLine("");
}

/*-----------------------------------------------------------------------------*/
// Display game level
void
simonDisplayGameLevel()
{
    switch(gLevel)
        {
        case    kGameEasy:
            displayTextLine(4, " Level: Easy ");
            break;
        case    kGameNormal:
            displayTextLine(4, " Level: Normal ");
            break;
        case    kGameHard:
            displayTextLine(4, " Level: Hard ");
            break;
        case    kGameTest:
            displayTextLine(4, " Level: Test ");
            break;
        default:
            displayTextLine(4, " Level: ??? ");
            break;
        }
}

/*-----------------------------------------------------------------------------*/
// Banner and messages for a new game
void
simonDisplayNewGame()
{
    int i;

    eraseDisplay();
    displayCenteredBigTextLine(1,"New");
    displayCenteredBigTextLine(3,"Game");

    // Flash "New Game" on LED display
    for(i=0;i<3;i++) {
        wdLedCenteredText( userPort, "New" );
        wait1Msec(500);
        wdLedCenteredText( userPort, "Game" );
        wait1Msec(500);
    }

    wait1Msec(1000);
}

/*-----------------------------------------------------------------------------*/
// Display for new round
void
simonDisplayNewRound(int r)
{
    char str[3];

    eraseDisplay();
    displayCenteredBigTextLine(1,"Round");
    displayCenteredBigTextLine(3,"%d", r);

    sprintf(str,"%d", r);
    wdLedCenteredText( userPort, str );
    wait1Msec(750);
}

/*-----------------------------------------------------------------------------*/
// Display for computer turn
void
simonDisplayComputerTurn()
{
    eraseDisplay();
    displayCenteredBigTextLine(1,"Computer");
    displayCenteredBigTextLine(3,"Turn");

    wdLedCenteredText( userPort, "Brain" );
    wait1Msec(1000);
}

/*-----------------------------------------------------------------------------*/
// Display for user turn
void
simonDisplayUserTurn()
{
    eraseDisplay();
    displayCenteredBigTextLine(1,"User");
    displayCenteredBigTextLine(3,"Turn");

    wdLedCenteredText( userPort, "User" );
}

/*-----------------------------------------------------------------------------*/
// Display when user wins
void
simonDisplayGameWin()
{
    eraseDisplay();
    displayCenteredBigTextLine(1,"Win");

    gAction = kActionWin;
    wdLedFlashText( userPort, "Win", 5, true );
}


/*-----------------------------------------------------------------------------*/
// Display when user looses
void
simonDisplayGameLoss( int score )
{
    eraseDisplay();
    displayCenteredBigTextLine(1,"Loss");
    displayCenteredBigTextLine(3,"%d of %d", score, gLength );

    gAction = kActionLoss;
    ledMatrixWriteString( userPort, "Try Again ", MATRIX_MODE_SPEED_FAST | MATRIX_MODE_SCROLL_LEFT );
}

/*-----------------------------------------------------------------------------*/
// Game engine
void
simonRunGame()
{
    int             currentRound = 0;
    int             currentLight;
    t_GameStates    gameState = kGameStatePre;

    while(true)
        {
        switch( gameState )
            {
            case    kGameStatePre:
                simonPreGame();
                gameState = kGameStateInit;
                break;

            case    kGameStateInit:
                simonNewGame();
                simonDisplayNewGame();

                gameState = kGameStateComputer;
                break;
            case    kGameStateComputer:
                simonDisplayNewRound(currentRound+1);
                simonDisplayComputerTurn();

                for(currentLight=0;currentLight<=currentRound;currentLight++)
                    {
                    simonBlinkLed( gameArray[ currentLight ] );

                    // Speed up later in game unless level is set to hard
                    if( currentRound < 5 && gLevel != kGameHard )
                        wait1Msec(350);
                    else
                        wait1Msec(200); // quicker near the end
                    }

                currentLight = 0;
                gameState = kGameStateUser;
                break;

            case    kGameStateUser:
                simonDisplayUserTurn();

                if( simonUserButtonGet() == gameArray[ currentLight ] )
                    {
                    if( currentLight == currentRound ) {
                        currentRound++;
                        if( currentRound == gLength )
                            gameState = kGameStateWin;
                        else
                            gameState = kGameStateComputer;
                        }
                    else
                        currentLight++;
                    }
                else
                    gameState = kGameStateLoss;
                break;

            case    kGameStateLoss:
                simonDisplayGameLoss(currentRound);
                gameState = kGameStateGameOver;
                break;

            case    kGameStateWin:
                simonDisplayGameWin();
                gameState = kGameStateGameOver;
                break;

            case    kGameStateGameOver:
                // 5 seconds and then reset
                wait1Msec(5000);
                return;
                break;

            default:
                gameState = kGameStatePre;
                break;
            }

        wait1Msec(25);
        }
}

/*-----------------------------------------------------------------------------*/
// main entry - initialize and run game engine
task main()
{
    // start the background tasks
    startTask(simonSoundTask);
    startTask(wdMotorTask);

    // Find the user sensor
    userPort = genericI2cFindFirst();

    // We need user sensor LED matrix for this game
    if(userPort < 0)
        {
        eraseDisplay();
        displayBigTextLine( 1, "Sensor");
        displayBigTextLine( 3, "Error");
        while(1) wait1Msec(10);
        }

    // Clear the LED matrix
    wdLedClear( userPort );

    // Wait for motors to home
    wait1Msec(3000);

    // Open and start game
    wdMotorFlagOpen();
    wait1Msec(1000);

    // Run the game forever
    while(1)
        {
        simonRunGame();
        wait1Msec(25);
        }
}