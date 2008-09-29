local spawnpoint = event.me
local jelly = event.activator

event.returnvalue = 1

local beacon = game:LocateBeacon("testmap_plugin " .. string.lower(jelly.name) .. " pen")
assert(beacon, "Could not find beacon 'testmap_plugin " .. string.lower(jelly.name) .. " pen'!")

local x, y = beacon.x, beacon.y

jelly:SetPosition(x, y)
