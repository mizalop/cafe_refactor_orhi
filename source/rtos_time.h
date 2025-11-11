/*******************************************************************************
 *
 *                       ULMA Embedded Solutions
 *                     ---------------------------
 *                        Embedded Design House
 *
 *                     http://www.ulmaembedded.com
 *                        info@ulmaembedded.com
 *
 *******************************************************************************
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *******************************************************************************
 * @file rtos_time.h
 * @author oarregi
 * @revision 
 * @brief  timing definitions
 * @creation date   2014-03-18
 * @revision date	
 * @version  V0.0
 ******************************************************************************/

#ifndef RTOS_TIME_H_
#define RTOS_TIME_H_

#include "FreeRTOS.h"


#define TIME_1MSEC    	( 1 / portTICK_RATE_MS)
#define TIME_5MSEC    	( 5 / portTICK_RATE_MS)
#define TIME_10MSEC		(10 / portTICK_RATE_MS)
#define TIME_20MSEC		(20 / portTICK_RATE_MS)
#define TIME_30MSEC		(30 / portTICK_RATE_MS)
#define TIME_40MSEC		(40 / portTICK_RATE_MS)
#define TIME_50MSEC		(50 / portTICK_RATE_MS)
#define TIME_60MSEC		(60 / portTICK_RATE_MS)
#define TIME_70MSEC		(70 / portTICK_RATE_MS)
#define TIME_80MSEC		(80 / portTICK_RATE_MS)
#define TIME_90MSEC		(90 / portTICK_RATE_MS)

#define TIME_100MSEC	(100 / portTICK_RATE_MS)
#define TIME_200MSEC	(200 / portTICK_RATE_MS)
#define TIME_300MSEC	(300 / portTICK_RATE_MS)
#define TIME_400MSEC	(400 / portTICK_RATE_MS)
#define TIME_500MSEC	(500 / portTICK_RATE_MS)
#define TIME_600MSEC	(600 / portTICK_RATE_MS)
#define TIME_700MSEC	(700 / portTICK_RATE_MS)
#define TIME_800MSEC	(800 / portTICK_RATE_MS)
#define TIME_900MSEC	(900 / portTICK_RATE_MS)

#define TIME_1SEC		( 1000 / portTICK_RATE_MS)
#define TIME_2SEC		( 2000 / portTICK_RATE_MS)
#define TIME_3SEC		( 3000 / portTICK_RATE_MS)
#define TIME_4SEC		( 4000 / portTICK_RATE_MS)
#define TIME_5SEC		( 5000 / portTICK_RATE_MS)
#define TIME_6SEC		( 6000 / portTICK_RATE_MS)
#define TIME_7SEC		( 7000 / portTICK_RATE_MS)
#define TIME_8SEC		( 8000 / portTICK_RATE_MS)
#define TIME_9SEC		( 9000 / portTICK_RATE_MS)
#define TIME_10SEC		(10000 / portTICK_RATE_MS)
#define TIME_20SEC		(20000 / portTICK_RATE_MS)
#define TIME_25SEC  	(25000 / portTICK_RATE_MS)
#define TIME_30SEC  	(30000 / portTICK_RATE_MS)

#define TIME_1MIN		( 60000 / portTICK_RATE_MS)
#define TIME_2MIN		(120000 / portTICK_RATE_MS)
#define TIME_5MIN		(300000 / portTICK_RATE_MS)


#endif /* RTOS_TIME_H_ */
