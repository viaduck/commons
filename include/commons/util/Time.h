/*
 * Copyright (C) 2015-2023 The ViaDuck Project
 *
 * This file is part of Commons.
 *
 * Commons is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Commons is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Commons.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMONS_TIME_H
#define COMMONS_TIME_H

#include <commons/util/Except.h>

#include <chrono>
#include <iomanip>
#include <sstream>

#ifdef WIN32
    /*
     * Even though we just need windows.h, winsock.h has to be included in case it is later used,
     * since winsock.h must always be included before windows.h
     */

    #include <winsock2.h>
    #include <windows.h>
#endif

DEFINE_ERROR(time, base_error);

/**
 * Provides a thread-safe, cross-platform way of creating and formatting timestamps with millisecond precision.
 */
class Time {
public:
    explicit Time(int64_t timestamp, bool utc = true) : mIsUTC(utc) {
        // given timestamp
        set(timestamp, utc);
    }

    static inline int64_t now() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    static Time nowUTC() {
        return Time(now(), true);
    }

    static Time nowLocal() {
        return Time(now(), false);
    }

    /**
     * Formats the timestamp in given format. See std::put_time for format specification.
     *
     * Note: The maximum resolution of the format is seconds.
     * This function is thread-safe.
     */
    std::string format(const char *fmt) {
        // format s_tm
        std::stringstream bruce;
        bruce << std::put_time(&mTime, fmt);

        return bruce.str();
    }

    /**
     * Same as format, but c++11-aware.
     * Supports %k for milliseconds and %z for timezone offset.
     * If this is non-UTC, %z resolves to local timezone, otherwise %z resolves to UTC +0000.
     *
     * Note: this function is thread-safe.
     */
    std::string formatFull(std::string fmt) {
        // pre-format fmt with millis using %k
        fmt = format_k(fmt, mMillis);

        if (mIsUTC) {
            // prevent %z from resolving to local timezone by manually formatting it to +0000 for UTC on all platforms
            fmt = format_z(fmt, 0);
        }
        else {
            // %z already resolves to local timezone except on windows mingw-w64, which we handle manually
#ifdef WIN32
            // get timezone info
            TIME_ZONE_INFORMATION tzinfo;
            long bias;

            /*
             * Note:
             * - Windows defines bias as UTC=localtime+bias, therefore we flip the sign
             * - StandardBias or DaylightBias is the additional bias introduced by standard time/daylight savings time
             * - In unknown case, use only timezone default bias
             */
            switch(GetTimeZoneInformation(&tzinfo)) {
                case TIME_ZONE_ID_UNKNOWN:
                    bias = -tzinfo.Bias;
                    break;
                case TIME_ZONE_ID_STANDARD:
                    bias = -(tzinfo.Bias + tzinfo.StandardBias);
                    break;
                case TIME_ZONE_ID_DAYLIGHT:
                    bias = -(tzinfo.Bias + tzinfo.DaylightBias);
                    break;
                default:
                    throw time_error("Failed to get timezone data");
            }

            // pre-format fmt with timezone using %z
            fmt = format_z(fmt, bias);
#endif
        }

        return format(fmt.c_str());
    }

    /**
     * Formats the UTC timestamp in ISO 8601 format.
     *
     * Note:
     * Format is <date>T<time>Z with <date> YYYY-MM-DD and <time> HH:MM:SS.SSS
     * This function is thread-safe.
     */
    std::string formatIso8601() {
        /*
         * Note: On windows/wine using mingw-w64, std::put_time does not handle the C++11 cases of %T and %F,
         * despite the C++ standard and documentation stating otherwise. Replace them by their long versions instead.
         */
        return formatFull("%Y-%m-%dT%H:%M:%S.%kZ");
    }

protected:
    static void gmtime_safe(const time_t *time, tm *result) {
#ifdef WIN32
        errno_t res = gmtime_s(result, time);
        if (res != 0)
            throw time_error(std::to_string(res));
#else
        if (!gmtime_r(time, result))
            throw time_error(std::to_string(errno));
#endif
    }

    static void localtime_safe(const time_t *time, tm *result) {
#ifdef WIN32
        errno_t res = localtime_s(result, time);
        if (res != 0)
            throw time_error(std::to_string(res));
#else
        if (!localtime_r(time, result))
            throw time_error(std::to_string(errno));
#endif
    }

    /**
     * Replaces all instances of "%k" in fmt with the millisecond part of timestamp
     */
    static std::string format_k(std::string fmt, int64_t millis) {
        // extract ms part to string with leading zeroes
        std::string s_millis = std::to_string(millis);
        s_millis.insert(0, 3 - s_millis.size(), '0');

        // while %k exists in fmt, replace it with s_millis
        size_t i_k;
        while ((i_k = scan_x(fmt, 'k')) != std::string::npos)
            fmt = replace_at(fmt, i_k, s_millis);

        return fmt;
    }

    /**
     * Replaces all instances of "%z" with timezone offset "+-HHMM"
     */
    static std::string format_z(std::string fmt, long minuteOffset) {
        // extract minutes and hours of timezone offset
        bool isPositive = minuteOffset >= 0;
        if (!isPositive) minuteOffset *= -1;

        long tMinutes = minuteOffset % 60;
        long tHours = minuteOffset / 60;

        // build +-HHMM offset
        std::stringstream bruce;
        bruce << (isPositive ? '+' : '-')
              << std::setfill('0') << std::setw(2) << tHours
              << std::setfill('0') << std::setw(2) << tMinutes;
        std::string tzOffset = bruce.str();

        // while %z exists in fmt, replace it with tz_offset
        size_t z_k;
        while ((z_k = scan_x(fmt, 'z')) != std::string::npos)
            fmt = replace_at(fmt, z_k, tzOffset);

        return fmt;
    }

    /**
     * Replaces one instance of "%x" at <at> with <with> in <str>
     */
    static std::string replace_at(const std::string &str, size_t at, const std::string &with) {
        // copy first part of string
        std::string result = str.substr(0, at);
        // replace %k with <with>
        result += with;
        // append part after %k
        return result + str.substr(at + 2);
    }

    /**
     * Searches the string for "%x", but ignores "%%x"
     *
     * @param fmt The format string to search
     * @return Index of first %x match or npos
     */
    static size_t scan_x(const std::string &fmt, char x) {
        size_t count_percent = 0;

        for (size_t i = 0; i < fmt.size(); i++) {
            // if we found %x with uneven number of %, return index of the preceding %
            if (fmt[i] == x && count_percent % 2 != 0)
                return i - 1;
            else if (fmt[i] == '%')
                count_percent++;
            else
                count_percent = 0;
        }

        return std::string::npos;
    }

    void set(int64_t timestamp, bool utc) {
        using namespace std::chrono;

        // save millis
        mMillis = static_cast<int>(timestamp % 1000);

        // convert millis to a system_clock timepoint, store in s_time
        milliseconds t_millis(timestamp);
        auto s_duration = duration_cast<system_clock::duration>(t_millis);
        auto s_time = system_clock::to_time_t(system_clock::time_point(s_duration));

        // convert s_time to s_tm
        if (utc)
            gmtime_safe(&s_time, &mTime);
        else
            localtime_safe(&s_time, &mTime);
    }

    // date and time with seconds precision
    tm mTime {};
    // additional millis
    int mMillis = 0;
    // local or UTC time?
    bool mIsUTC = true;
};

#endif //COMMONS_TIME_H
