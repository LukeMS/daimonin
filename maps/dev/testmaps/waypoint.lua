-- Just a basic test of waypoint triggers

monster  = event.activator -- The monster
waypoint = event.me        -- The waypoint

-- Do something
monster:Say('I just reached the waypoint "' .. waypoint.name .. '"')
