#ifndef TIMES_HPP
#define TIMES_HPP
#include <QString>
/**
 * @brief Convert days in hours.
 * @param days Input number of days.
 * @param remaining Fractional part of hours (output).
 * @return Number of hours.
 */
double dayToHours(double days, double &remaining);

/**
 * @brief Convert days in minutes.
 * @param days Input number of days.
 * @param remaining Fractional part of minutes (output).
 * @return Number of minutes.
 */
double dayToMins(double days, double &remaining);

/**
 * @brief Convert days in seconds.
 * @param days Input number of days.
 * @param remaining Fractional part of seconds (output).
 * @return Number of seconds.
 */
double dayToSecs(double days, double &remaining);

/**
 * @brief Convert days in milliseconds.
 * @param days Input number of days.
 * @param remaining Fractional part of milliseconds (output).
 * @return Number of milliseconds.
 */
double dayToMSecs(double days, double &remaining);

/**
 * @brief Convert hours in minutes.
 * @param hours Input number of hours.
 * @param remaining Fractional part of minutes (output).
 * @return Number of minutes.
 */
double hourToMins(double hours, double &remaining);

/**
 * @brief Convert hours in seconds.
 * @param hours Input number of hours.
 * @param remaining Fractional part of seconds (output).
 * @return Number of seconds.
 */
double hourToSecs(double hours, double &remaining);

/**
 * @brief Convert hours in milliseconds.
 * @param hours Input number of hours.
 * @param remaining Fractional part of milliseconds (output).
 * @return Number of milliseconds.
 */
double hourToMSecs(double hours, double &remaining);

/**
 * @brief Convert minutes in seconds.
 * @param mins Input number of minutes.
 * @param remaining Fractional part of seconds (output).
 * @return Number of seconds.
 */
double minToSecs(double mins, double &remaining);

/**
 * @brief Convert minutes in milliseconds.
 * @param mins Input number of minutes.
 * @param remaining Fractional part of milliseconds (output).
 * @return Number of milliseconds.
 */
double minToMSecs(double mins, double &remaining);

/**
 * @brief Convert seconds in milliseconds.
 * @param mins Input number of seconds.
 * @param remaining Fractional part of milliseconds (output).
 * @return Number of milliseconds.
 */
double secToMSecs(double secs, double &remaining);

/**
 * @brief Convert milliseconds in days.
 * @param ms Input number of milliseconds.
 * @param remaining Fractional part of days (output).
 * @return Number of days.
 */
double msecsToDays(qint64 ms, double &remaining);

/**
 * @brief Convert milliseconds in hours.
 * @param ms Input number of milliseconds.
 * @param remaining Fractional part of hours (output).
 * @return Number of hours.
 */
double msecsToHours(qint64 ms, double &remaining);

/**
 * @brief Convert milliseconds in minutes.
 * @param ms Input number of milliseconds.
 * @param remaining Fractional part of minutes (output).
 * @return Number of minutes.
 */
double msecsToMins(qint64 ms, double &remaining);

/**
 * @brief Convert milliseconds in seconds.
 * @param ms Input number of milliseconds.
 * @param remaining Fractional part of seconds (output).
 * @return Number of seconds.
 */
double msecsToSecs(qint64 ms, double &remaining);

/**
 * @brief Computes seconds and milliseconds from fractional part of minutes.
 * @param fmt Format of the result string.
 * @param min_fraction Fractional number of minutes.
 * @param s Output number of seconds.
 * @param z Output number of milliseconds.
 */
void fromMinutes(QString fmt, double min_fraction, double &s, double &z);

/**
 * @brief Computes minutes, seconds and milliseconds from fractional part of hour.
 * @param fmt Format of the result string.
 * @param hour_fraction Fractional number of hour.
 * @param m Output number of minutes.
 * @param s Output number of seconds.
 * @param z Output number of milliseconds.
 */
void fromHours(QString fmt, double hour_fraction, double &m, double &s, double &z);
/**
 * @brief Computes hours, minutes, seconds and milliseconds from fractional part of day.
 * @param fmt Format of the result string.
 * @param min_fraction Fractional number of day.
 * @param h Output number of hours.
 * @param m Output number of minutes.
 * @param s Output number of seconds.
 * @param z Output number of milliseconds.
 */
void fromDays(QString fmt, double day_fraction, double &h, double &m, double &s, double &z);

/**
 * @brief Fancy string representation of elapsed in milliseconds.
 * @param fmt Format of the result string.
 * @param msecs Milliseconds elpased.
 * @return Formatted string.
 * @details
 * The method replaces tags from <i>fmt</i> by appropriate values. Supported tags are the following:
 *  - %d, number of days.
 *  - %h, number of hours.
 *  - %m, number of minutes.
 *  - %s, number of seconds.
 *  - %z, number of milliseconds.
 *
 * All other characters are not modified.
 */
QString msecsToTimeFormat(qint64 msecs, QString fmt);

#endif // TIMES_HPP
