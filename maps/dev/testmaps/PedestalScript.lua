local pl = event.activator -- the player activating the pedestal
local me = event.me -- the pedestal
local opt = event.options -- the options string

local tracing = true
local script = 'gr_door_switch.lua'

-- trace and bug functions
function LOG(text)
    game:Log(game.LOG_INFO, text)
end

function TRACE(text)
    if tracing then
        LOG ('TRACE ['..os.date()..'] ('..script..'): '..text)
    end
end

function NAME(text, obj)
    local safename
    if obj == nil then
        safename = '(nil)'
    else
        safename = obj.name
    end
    TRACE(text..' = '..safename)
end

function BUG(text)
    LOG ('BUG ('.. script..'): '..text)
end

-- Main
NAME('activator', event.activator)
NAME('me', event.me)
NAME ('other', event.other)
TRACE('me.state = '..me.state..'; me.weight_limit = '..me.weight_limit..'; face = '..me:GetFace() .. "; ac = "..me.armour_class)