-------------------------------------------------------------------------------
-- insert_marker.lua
--
-- This script makes it possible to insert a force inside a player like a marker does, but without the catch of requiring a player to walk over it.
--
-- It has an option to set a slaying field which will be identical to the name of the force if the option is set. The slaying field is used just like
-- they are with keys, which means the marker force can act as a temporary key to get through locked doors and the like. This means that care must be
-- taken to ensure the slaying field is unique as with keys. It also has an option to replace an existing marker force in the player's inventory with an
-- identical name, unlike a regular marker object where you have to wait for the marker to run out before you can insert a new one. Finally, there is an
-- option to determine if you want to stop the marker force from counting down when it is inserted into the player's inventory.
--
-- The marker force can be inserted when a player applies a lever for example, or even when a player kills a mob. The script just needs to be inserted in
-- the inventory of the object you want the player to apply, kill, etc.
--
-- There are a few parameters that can be specified in the event options string. Each parameter must be separated by a '|' character.
-- Eg, "test_key_temp|125|1|You suddenly gain a temporary ability to unlock the door next to you!|0|0"
--
-- Parameter types:
-- marker_name: string (required)      - The name of the marker force. Used for identifying the force later. Make sure to make this name unique.
-- marker_duration: integer (required) - The food attribute of the marker. This determines how long the marker will last until it disappears. A value of
--                                       0 will mean it will last forever. A food value of 1 will last about 12.5 seconds.
-- marker_slaying: integer (optional)  - A value of 0 will create a force without a slaying field. A value of 1 will add the slaying field identical to
--                                       the name of the force. The slaying field is used like keys to open doors, pass through check_invs, etc. Default
--                                       behaviour is 0.
-- marker_message: string (optional)   - If specified, will print out the string to the player if the marker is successfully inserted into the inventory
--                                       of the player.
-- marker_replace: integer (optional)  - A value of 0 will make the script only insert the marker force if no others exist with an identical slaying
--                                       field in the player's inventory. This is identical to the behaviour of regular marker objects. A value of 1
--                                       will always replace the marker force if another exists. Default behaviour is 0.
-- marker_stopped: integer (optional)  - A value of 0 will start the marking force counting down as soon as it's inserted in the player's inventory. A
--                                       value of 1 will insert the marker in a frozen state. The marking force can potentially exist in the player's
--                                       inventory permanently if this is set so be careful. Default behaviour is 0.
-------------------------------------------------------------------------------

local me = event.me
local ac = event.activator

-- Parse the event option string. The marker_slaying, marker_message, marker_replace and marker_stopped parameters are optional.
local options = { string.find(event.options, "([^|]+)%s*|%s*(%d+)%s*|?%s*(%d*)%s*|?%s*([^|]*)%s*|?%s*(%d*)%s*|?%s*(%d*)") }; assert(options[1], "Insufficient options passed to script!")

local marker_name     = options[3]
local marker_duration = options[4]
local marker_slaying  = options[5]
local marker_message  = options[6]
local marker_replace  = options[7]
local marker_stopped  = options[8]

-- Search the player's inventory for an existing marker with the same name
local marker_cmp = ac:CheckInventory(0, "player_force", marker_name)

-- If override mode is specified, check to see if we have an existing marker first
if marker_replace == "1" and marker_cmp then
    -- We want to replace the marker, remove it so we can add a new one below
    marker_cmp:Remove()
    marker_cmp = nil
end
-- If there are no markers, we can now safely insert one in the player's inventory
if marker_cmp == nil then
    if marker_stopped == "0" then
        -- Create the marker force in the player's inventory. Add 1 food to the duration because a force created with food 1 instantly disappears.
        local marker = ac:CreatePlayerForce(marker_name, marker_duration + 1)
    else
        -- Stopped option is set, so create an infinite force
        local marker = ac:CreatePlayerForce(marker_name, 0)
    end
    if marker then
        -- Set the speed to 0.01 (same value as regular markers)
        marker.speed = 0.01
        -- Add the slaying field to the marker if option is set
        if marker_slaying == "1" then
            marker.slaying = marker_name
        end
        -- Give the player the message if one exists
        if marker_message ~= "" then
            ac:Write(marker_message)
        end
    end
end

