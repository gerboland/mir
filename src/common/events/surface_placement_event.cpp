/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "mir/events/surface_placement_event.h"

MirSurfacePlacementEvent::MirSurfacePlacementEvent()
{
    event.initSurfacePlacement();
}

int MirSurfacePlacementEvent::id() const
{
    return event.asReader().getSurfacePlacement().getId();
}

void MirSurfacePlacementEvent::set_id(int const id)
{
    event.getSurfacePlacement().setId(id);
}

MirRectangle MirSurfacePlacementEvent::placement() const
{
    auto rect = event.asReader().getSurfacePlacement().getRectangle();
    return {rect.getLeft(), rect.getTop(), rect.getWidth(), rect.getHeight()};
}

void MirSurfacePlacementEvent::set_placement(MirRectangle const& placement)
{
    auto rect = event.getSurfacePlacement().getRectangle();
    rect.setLeft(placement.left);
    rect.setTop(placement.top);
    rect.setWidth(placement.width);
    rect.setHeight(placement.height);
}

MirSurfacePlacementEvent const* MirEvent::to_surface_placement() const
{
    return static_cast<MirSurfacePlacementEvent const*>(this);
}
