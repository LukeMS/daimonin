-------------------------------------------------------------------------------
-- This script loads and executes when the plugin is loaded, it is meant to
-- load libraries, extend system libraries and other fancy things.
--
-- Please use local variables for anything done in here...
--
-- Also note that this script is _only_ loaded at server startup.
-------------------------------------------------------------------------------
-------------------
-- Extend type() to recognize addon tables.
--
-- These are tables which encapsulate addon data and provide methods for the
-- manipulation of that data. Addon tables have a metatable with a
-- __metatable metamethod which is a function returning 3 strings: "addon";
-- "<secondary>"; "<tertiary>". -- type(v) returns these three strings or just
-- one string as normal for other types.
-- 
-- So the three strings act as a 'fingerprint' for an addon: "addon" identifies
-- this as a special addon table and guarantees that there are two more
-- strings;  "<secondary>" identifies the subtype addon and may be "module" or
-- "utility"; "<tertiary>" identifies the subsubtype addon and depends on what
-- <secondary> was -- by convention utilities have a two-letter identifier and
-- modules have a three or more-letter identifier. All strings are lower case
-- letters only.
-------------------
function wrap_type()
    local orig_type = type
    return function(v)
        local t = orig_type(v)
        if t ~= "table" then
            return t
        end
        local mt = getmetatable(v)
        if type(mt) ~= "function" then
            return t
        end
        local a, b, c = mt()
        if a ~= "addon" or
            type(b) ~= "string" or
            type(c) ~= "string" then
            return t
        end
        return a, b, c
    end
end
type = wrap_type()

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

require("plugin/security")
require("plugin/emulate")
require("data_store")

--
-- Tables of gender-specific nouns and pronouns.
-- Use eg, "blah " .. gender_possessive[obj:GetGender()] .. " item." for "blah his item."
-- Note that the possessive is in the sense of, eg, "that is her item" rather than "that item is hers". I think this form will prove the more useful.
--

gender_noun       = { [game.NEUTER] = "neuter", [game.MALE] = "male", [game.FEMALE] = "female", [game.HERMAPHRODITE] = "hermaphrodite" }
gender_objective  = { [game.NEUTER] = "it",     [game.MALE] = "he",   [game.FEMALE] = "she",    [game.HERMAPHRODITE] = "they" }
gender_subjective = { [game.NEUTER] = "it",     [game.MALE] = "him",  [game.FEMALE] = "her",    [game.HERMAPHRODITE] = "them" }
gender_possessive = { [game.NEUTER] = "its",    [game.MALE] = "his",  [game.FEMALE] = "her",    [game.HERMAPHRODITE] = "their" }


--
-- A couple of extensions to the string library
--

function string.capitalize(s, b_keep)
    local t
    if type(s) == "table" then
        t = s
    else
        t = { [1] = s }
    end
    for i, v in ipairs(t) do
        if type(v) == "string" then
            local leading, first, rest = string.match(v, "^(%s*)(%a?)(.*)")
            if first and first ~= "" then
                first = string.upper(first)
            end
            if rest and rest ~= "" and not b_keep then
                rest = string.lower(rest)
            end
            t[i] = leading .. first .. rest
        end
    end
    return table.concat(t, " ")
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

---------
-- string.match is a useful addition the the Lua 5.1 library so emulate it.
---------
if not string.match then
    function string.match(s, pattern, init)
        local t = { string.find(s, pattern, init) }
        if t[1] ~= nil then
            if t[3] == nil then
                return s
            else
                table.remove(t, 1)
                table.remove(t, 1)
                return unpack(t)
            end
        end
        return nil
    end
end

--
-- Very useful and elegant iterator functions for for loops
-- for obj in obj_inventory(object) loops through the inventory of object
-- for obj in map_objects(map, x, y) loops through all the objects on square
-- for obj in multipart(object) loops through all the parts of object starting at head
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

function multipart(obj)
    local iterator = function(_, last) return last.more end

    if obj.head then
        return iterator, nil, { more = obj.head }
    else
        return iterator, nil, { more = obj }
    end
end

-------------------
-- absolute_path() returns the absolute path of suffix based on prefix.
-- Becuase the map readying methods only work on absolute paths, but these are
-- shunned in all other circumstances (ie, exit paths).
-------------------
function absolute_path(prefix, suffix)
    ---------
    -- If suffix is already absolute, nothing to do. Return it unaltered.
    ---------
    if string.sub(suffix, 1, 1) == "/" then
        return suffix
    end

    local len = string.len(prefix)

    ---------
    -- Find the last / in prefix. This separates the filename from the dirs.
    ---------
    for i = 1, len do
        if string.sub(prefix, i, i) == "/" then
            len = i
        end
    end

    ---------
    -- Truncate prefix.
    ---------
    prefix = string.sub(prefix, 1, len)

    ---------
    -- Concatenate prefix and suffix to get the absolute path. Return this.
    ---------
    return prefix .. suffix
end

---------------------------------------
-- Magic errorhandler, sends script errors to MWs/MMs/SAs (either globally via
-- the MW channel or individually to any such gmasters directly involved in the
-- script's running, depending on the availabilitt of the channel or method).
---------------------------------------
function _error(prefix)
    -------------------
    -- Prefix. The error prefix tends to contain '`' chars which are
    -- interpreted by Dai as 'toggle underline'. We don't want this so change
    -- them to '''.
    -------------------
    prefix = string.gsub(prefix, "`", "'")
    local message = prefix .. "\n"

    -------------------
    -- Affix all the event data. Also this is a convenient time to ascertain
    -- our reporter. It does not matter who or what it is, any GameObject will
    -- do.
    -------------------
    local ev = {
        [1] = { k = "me" },
        [2] = { k = "activator" },
        [3] = { k = "other" },
        [4] = { k = "message" },
        [5] = { k = "options" } } 
    local reporter = nil
    for _, v in ipairs(ev) do 
        local val = event[v.k]
        local typ = type(val)
        if typ == "GameObject" or typ == "Map" then
            if game:IsValid(val) == false then
                 val = nil
                 typ = "nil"
            elseif typ == "GameObject" then
                reporter = val
            end
        end
        v.v = val
        v.t = typ
        message = message .. v.k .. "="
        if v.t == "string" then
            message = message .. "~\"" .. tostring(v.v) .. "\"~\n"
        elseif v.v ~= nil then
            message = message .. "~" .. tostring(v.v) .. "~\n"
        else
            message = message .. "nil\n"
        end
    end

    -------------------
    -- Finally suffix with a traceback.
    -------------------
    message = message .. debug.traceback()

    -------------------
    -- Where reporter is nil or messaging the MW channel fails, just return the
    -- entire message. The server logs this in the tech logs.
    -------------------
    if reporter == nil or not reporter:ChannelMsg("MW", message, game.CHANNEL_MODE_SYSTEM) then
        return message
    -------------------
    -- Otherwise return the prefix (which is the basic error) and a quick note
    -- as the detail is already logged in the chat logs. The server logs this
    -- in the tech logs.
    -------------------
    else
        return prefix .. "\n(See chat logs for details.)"
    end
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

-------------------
-- If ../maps/lua/locality/init.lua exists, require it. This file is only
-- available with the official maps. It sets up the Locality structure (for
-- rumours, etc). If it does not exist, provide some stubs so everything still
-- works.
-------------------
--if io.exists("../maps/lua/locality/init.lua") then
--    require("locality/init")
--else
    Locality = {}
    function Locality.SpreadRumour()
        return "|[THIS SPACE INTENTIONALLY LEFT BLANK]|"
    end
--end
