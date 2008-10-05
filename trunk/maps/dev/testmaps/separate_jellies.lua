-------------------------------------------------------------------------------
-- separate_jellies.lua -- apply on spawnpoint at testmap_plugin x y
--
-- Demonstrates APPLY scripts on spawnpoints.
-------------------------------------------------------------------------------
local spawnpoint = event.me
local jelly = event.activator
local loot = event.other

---------------------------------------
-- Interrupt the normal spawn process. This means that the mob will  cease to
-- exist when the script returns unless we put him in a map (which we do). Also
-- he will not be registered with the spawnpoint, which means (a) he has no
-- home (he's a wandering mob) and (b) the spawnpoint iis still free to attempt
-- another spawn -- which means theoretically we can generate infinite mobs
-- from a single spawnpoint and have them all in play at the same time (however
-- be cautious that this may not be a good thing and this behaviour may change).
---------------------------------------
event.returnvalue = 1

---------------------------------------
-- This section demonstrates that we have access to the mob's entire inventory.
-- We can view it and modify/delete it. The inventory is separated into two
-- parts, loot and treasure. Loot is the particular items given to an indivdual
-- mob at map creation time, and is held in a special temporary 'container'
-- object which is passed to the script in event.other. Treasure is any
-- randomitem(s) generated at spawntime, and is held in mob.inventory as normal.
-- 
-- This script lists all this ingame so you can see it, and also extracts any
-- money from the mob's treasure and leaves it on top of the spawnpoint.
---------------------------------------
local coins = {}

    spawnpoint.map:Message(spawnpoint.x, spawnpoint.y, 12, tostring(loot), game.COLOR_RED)
for obj in obj_inventory(loot) do
    spawnpoint.map:Message(spawnpoint.x, spawnpoint.y, 12, "LOOT: " .. tostring(obj))
end

for obj in obj_inventory(jelly) do
    spawnpoint.map:Message(spawnpoint.x, spawnpoint.y, 12, "TREASURE: " .. tostring(obj))
    if obj.type == game.TYPE_MONEY then
        table.insert(coins, obj)
    end
end

for i, v in ipairs(coins) do
    v:SetPosition(spawnpoint.map, spawnpoint.x, spawnpoint.y)
end

---------------------------------------
-- Here we put the mob in a pen, depending on what type it is.
---------------------------------------
local beacon = game:LocateBeacon("testmap_plugin " .. string.lower(jelly.name) .. " pen")
assert(beacon, "Could not find beacon 'testmap_plugin " .. string.lower(jelly.name) .. " pen'!")

jelly:SetPosition(beacon.map, beacon.x, beacon.y)
