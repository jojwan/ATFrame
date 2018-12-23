/*********************************************************************
 * INCLUDES
 */

#include <stdint.h>
#include <string.h>

#include "sys_time.h"


/*********************************************************************
 * MACROS
 */

#define	YearLength(yr)	(IsLeapYear(yr) ? 366 : 365)

/*********************************************************************
 * CONSTANTS
 */
#define	BEGYEAR	        2000     // UTC started at 00:00:00 January 1, 2000

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */
static uint8_t monthLength( uint8_t lpyr, uint8_t mon );

unsigned int  sys_utctime = 0;

#define  g_sys_utctime     sys_utctime
/*********************************************************************
 * FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn      sysTimeUpdate
 *
 * @brief   add 1 second
 *
 * @param   None.
 *
 * @return  None.
 */
 
void sysTimeUpdateSecond( void )
{
	g_sys_utctime ++;
}

/*********************************************************************
 * @fn      sys_setClock
 *
 * @brief   Set the new time.  This will only set the seconds portion
 *          of time and doesn't change the factional second counter.
 *
 * @param   newTime - number of seconds since 0 hrs, 0 minutes,
 *                    0 seconds, on the 1st of January 2000 UTC
 *
 * @return  none
 */
void sys_setClock( UTCTime newTime )
{
	g_sys_utctime = newTime;
}

void sys_setClock_t(UTCTime_t *tm)
{
    g_sys_utctime = sys_ConvertUTCSecs(tm);
}

/*********************************************************************
 * @fn      sys_getClock
 *
 * @brief   Gets the current time.  This will only return the seconds
 *          portion of time and doesn't include the factional second
 *          counter.
 *
 * @param   none
 *
 * @return  number of seconds since 0 hrs, 0 minutes, 0 seconds,
 *          on the 1st of January 2000 UTC
 */
UTCTime sys_getClock( void )
{
  return ( g_sys_utctime );
}

/*********************************************************************
 * @fn      sys_ConvertUTCTime
 *
 * @brief   Converts UTCTime to UTCTime_t
 *
 * @param   tm - pointer to breakdown struct
 *
 * @param   secTime - number of seconds since 0 hrs, 0 minutes,
 *          0 seconds, on the 1st of January 2000 UTC
 *
 * @return  none
 */
void sys_ConvertUTCTime( UTCTime_t *tm, UTCTime secTime )
{
  // calculate the time less than a day - hours, minutes, seconds
  {
    uint32_t day = secTime % SECOND_PER_DAY;
    tm->seconds = day % 60UL;
    tm->minutes = (day % 3600UL) / 60UL;
    tm->hour = day / 3600UL;
  }

  // Fill in the calendar - day, month, year
  {
    uint16_t numDays = secTime / SECOND_PER_DAY;
    tm->weekday = (numDays + 6) % 7;// 2000-1-1 is Staturday
    tm->year = BEGYEAR;
    while ( numDays >= YearLength( tm->year ) )
    {
      numDays -= YearLength( tm->year );
      tm->year++;
    }

    tm->month = 0;
    while ( numDays >= monthLength( IsLeapYear( tm->year ), tm->month ) )
    {
      numDays -= monthLength( IsLeapYear( tm->year ), tm->month );
      tm->month++;
    }

    tm->day = numDays;
  }
}

void sys_ConvertTimeOnly( UTCTime_t *tm, UTCTime secTime )
{
    uint32_t day = secTime % SECOND_PER_DAY;
    tm->seconds = day % 60UL;
    tm->minutes = (day % 3600UL) / 60UL;
    tm->hour = day / 3600UL;
}
/*********************************************************************
 * @fn      monthLength
 *
 * @param   lpyr - 1 for leap year, 0 if not
 *
 * @param   mon - 0 - 11 (jan - dec)
 *
 * @return  number of days in specified month
 */
static uint8_t monthLength( uint8_t lpyr, uint8_t mon )
{
  uint8_t days = 31;

  if ( mon == 1 ) // feb
  {
    days = ( 28 + lpyr );
  }
  else
  {
    if ( mon > 6 ) // aug-dec
    {
      mon--;
    }

    if ( mon & 1 )
    {
      days = 30;
    }
  }

  return ( days );
}

/*********************************************************************
 * @fn      sys_ConvertUTCSecs
 *
 * @brief   Converts a UTCTime_t to UTCTime
 *
 * @param   tm - pointer to provided struct
 *
 * @return  number of seconds since 00:00:00 on 01/01/2000 (UTC)
 */
UTCTime sys_ConvertUTCSecs( UTCTime_t *tm )
{
  uint32_t seconds;

  /* Seconds for the partial day */
  seconds = (((tm->hour * 60UL) + tm->minutes) * 60UL) + tm->seconds;

  /* Account for previous complete days */
  {
    /* Start with complete days in current month */
    uint16_t days = tm->day;

    /* Next, complete months in current year */
    {
      int8_t month = tm->month;
      while ( --month >= 0 )
      {
        days += monthLength( IsLeapYear( tm->year ), month );
      }
    }

    /* Next, complete years before current year */
    {
      uint16_t year = tm->year;
      while ( --year >= BEGYEAR )
      {
        days += YearLength( year );
      }
    }

    /* Add total seconds before partial day */
    seconds += (days * SECOND_PER_DAY);
  }

  return ( seconds );
}

/*********************************************************************
*********************************************************************/
