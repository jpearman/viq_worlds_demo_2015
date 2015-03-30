/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2015                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     wdMatrixUtil.c                                               */
/*    Author:     James Pearman                                                */
/*    Created:    27 March 2015                                                */
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
/*    Some more LED matrix useful functions                                    */
/*                                                                             */
/*-----------------------------------------------------------------------------*/


#ifndef __WD_LED_UTIL__
#define __WD_LED_UTIL__

#pragma systemFile

/*-----------------------------------------------------------------------------*/
// Useful bitmaps
static char bitmap_none[32] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static char bitmap_128_1[32] =
{
0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,
0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,
0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,
0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA
};

static char bitmap_128_2[32] =
{
0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,
0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,
0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,
0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55
};

static char bitmap_vex[32] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x60,
0x80,0x80,0x60,0x1C,0x00,0x00,0x3E,0x49,
0x49,0x41,0x41,0x00,0x00,0x84,0x48,0x30,
0x30,0x48,0x84,0x00,0x00,0x00,0x00,0x00
};

static char bitmap_vexiq[32] = {
0x1C,0x60,0x80,0x80,0x60,0x1C,0x00,0x3E,
0x49,0x49,0x41,0x41,0x00,0x84,0x48,0x30,
0x30,0x48,0x84,0x00,0x80,0x80,0xE4,0x9C,
0x04,0x04,0x60,0x98,0x84,0xA4,0x44,0xB8
};

/*-----------------------------------------------------------------------------*/
// Clear the display
// Write nul text string and disable boitmap
void
wdLedClear( portName port )
{
    ledMatrixWriteString( port, " " );
    ledMatrixDisableBitmap( port );
}

/*-----------------------------------------------------------------------------*/
// Create centered text string
void
wdLedTextCenter( char *text, char *buffer, int *offset )
{
    int  i;
    int  tl, sl;
    const int matrixLength = MATRIX_BITMAP_LENGTH;

    // matrix is 32 pixels wide
    // Text chars are 5 pixelss wide
    // space is 1 pixel wide

    // calculate text width - ignore space after last character
    // VEX would be 17 pixels wide
    tl = (strlen( text ) * 6) - 1;

    // calculate number of leading spaces to center
    // eg VEX would be ((16 - 8) + 5)/2 = 2
    // so two extra spaces
    sl = ( (matrixLength/2 - tl/2) + 5 ) / 6;
    // limit extra space
    if( sl < 0 || sl > 6 ) sl = 0;

    // add spaces and text to buffer
    for(i=0;i<sl;i++)
        buffer[i] = ' ';
    strcpy(&buffer[sl],text );

    // Calculate offset to allow centered text
    *offset = (sl * 6) - ((matrixLength/2) - tl/2);
    if( *offset < 0 )
        *offset = 0;
}

/*-----------------------------------------------------------------------------*/
// Display centered text
// 5 characters max
void
wdLedCenteredText( portName port, char *text  )
{
    char buffer[32];
    int offset = 0;

    wdLedTextCenter( text, buffer, &offset );

    ledMatrixWriteString( port, buffer, (offset << 4) | MATRIX_MODE_STATIC );
}


/*-----------------------------------------------------------------------------*/
// Flash text with optional centering
void
wdLedFlashText( portName port, char *text, int loops = 5, bool center = true )
{
    char buffer[32];
    int offset = 0;
    int i;

    if( center )
        wdLedTextCenter( text, buffer, &offset );
    else
        {
        strcpy( buffer, text );
        // fill remaining buffer with spaces
        for(i=strlen(text);i<6;i++)
            buffer[i] = ' ';
        }

    for(i=0;i<loops;i++) {
        // clear text
        ledMatrixWriteString( port, " " );
        wait1Msec(200);

        // write string
        ledMatrixWriteString( port, buffer, (offset << 4) | MATRIX_MODE_STATIC );
        wait1Msec(200);
      }
}

/*-----------------------------------------------------------------------------*/
// Flash the text "VEX"
void
wdLedFlashVex(portName port)
{
    //buffer[3] = 126; // shifted E
    wdLedFlashText( port, "VEX" );
}

/*-----------------------------------------------------------------------------*/
// Flash a bitmap
void
wdLedFlashBitmap( portName port, char *bitmap, int loops = 5 )
{
    int     i;

    for(i=0;i<loops;i++) {
         ledMatrixWriteBitmap( port, bitmap );
         wait1Msec(200);
         ledMatrixDisableBitmap( port );
         wait1Msec(200);
      }
}

/*-----------------------------------------------------------------------------*/
// Toggle between two bitmaps
void
wdLedFlashBitmap( portName port, char *bitmapa, char *bitmapb, int loops = 5 )
{
    int     i;

    for(i=0;i<loops;i++) {
         ledMatrixWriteBitmap( port, bitmapa );
         wait1Msec(200);
         ledMatrixWriteBitmap( port, bitmapb );
         wait1Msec(200);
      }
}

/*-----------------------------------------------------------------------------*/
// Curtain Effect in bitmap
void
wdLedCurtainBitmap( portName port, char *bitmap, bool vis = true )
{
    char    buffer[32];
    int     i, col;

    if( vis )
        {
        for(col=16;col>=0;col--)
            {
            for(i=0;i<16;i++)
                {
                buffer[i]    = bitmap[i];
                buffer[31-i] = bitmap[31-i];

                if( i < col )
                    {
                    buffer[i]    |= 0xff;
                    buffer[31-i] |= 0xff;
                    }
                }
            ledMatrixWriteBitmap( port, buffer );
            wait1Msec(50);
            }
        }
    else
        {
        for(col=0;col<17;col++)
            {
            for(i=0;i<16;i++)
                {
                buffer[i]    = bitmap[i];
                buffer[31-i] = bitmap[31-i];

                if( i < col )
                    {
                    buffer[i]    |= 0xff;
                    buffer[31-i] |= 0xff;
                    }
                }
            ledMatrixWriteBitmap( port, buffer );
            wait1Msec(50);
            }
        }
    }

#endif // __WD_LED_UTIL__
