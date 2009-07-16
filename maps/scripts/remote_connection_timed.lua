-------------------------------------------------------------------------------
-- remote_connection_timed.lua
--
-- This script is basically remote_connection.lua copied over and extended to make timed connections possible.
-- I would have just used require("/scripts/remote_connection.lua"), but this script requires an extra parameter for the event.options string, which
-- makes it a bit incompatible.
--
-- As a result, this script functions similar to remote_connection.lua, but with a couple of differences. The main difference is that after so long, the
-- script resets the connection, like with a tigger_lever. It is also possible to specify how long it takes for the connection to be reset.
--
-- If anyone wonders why I don't have just the one handle that applies itself after a coroutine.yield, to put it simply, it is not possible to script it
-- this way. It *must* be an apply script in order to work properly, so when it applies itself, it will trigger more scripts recursively. It is not
-- possible to determine when or when it should not do this. So we are stuck with requiring a second handle to reset the connection.
--
-- In order to use this script, the script needs to be inserted into a handle object. Then a hidden handle object should be placed with a beacon in its
-- inventory and it should generally be out of sight to players. If you are doing this on a remote map, make sure to insert a remote_connection.lua
-- script into the hidden handle, and a beacon into the handle containing this script, so that the connection reset will work.
--
-- There are a few parameters that can be specified in the event options string. Each parameter must be separated by a '|' character.
-- Eg, "/dev/testmaps/testmap_main|testmap_beacon1|5"
--
-- Parameter types:
-- map_path: string (required)         - The absolute path to the map with the hidden handle.
-- beacon_name: string (required)      - The name of the beacon which is in the inventory of the hidden handle. Make sure to give this a unique name.
-- handle_duration: integer (required) - The duration in seconds until the connection is reset.
-------------------------------------------------------------------------------

local me = event.me
local ac = event.activator

-- Parse the event option string
local options = { string.find(event.options, "([^|]+)%s*|%s*([^|]+)%s*|%s*(%d+)") }; assert(options[1], "Insufficient options passed to script!")

local map_path        = options[3]
local beacon_name     = options[4]
local handle_duration = options[5]

-- Check if the handle is triggered or not
if me.weight_limit == 0 then
    -- Ready the second map. This makes sure it is in memory, loading it if necessary.
    game:ReadyMap(map_path)

    -- Locate the beacon and by extension the second switch.
    local beacon = game:LocateBeacon(beacon_name); assert(beacon, "Could not find beacon!")
    local handle = beacon.environment;             assert(handle, "Could not find trigger!")

    -- Delay the echoed trigger by the duration determined by the mapper
    coroutine.yield(handle_duration)

    -- Check if the object reference for the trigger still exists
    if game:IsValid(handle) then
        -- Activate the trigger.
        if handle:Apply(handle, game.APPLY_TOGGLE) == 0 then error("Could not activate trigger!") end
    end
else
    -- Don't let the player trigger the handle again until the connection is reset
    ac:Write("The "..me.name.." doesn't move.", game.COLOR_WHITE)
    event.returnvalue = 1
end

