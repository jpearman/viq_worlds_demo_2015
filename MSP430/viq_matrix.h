/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*                        Copyright (c) James Pearman                          */
/*                                   2015                                      */
/*                            All Rights Reserved                              */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
/*                                                                             */
/*    Module:     viq_matrix.h                                                 */
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

#ifndef __VIQ_MATRIX___
#define __VIQ_MATRIX___

/*-----------------------------------------------------------------------------*/
/// @file    viq_matrix.h
/// @brief   Vex IQ sensor Olimex matrix control header
/*-----------------------------------------------------------------------------*/

#define DATA_LATCH_PIN      BIT1
#define SPI_CLK_OUT_PIN     BIT4
#define SPI_DATA_OUT_PIN    BIT2
#define SPI_DATA_IN_PIN     BIT1    // not used - we leave as GPIO

#define MATRIX_COUNT        4       // Count of the connected led matrices

#ifdef __cplusplus
extern "C" {
#endif

void    viq_matrix_init(void);
void    viq_matrix_task(void);
void    viq_matrix_update(void);
void    viq_matrix_strcpy( int8_t *str );
int16_t viq_matrix_strlen( int8_t *str );
void    viq_matrix_write(void);

#ifdef __cplusplus
}
#endif

#endif  // __VIQ_MATRIX___
