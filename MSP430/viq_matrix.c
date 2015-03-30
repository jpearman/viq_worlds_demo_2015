/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2015                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     viq_matrix.c                                                 */
/*    Author:     James Pearman                                                */
/*    Created:    19 March 2015                                                */
/*                                                                             */
/*    Revisions:                                                               */
/*                V1.00     19 March 2015 - Initial release                    */
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

#include "viq.h"
#include "viq_matrix.h"
#include "font.h"

// Offsets into user register space
#define DATA_M_MODE     5   ///< 0x29 Message mode, 0=static, 1=left scroll etc.
#define DATA_M_FLAG     6   ///< 0x2A New message available
#define DATA_B_MODE     7   ///< 0x2B Bitmap mode
#define DATA_MESSAGE   12   ///< Message for matrix 0x30 - 0x3F

// Maximum length of text
#define MESSAGE_MAX    20

// shift chars down or leave alone
#define TEXT_POSITION_UP    1
#define TEXT_POSITION_DN    0

static  int8_t  matrix_text[MESSAGE_MAX];
static  int16_t matrix_first = 0;
static  int16_t matrix_text_length;
static  int8_t  matrix_bitmap_mode;
static  int8_t  matrix_text_position;

// Buffer for bitmapped graphics
static  uint8_t bitmapBuffer[MATRIX_COUNT * 8];

/*-----------------------------------------------------------------------------*/
/** @brief     Initialize USCI A0 controller for SPI                           */
/*-----------------------------------------------------------------------------*/

void
viq_matrix_init()
{
    register uint8_t        *data;
    int16_t     i;

    // SPI output
    P1OUT  |= ( SPI_CLK_OUT_PIN | SPI_DATA_OUT_PIN );
    P1SEL  |= ( SPI_CLK_OUT_PIN | SPI_DATA_OUT_PIN );
    P1SEL2 |= ( SPI_CLK_OUT_PIN | SPI_DATA_OUT_PIN );

    UCA0CTL1 |= UCSWRST;
    UCA0CTL0  = UCMST + UCMODE_0 + UCSYNC + UCCKPH;

    // 16MHz / 16 = 1MHz
    UCA0CTL1 |= UCSSEL_2;
    UCA0BR0  = 0x10;
    UCA0BR1  = 0;

    UCA0CTL1 &= ~UCSWRST;

    // Clear bitmap
    for(i=0;i<MATRIX_COUNT * 8;i++)
        bitmapBuffer[i] = 0;

    // clear message
    for(i=0;i<MESSAGE_MAX;i++)
        matrix_text[i] = 0x20;
    matrix_text[5] = '_';
    matrix_text_length = viq_matrix_strlen( matrix_text );

    // clear - leaves dot in column 8
    for(i=0;i<9;i++) {
        viq_matrix_write();
        __delay_cycles(200);
        }

    // Default
    viq_matrix_strcpy("HELLO ROBOTEERS     ");
    matrix_text_length = viq_matrix_strlen( matrix_text );

    // Get pointer for user data area
    data = viq_i2c_get_data_ptr();
    data[DATA_M_MODE] = 0x21;
}

/*-----------------------------------------------------------------------------*/
/** @brief     Copy string to matrix message                                   */
/*-----------------------------------------------------------------------------*/

