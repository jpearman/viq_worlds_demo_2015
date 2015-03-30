/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2015                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     matrix.c                                                     */
/*    Author:     James Pearman                                                */
/*    Created:    20 March 2015                                                */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     TBD - Initial release                              */
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
/*    Utility function for control of the VEX IQ custom 8x32 matrix            */
/*                                                                             */
/*-----------------------------------------------------------------------------*/


#ifndef __WD_MATRIX__
#define __WD_MATRIX__

#define     MATRIX_TEXT_MODE            0x29
#define     MATRIX_TEXT_UPDATE          0x2A
#define     MATRIX_BITMAP_MODE          0x2B
#define     MATRIX_STRING               0x30

#define     MATRIX_MAX_STRING           20
#define     MATRIX_BITMAP               0xA0
#define     MATRIX_BITMAP_LENGTH        32         // Number of led displays * 8

// Text modes
#define     MATRIX_MODE_STATIC          0
#define     MATRIX_MODE_SCROLL_LEFT     1
#define     MATRIX_MODE_SCROLL_RIGHT    2
#define     MATRIX_MODE_TEXT_DN         0
#define     MATRIX_MODE_TEXT_UP         8

// Bitmap modes
#define     MATRIX_MODE_BITMAP_OFF      0
#define     MATRIX_MODE_BITMAP_ON       1

// defined scroll speeds - larger number is slower
#define     MATRIX_MODE_SPEED_FAST      (0<<4)
#define     MATRIX_MODE_SPEED_SLOW3     (1<<4)
#define     MATRIX_MODE_SPEED_SLOW2     (2<<4)
#define     MATRIX_MODE_SPEED_SLOW1     (3<<4)

// Used to shift left the text to help with centering
#define     MATRIX_MODE_OFFSET_0        (0<<4)
#define     MATRIX_MODE_OFFSET_1        (1<<4)
#define     MATRIX_MODE_OFFSET_2        (2<<4)
#define     MATRIX_MODE_OFFSET_3        (3<<4)
#define     MATRIX_MODE_OFFSET_4        (4<<4)
#define     MATRIX_MODE_OFFSET_5        (5<<4)

static  int currentTextMode   = MATRIX_MODE_STATIC;
static  int currentBitmapMode = MATRIX_MODE_BITMAP_OFF;

/*-----------------------------------------------------------------------------*/
// Set text modes
// static, scroll left, scroll right etc.
void
ledMatrixWriteMode( portName port, int mode = 0 )
{
    char    buffer[2];

    // Send new mode and request text update
    buffer[0] = mode;
    genericI2cWrite( port, MATRIX_TEXT_MODE, buffer, 1 );

    // save
    currentTextMode =  mode;
}

/*-----------------------------------------------------------------------------*/
// Send text string to the LED matrix with optional text mode
void
ledMatrixWriteString( portName port, char *str, int mode = MATRIX_MODE_STATIC )
{
    char    buffer[MATRIX_MAX_STRING];
    int     len = strlen(str);

    if( len >= MATRIX_MAX_STRING )
        len = MATRIX_MAX_STRING-1;

    // We need to send string plus null
    strncpy(buffer, str, len);
    buffer[len] = 0;

    // send to display
    genericI2cWrite( port, MATRIX_STRING, buffer, len+1 );

    // Send new mode and request text update
    buffer[0] = mode;
    buffer[1] = 1;
    genericI2cWrite( port, MATRIX_TEXT_MODE, buffer, 2 );

    currentTextMode =  mode;
}

/*-----------------------------------------------------------------------------*/
// Write a new bitmap to the LED matrix
// bitmap MUST be an array of 32 chars - no bounds check on this
void
ledMatrixWriteBitmap( portName port, char *bitmap )
{
    char    buffer[2];

    // send to display
    genericI2cWrite( port, MATRIX_BITMAP, bitmap, MATRIX_BITMAP_LENGTH );

    // Send new mode
    currentBitmapMode = MATRIX_MODE_BITMAP_ON;
    buffer[0] = currentBitmapMode;
    genericI2cWrite( port, MATRIX_BITMAP_MODE, buffer, 1 );
}

/*-----------------------------------------------------------------------------*/
// Cleat the bitmap on the LED matrix
// this sends a bitmap that has all 0's
void
ledMatrixClearBitmap( portName port )
{
    char    buffer[MATRIX_BITMAP_LENGTH];
    int     i;

    for(i=0;i<MATRIX_BITMAP_LENGTH;i++)
        buffer[i] = 0;

    // send to display
    genericI2cWrite( port, MATRIX_BITMAP, buffer, MATRIX_BITMAP_LENGTH );
}

/*-----------------------------------------------------------------------------*/
// Disable use of the LED matrix bitmap
void
ledMatrixDisableBitmap( portName port )
{
    char    buffer[2];

    // Send new mode
    currentBitmapMode = MATRIX_MODE_BITMAP_OFF;
    buffer[0] = currentBitmapMode;
    genericI2cWrite( port, MATRIX_BITMAP_MODE, buffer, 1 );
}

#endif // __WD_MATRIX__
