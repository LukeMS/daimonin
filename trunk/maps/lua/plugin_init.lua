--[[
-- This script loads and executes when the plugin is loaded,
-- it is meant to load libraries, extend system libraries and
-- other fancy things.
--
-- Please use local variables for anything done in here...
--
-- Also note that this script is _only_ loaded at server startup
]]

require("data_store")

-- string.split() function
function string.split(s, sep)
	if s == nil then return nil end
	if sep == nil or sep == "" then
		sep = "%s"
	end
	local t={}, n
	for n in string.gfind(s, "[^" .. sep .. "]+") do
		table.insert(t, n)
	end
	return t
end

-- Magic errorhandler, sends script errors to involved DM:s
-- TODO: possibility to turn on/off either via a script or custom commands
-- TODO: possibility to register DM's that should get messages even if not involved
function _error(msg)
    local function msg_wiz_obj(obj)
        if obj and game.IsValid(obj) and obj.f_wiz then
            obj.Write(obj, "LUA: "..tostring(msg))
            return true
        end
        return false
    end

    if event then
        msg_wiz_obj(event.activator)
        msg_wiz_obj(event.me)
        msg_wiz_obj(event.other)
    end
	
    return msg
end