void
viq_matrix_strcpy( int8_t *str )
{
    int16_t  i;

    for(i=0;i<MESSAGE_MAX;i++)
        {
        if(*str != 0 )
            matrix_text[i] = *str++;
        else
            matrix_text[i] = 0;
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief     Calculate length of matrix message                              */
/*-----------------------------------------------------------------------------*/

int16_t
viq_matrix_strlen( int8_t *str )
{
    int16_t  i;
    int16_t  l = 0;

    for(i=0;i<MESSAGE_MAX;i++)
        {
        if(matrix_text[i] != 0)
            l++;
        else
            return(l);
        }

    return(l);
}

/*-----------------------------------------------------------------------------*/
/** @brief     Copy data to bitmap buffer                                      */
/*-----------------------------------------------------------------------------*/

void
viq_matrix_bitmapcpy( int8_t *data )
{
    int16_t  i;

    for(i=0;i<MATRIX_COUNT * 8;i++)
        bitmapBuffer[i] = *data++;
}

/*-----------------------------------------------------------------------------*/
/** @brief     Task to send data to matrix - run fast, depends pixel count     */
/*-----------------------------------------------------------------------------*/

void
viq_matrix_task(void)
{
    viq_matrix_write();
}

/*-----------------------------------------------------------------------------*/
/** @brief     Task to read commands from VEX IQ brain                         */
/*-----------------------------------------------------------------------------*/

void
viq_matrix_update(void)
{
    register uint8_t        *data;
             uint8_t         matrix_text_mode;
    static   uint16_t        delay = 0;

    // Get pointer for user data area
    data = viq_i2c_get_data_ptr();

    // Update message
    if( data[DATA_M_FLAG] ) {
        // copy new message
        viq_matrix_strcpy( &data[DATA_MESSAGE] );
        // recalculate message length
        matrix_text_length = viq_matrix_strlen( matrix_text );
        data[DATA_M_FLAG] = 0;
        }

    // get message mode
    matrix_text_mode   = data[DATA_M_MODE];
    matrix_bitmap_mode = data[DATA_B_MODE];

    // bit 3 now sets text position
    matrix_text_position = ( matrix_text_mode & 0x08) ? 1 : 0;

    // scrolling
    switch(matrix_text_mode & 0x07)
        {
        // static
        case    0:
            matrix_first = (matrix_text_mode >> 4) & 0x0f;
            delay = 0;
            break;
        case    1:
            // H scroll left - speed variable
            if(delay++ >= (matrix_text_mode >> 4) & 0x0f) {
                matrix_first = (matrix_first + 1) % (matrix_text_length * 6);
                delay = 0;
                }
            break;
        case    2:
            // H scroll right - speed variable
            if(delay++ >= (matrix_text_mode >> 4) & 0x0f) {
                matrix_first--;
                if (matrix_first == -1)
                    matrix_first = matrix_text_length * 6 - 1;
                delay = 0;
                }
            break;
        default:
            break;
        }

    // Copy bitmap buffer
    if( matrix_bitmap_mode )
        {
        // Get pointer for extended user data area
        data = viq_i2c_get_extended_data_ptr();
        viq_matrix_bitmapcpy(data);
        }
}

/*-----------------------------------------------------------------------------*/
/** @brief     Function that sends pixels for one column to each matrix        */
/*-----------------------------------------------------------------------------*/

void
viq_matrix_write()
{
    static int16_t x, x_div, x_mod;
    static int8_t index=0, offset, Column=1;
    const int16_t COLUMN_COUNT = MATRIX_COUNT * 8;
    int16_t column_length = matrix_text_length * 6;
    uint8_t row_data;

    // Latch Pulse
    P1OUT = P1OUT ^ DATA_LATCH_PIN;
    P1OUT = P1OUT ^ DATA_LATCH_PIN;

    // Sending MATRIX_COUNT portions of data via SPI
    // to the differents matrices. They use one latch so
    // we have to send the respective data to each of them before we pulse it.
    // Offset is increased with 8 because we send data to a different matrix
    // Columns (for 3 matrices) are in that order 0, 8, 16, 1, 9, 17, 2, 10, 18 etc.
    for (offset=0; offset<COLUMN_COUNT; offset=offset+8)
        {
        // calculate the number of the column (IN THE MESSAGE BUFFER) based on:
        // matrix_first - offset caused by the slided text
        // offset - offset caused by number of the different matrix
        // index - offset caused by number of the different columns in one matrix
        // matrix_text_length - length of the message (in columns) to avoid displaying unexisting columns
        // (ex. we have 120 columns [0..119] and try do display 120 --> we will display the 0 column)
        x = (matrix_first + offset + index) % (column_length);
        x_div = x/6;
        x_mod = x%6;

        // Get bitmap data
        if( matrix_bitmap_mode )
            row_data = bitmapBuffer[offset + index];
        else
            row_data = 0x00;

        // Get text data - space between characters
        if ((x_mod != 5) && (matrix_text[x_div] != 0))
            {
            // default is now to shift by one line all text
            if(matrix_text_position)
                row_data |= (FontLookup[matrix_text[x_div] - 0x20][x_mod]);
            else
                row_data |= (FontLookup[matrix_text[x_div] - 0x20][x_mod]) << 1;
            }

        // Send row
        while(!(IFG2 & UCA0TXIFG));
        UCA0TXBUF = row_data;

        // send column
        while(!(IFG2 & UCA0TXIFG));
        UCA0TXBUF = Column;
        }

    // Increase the number of the column for each of the matrices
    if (++index & 7)
        Column = Column<<1; // Activate next column
    else
        {
        // if index reach value 8 start over from 0, and the column from 1
        Column = 1;
        index = 0;
        }
}
