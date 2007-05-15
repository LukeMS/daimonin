--[[
-- This script loads and executes when the plugin is loaded,
-- it is meant to load libraries, extend system libraries and
-- other fancy things.
--
-- Please use local variables for anything done in here...
--
-- Also note that this script is _only_ loaded at server startup
--]]

if game == nil then require "plugin_emulate" end

-- Functions that need to be declared before we load the security system:
local orig_io_open = io.open
io.exists = function(filename)
    file = orig_io_open(filename, "r")
    if file ~= nil then
        io.close(file)
        return true
    end
    return false
end

require("security")
require("data_store")

--
-- A couple of extensions to the string library
--

function string.capitalize(s, b_keep)
    local t = {}
    if type(s) == "table" then
        t = s
    else
        table.insert(t, s)
    end
    local words = table.getn(t)
    for i= 1, words do
        if type(t[i]) == "string" and t[i] ~= "" then
            local s1, s2 = string.sub(t[i],1,1), string.sub(t[i],2)
            if s2 then
                if not b_keep then
                    s2 = string.lower(s2)
                end
                t[i] = string.upper(s1) .. s2
            else
                t[i] = string.upper(s1)
            end
        end
    end
    return table.concat(t," ")
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

--
-- Very useful and elegant iterator functions for for loops
-- for object in obj_inventory(obj)
-- for object in map_objects(map, x, y)
--

-- TODO: In the future I want to add those to the Map and Object classes,
-- but this will do for now. I hope people will use them

function obj_inventory(obj)
    local iterator = function(_, last) return last.below end
    return iterator, nil, { below=obj.inventory }
end

function map_objects(map, x, y)
    local iterator = function(_, last) return last.above end
    return iterator, nil, { above = map:GetFirstObjectOnSquare(x,y) }
end

-- Magic errorhandler, sends script errors to involved DM:s
-- TODO: possibility to turn on/off either via a script or custom commands
-- TODO: possibility to register DM's that should get messages even if not involved
function _error(msg)
    msg = debug.traceback(msg)

    local function msg_wiz_obj(obj)
        if obj and game:IsValid(obj) and obj.f_wiz then
            obj:Write("LUA: "..tostring(msg))
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

-- Redirect print() to game:Log()
function print(...)
    local text = ""
    if arg.n >= 1 then
        text = tostring(arg[1])
        for i=2,arg.n do
            text = text .. "\t" .. tostring(arg[i])
        end
    end
    game:Log(game.LOG_INFO, text)
end

-- Finished with the initialization
print "    plugin_init.lua loaded successfully"
