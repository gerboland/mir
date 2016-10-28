/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Daniel van Vugt <daniel.van.vugt@canonical.com>
 */

#ifndef MIR_TIME_POSIX_TIMESTAMP_H_
#define MIR_TIME_POSIX_TIMESTAMP_H_

#include <chrono>
#include <ctime>

namespace mir { namespace time {

/*
 * We need absolute precision here so sadly can't use high-level C++ clocks...
 *  - Graphics frame timing needs support for at least the kernel clocks
 *    CLOCK_REALTIME and CLOCK_MONOTONIC, to be selected at runtime, whereas
 *    std::chrono does not support CLOCK_REALTIME or easily switching clocks.
 *  - mir::time::Timestamp is relative to the (wrong) epoch of steady_clock,
 *    so converting to/from mir::time::Timestamp would be dangerously
 *    inaccurate at best.
 */

struct PosixTimestamp
{
    clockid_t clock_id;
    std::chrono::nanoseconds nanoseconds;

    PosixTimestamp()
        : clock_id{CLOCK_MONOTONIC}, nanoseconds{0} {}
    PosixTimestamp(clockid_t clk, std::chrono::nanoseconds ns)
        : clock_id{clk}, nanoseconds{ns} {}
    PosixTimestamp(clockid_t clk, struct timespec const& ts)
        : clock_id{clk}, nanoseconds{ts.tv_sec*1000000000LL + ts.tv_nsec} {}

    static PosixTimestamp now(clockid_t clock_id)
    {
        struct timespec ts;
        clock_gettime(clock_id, &ts);
        return PosixTimestamp(clock_id, ts);
    }
};

}} // namespace mir::time

#endif // MIR_TIME_POSIX_TIMESTAMP_H_