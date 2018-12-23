#ifndef SYSTEM_TIME_H
#define SYSTEM_TIME_H

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * MACROS
 */
#define	SECOND_PER_DAY             86400UL  // 24 hours * 60 minutes * 60 seconds

#define	IsLeapYear(yr)	(!((yr) % 400) || (((yr) % 100) && !((yr) % 4)))

/*********************************************************************
 * TYPEDEFS
 */

typedef enum{
week_Monday = 0,
week_Tuesday,
week_Wednesday,
week_Thursday,
week_Friday,
week_Saturday,
week_Sunday
}week_day_t;
 
// number of seconds since 0 hrs, 0 minutes, 0 seconds, on the
// 1st of January 2000 UTC
typedef uint32_t UTCTime;

 typedef struct{
    uint16_t    year;   // 2000+
    uint8_t     month;  // 0-11
    uint8_t     day;    // 0-30
    uint8_t     weekday;// 0-Monday
    uint8_t     hour;   // 0-23
    uint8_t     minutes;// 0-59
    uint8_t     seconds;// 0-59
}UTCTime_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

  /*
   * Updates the OSAL clock and Timers from the MAC 320us timer tick.
   */
  void sysTimeUpdateSecond(void );

  /*
   * Set the new time.  This will only set the seconds portion
   * of time and doesn't change the factional second counter.
   *     newTime - number of seconds since 0 hrs, 0 minutes,
   *               0 seconds, on the 1st of January 2000 UTC
   */
  void sys_setClock( UTCTime newTime );
  void sys_setClock_t(UTCTime_t *tm);

  /*
   * Gets the current time.  This will only return the seconds
   * portion of time and doesn't include the factional second counter.
   *     returns: number of seconds since 0 hrs, 0 minutes,
   *              0 seconds, on the 1st of January 2000 UTC
   */
  UTCTime sys_getClock(void );

  /*
   * Converts UTCTime to UTCTime_t
   *
   * secTime - number of seconds since 0 hrs, 0 minutes,
   *          0 seconds, on the 1st of January 2000 UTC
   * tm - pointer to breakdown struct
   */
  void sys_ConvertUTCTime( UTCTime_t *tm, UTCTime secTime );

  void sys_ConvertTimeOnly( UTCTime_t *tm, UTCTime secTime );

  /*
   * Converts UTCTime_t to UTCTime (seconds since 00:00:00 01/01/2000)
   *
   * tm - pointer to UTC time struct
   */
   UTCTime sys_ConvertUTCSecs( UTCTime_t *tm );
   

/*********************************************************************
*********************************************************************/

#endif /* OSAL_TIMERS_H */
