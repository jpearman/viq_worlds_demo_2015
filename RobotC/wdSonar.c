/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2015                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     wdSonarLib.c                                                 */
/*    Author:     James Pearman                                                */
/*    Created:    8 April 2015                                                 */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     tbd - Initial release                              */
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
/*    Sonar control for the 2015 VEX IQ worlds demo bot                        */
/*-----------------------------------------------------------------------------*/

#ifndef wdSonarPort
#define wdSonarPort     port7
#endif

static  float distanceFiltered = 0;

#define DETECT_THRESHOLD    800

/*-----------------------------------------------------------------------------*/
// Check that sonar at index is present
bool
wdSonarCheck(tSensors index)
{
    TVexIQDeviceTypes   type;
    TDeviceStatus       status;
    short               ver;

    getVexIqDeviceInfo( (portName)index, type, status, ver );
    if( type == vexIQ_SensorSONAR )
        return(true);
    else
        return(false);
}

/*-----------------------------------------------------------------------------*/
// Return filtered sonar distance as an int
int
wdSonarGetDistance()
{
    return( (int) distanceFiltered );
}

/*-----------------------------------------------------------------------------*/
// Return true if below my preset threshold
bool
wdGetSonarState()
{
    if(distanceFiltered > 0 )
        {
        return( distanceFiltered < DETECT_THRESHOLD );
        }
    else
        return(false);
}

/*-----------------------------------------------------------------------------*/
// Task to read and filter the sonar
task
wdSonarTask()
{
    int currentDistance;

    // check sonar is plugged in
    if( wdSonarCheck(wdSonarPort) == false )
        {
        distanceFiltered = -1;
        return;
        }

    // setup sensor
    SensorType[ wdSonarPort ] = vexIQ_SensorSONAR;

    // Init sonar
    setDistanceMaxRange( wdSonarPort, 1200 );
    setDistanceFilterFactor( wdSonarPort, 4 );
    setDistanceBrightnessThreshold( wdSonarPort, 20 );

    // read and filter
    while(true)
        {
        currentDistance = getDistanceValue(wdSonarPort);

        if( currentDistance > 1000 )
            distanceFiltered = 1000;
        else
            // simple IIR filter
            distanceFiltered = (distanceFiltered * 0.8) + (currentDistance * 0.2);

        sleep(100);
        }
}
