/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2015                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     wdMotorLib.c                                                 */
/*    Author:     James Pearman                                                */
/*    Created:    20 March 2015                                                */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     tbd - Initial release                              */
/*                 4 April  Added motor port check so we can disconnect        */
/*                          motors if needed.                                  */
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
/*    Motor control for the 2015 VEX IQ worlds demo bot                        */
/*-----------------------------------------------------------------------------*/

#ifndef wdMotorFlagLeft
#define wdMotorFlagLeft     motor1
#endif
#ifndef wdMotorFlagRight
#define wdMotorFlagRight    motor6
#endif

static  int  wdMotorTargetL;
static  int  wdMotorTargetR;
static  bool wdwdMotorFlagRightepositionRequest = false;

/*-----------------------------------------------------------------------------*/
// Send the two flag motors to the open position
void
wdMotorFlagOpen()
{
    wdMotorTargetL = 180;
    wdMotorTargetR = 180;
    wdwdMotorFlagRightepositionRequest = true;
}

/*-----------------------------------------------------------------------------*/
// Send the two flag motors to the close position

void
wdMotorFlagClose()
{
    wdMotorTargetL = 0;
    wdMotorTargetR = 0;
    wdwdMotorFlagRightepositionRequest = true;
}

/*-----------------------------------------------------------------------------*/
// Check that motor at index is present
bool
wdMotorCheck(tMotor index)
{
    TVexIQDeviceTypes   type;
    TDeviceStatus       status;
    short               ver;

    getVexIqDeviceInfo( (portName)index, type, status, ver );
    if( type == vexIQ_SensorMOTOR )
        return(true);
    else
        return(false);
}

/*-----------------------------------------------------------------------------*/
// Initialize the flag motors - set low current etc.

int
wdMotorInit()
{
    // Check motors are connected
    // Motors are checked by ROBOTC on main entry usually
    // this can be turned off in the pref dialog or by not including in the
    // motors & sensors setup.
    if(wdMotorCheck( wdMotorFlagLeft ) == false )
        return(0);
    if(wdMotorCheck( wdMotorFlagRight ) == false )
        return(0);

    // Limit current and put each motor
    // into coast mode
    setMotorCurrentLimit(wdMotorFlagLeft, 500);
    setMotorBrakeMode(wdMotorFlagLeft, motorCoast);
    setMotorCurrentLimit(wdMotorFlagRight, 500);
    setMotorBrakeMode(wdMotorFlagRight, motorCoast);

    // right needs to be reversed
    setMotorReversed(wdMotorFlagRight, true);

    // Clear encoders
    nMotorEncoder[ wdMotorFlagLeft ] = 0;
    nMotorEncoder[ wdMotorFlagRight ] = 0;

    return(1);
}

/*-----------------------------------------------------------------------------*/
// Home sequence for the two flag motors
// Blocking function ! timeout set at three seconds if hom sequence fails
// for any reason

void
wdMotorFlagHome()
{
    unsigned long  homeTime = nSysTime + 3000;
    long  lastEncL = 0, lastEncR = 0;

    // Clear encoders
    nMotorEncoder[ wdMotorFlagLeft ] = 0;
    nMotorEncoder[ wdMotorFlagRight ] = 0;

    // Run motors towards the center at low speed
    motor[ wdMotorFlagLeft ] = -32;
    motor[ wdMotorFlagRight ] = -32;
    // We need to wait for some movement
    wait1Msec(500);

    // Now see when they stop
    while( nSysTime < homeTime )
        {
        wait1Msec(50);

        // look for no movement
        if( abs(nMotorEncoder[ wdMotorFlagLeft ] - lastEncL) < 2 )
            motor[ wdMotorFlagLeft ] = -5;
        if( abs(nMotorEncoder[ wdMotorFlagRight ] - lastEncR) < 2 )
            motor[ wdMotorFlagRight ] = -5;

        // this position
        lastEncL = nMotorEncoder[ wdMotorFlagLeft ];
        lastEncR = nMotorEncoder[ wdMotorFlagRight ];
        }

    // Stop motors and then zero encoders at home position
    nMotorEncoder[ wdMotorFlagLeft ] = 0;
    nMotorEncoder[ wdMotorFlagRight ] = 0;
    wait1Msec(200);
    motor[ wdMotorFlagLeft ] = 0;
    motor[ wdMotorFlagRight ] = 0;
}

/*-----------------------------------------------------------------------------*/
// Task to move the flag motors to a new position and then kill power to them

task
wdMotorTask()
{
    // Initialize the motors
    if( wdMotorInit() == 0 )
        {
        // A motor is not connected so we are done
        while(1)
            wait1Msec(10);
        }

    //Home the motors
    wdMotorFlagHome();

    // Now sit in a loop waiting for new commanded positions
    while(1)
        {
        // check for new position request
        if( wdwdMotorFlagRightepositionRequest )
            {
            setMotorTarget(wdMotorFlagLeft, wdMotorTargetL, 32);
            setMotorTarget(wdMotorFlagRight, wdMotorTargetR, 32);
            wait1Msec(250);
            wdwdMotorFlagRightepositionRequest = false;
            }

        // Kill power when in position
        if( abs(nMotorEncoder[wdMotorFlagLeft] - wdMotorTargetL) < 5  )
            motor[ wdMotorFlagLeft ] = 0;
        if( abs(nMotorEncoder[wdMotorFlagRight] - wdMotorTargetR) < 5  )
            motor[ wdMotorFlagRight ] = 0;

        // run slowly
        wait1Msec(50);
        }
}
