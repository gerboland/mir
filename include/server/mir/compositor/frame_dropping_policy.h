/*
 * Copyright © 2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 */

#ifndef MIR_COMPOSITOR_FRAME_DROPPING_POLICY_H
#define MIR_COMPOSITOR_FRAME_DROPPING_POLICY_H

#include <functional>

namespace mir
{
namespace compositor
{
/**
 * \brief Policy to determine when to drop a frame from a client
 */
class FrameDroppingPolicy
{
public:
    virtual ~FrameDroppingPolicy() = default;

    /**
     * \brief Notify that a swap has blocked
     */
    virtual void swap_now_blocking() = 0;
    /**
     * \brief Notify that previous swap is no longer blocking
     */
    virtual void swap_unblocked() = 0;
};

}
}

#endif // MIR_COMPOSITOR_FRAME_DROPPING_POLICY_H
