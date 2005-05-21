--[[
-- This script loads and executes when the plugin is loaded,
-- it is meant to load libraries, extend system libraries and
-- other fancy things.
--
-- Please use local variables for anything done in here...
--
-- Also note that this script is _only_ loaded at server startup
]]

require("security")
require("data_store")

--
-- A couple of extensions to the string library
--

function string.capitalize(s, b_keep)
	local f = string.sub(s, 1, 1)
	if string.len(s) > 1 then
	s = string.sub(s, 2)
	if not b_keep then
		s = string.lower(s)
	end
	else
		s = ""
	end
	return string.upper(f) .. s
end

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
    msg = debug.traceback(msg)

    local function msg_wiz_obj(obj)
        if obj and game:IsValid(obj) and obj.f_wiz then
            obj.Write(obj, "LUA: "..tostring(msg))
            return true
        end
        return false
    end

    if event and game:IsValid(event) then
        msg_wiz_obj(event.activator)
        msg_wiz_obj(event.me)
        msg_wiz_obj(event.other)
    end

    return msg
end

-- Shutdown hook called when server unloads the plugin
function _shutdown()
	_data_store.save(true)
end

-- Finished with the initialization
print "    plugin_init.lua loaded successfully"
