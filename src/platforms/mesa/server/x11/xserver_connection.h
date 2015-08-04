/*
 * Copyright © 2015 Canonical Ltd.
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
 * Authored by: Cemil Azizoglu <cemil.azizoglu@canonical.com>
 */

#ifndef MIR_X_XSERVER_CONNECTION_H_
#define MIR_X_XSERVER_CONNECTION_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace mir
{
namespace X
{

class X11Connection
{
public:
    X11Connection()
    {
        dpy = XOpenDisplay(nullptr);
    }

    ~X11Connection()
    {
        XCloseDisplay(dpy);
    }

    operator ::Display*() const
    {
        return dpy;
    }

private:
    ::Display *dpy;
};

}
}
#endif /* MIR_X_XSERVER_CONNECTION_H_ */
