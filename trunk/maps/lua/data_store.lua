-------------------------------------------------------------------------------
-- data_store.lua | Utility
--
-- The data store is for manipulating and optionally permanently storing to
-- disk (on by default) arbitrary data, either globally or about a specific
-- player.
-------------------------------------------------------------------------------
---------------------------------------
-- If DataStore already exists then check it is the correct addon type.
-- If it is not, this is an error. If it is, we (presumably) have already
-- required this chunk so no need to do it again.
---------------------------------------
if DataStore ~= nil then
    local a, b, c = type(DataStore)
    assert(a == "addon" and
        b == "utility" and
        c == "ds",
        "Global DataStore exists but is not an addon utility ds!")
    return
end

_data_store = { _global = {}, _players = {}, _special = {}}

function _data_store._object(objtype, id)
    container = { ['type'] = objtype, ['type_id'] = id }
    setmetatable(container, _data_store._special)
    return container
end

function _data_store._serialize(value)
    local tables = {}

    local function serialize(path, value, depth)
        local t = type(value)

        if t == 'string' then
            return string.format("%q", value)
        elseif t == 'function' then
            return 'loadstring('.. string.format("%q", string.dump(value)) ..')'
        elseif t == 'number' or t == 'boolean' then
            return tostring(value)
        elseif t == 'table' then
            if tables[value] ~= nil then
                -- We have seen this table referenced before
                table.insert(tables[value]["refs"], path)
                return "{}"
            elseif getmetatable(value) == _data_store._special then
                -- Probably a player or normal object
                if value.type == game.TYPE_PLAYER and value.type_id then
                    return '_data_store._object(game.TYPE_PLAYER, ' .. string.format("%q", value.type_id) .. ')'
                else
                    return 'nil' -- We don't try to save anything else
                end
            else
                local ret = ""
                local indent = depth .. "  "
                local key 

                tables[value] = {["path"] = path, ["refs"] = {}}
                for k,v in value do
                    if type(k)=="string" then
                        key = '[' .. string.format("%q", k) .. ']'
                    else
                        key = '[' .. tostring(k) .. ']'
                    end

                    ret = ret .. indent .. key .. ' = ' .. serialize(path .. key, v, indent) .. ",\n"
                end
                return "{\n" ..  ret ..  depth .. '}'
            end
        else
            return 'nil' -- Something we can't handle (yet)
        end
    end

    local ret = "local t = " .. serialize('t', value, '') .. '\n'

    -- write out all references at the end
    for _,info in pairs(tables) do
        for _,ref in info["refs"] do
            ret = ret .. ref .. " = " .. info["path"] .. "\n"
        end
    end
    return ret .. "return t\n"
end

-- Build a path to a player directory
function _data_store._player_path(player)
    -- Replace up to 2 (we only care about the first 2 characters) - with _
    local munge = string.gsub(player, "%-", "_", 2)

    return "data/players/" .. string.lower(string.sub(munge, 1, 1) .. "/" ..
           string.sub(munge, 1, 2)) .. "/" .. player
end

function _data_store._load(id, player)
    local path, t
    if player then
        path = _data_store._player_path(player)
        t = _data_store._players[player]
        if not t then
            t = {n = 0}
            _data_store._players[player] = t
        end
    else
        path = "data/global"
        t = _data_store._global
    end
    path = path .. "/" .. id .. ".dsl"

    if not t[id] then
        local f = loadfile(path)
        if f == nil then
            return false
        end
        local data = f()
        if data == nil then
            print("DataStore: corrupt datastore file: "..path)
            local SA = game:FindPlayer('_person_')
            if SA == nil then
                return nil
            else
                -- Do a hard read on the file. loadfile compiles a Lua script file
                -- so it returns nil if there's only half a file.
                local f = assert(io.open(path, "r"))
                local t = f:read("*all")
                f:close()
                SA:ChannelMsg('SA', t, 0)
            end
        end
        t[id] = {_changed = 0, _persist = true, _data = data}
        setmetatable(t[id]._data, _DataStore_mt)
        setmetatable(t[id], { __index = t[id]._data, __newindex = t[id]._data })
    end

    return t[id]
end

-- Returns true if there was no error, or false if any error occured
function _data_store._save(time, player, b_force)
    local everything_ok = true
    local t
    if player then
        t = _data_store._players[player]
    else
        t = _data_store._global
    end
    -- Nothing to save?
    if not t then return true end

    for k, v in pairs(t) do
        if k ~= 'n' then
            local b_save = false
            if not b_force then
                local changed = t._changed
                if changed ~= 0 and os.difftime(time, changed) >= 120 then
                    b_save = true
                end
            else
                b_save = true
            end

            -- don't save non-persistent datastores
            if rawget(v, "_persist") == false then
                b_save = false
            end

            if b_save then
                local dir
                if player then
                    dir = _data_store._player_path(player)

                    -- Don't save datastore if player file doesn't exist
                    -- (that is normally a player without exp)
                    if not io.exists(dir.."/"..player..".pl") then
                        print("DataStore: Not saving stored data for player "..player.." - "..dir.."/"..player..".pl file missing")
                        dir = nil
                    end
                else
                    dir = "data/global"
                end
                if dir then
                    local filename = dir .. "/" .. k .. ".dsl"
                    local f = io.open(filename, "wb")
                    if not f then
                        print("DataStore: Couldn't open " .. filename .. " for writing")
                        everything_ok = false
                    else                    
                        -- print("DataStore: saving stored data for player "..player.." in "..filename)
                        f:write(_data_store._serialize(v._data))
                        f:close()
                    end
                end
            end
        end
    end
    return everything_ok
end

-- Remove's the player's datastore from memory
function _data_store.forget_player(player)
    _data_store._players[player] = nil
end

-- Returns true if there was no error, or false if any error occured
function _data_store.save_player(player, b_force)
    -- Nothing to save?
    if _data_store._players[player] == nil then return true end

    return _data_store._save(os.time(), player, b_force)
end

-- Returns true if there was no error, or false if any error occured
function _data_store.save(b_force)
    local everything_ok = true
    local players, time = _data_store._players, os.time()
    _data_store._save(time, nil, b_force)
    for player in players do
        if player ~= 'n' then
            if not _data_store._save(time, player, b_force) then
                everything_ok = false
            end
        end
    end
    return everything_ok
end

---------------------------------------
-- Assign the global DataStore table. Give it a metatable. The __call
-- metamethod means that calling DataStore() returns a new instance of
-- the addon (as specified by the __metatable metamethod).
---------------------------------------
DataStore = {}
setmetatable(DataStore, {
    __call = function(self, id, player)
        assert(type(id) == "string", "Arg #1 must be string!")
        assert(type(player) == "string" or
               (type(player) == "GameObject" and player.type == game.TYPE_PLAYER) or
               player == nil,
               "Arg #2 must be string or player GameObject or nil!")
        local t
        if player == nil then
            t = _data_store._global
        else
            if type(player) == "GameObject" then
                player = player.name
            end
            player = string.capitalize(player)
            local players = _data_store._players
            t = players[player]
            if t == nil then
                players[player] = {n = 0}
                t = players[player]
            end
        end
        local obj = t[id]
        if obj == nil then
            if not _data_store._load(id, player) then
                obj = {_changed = 0, _persist = true, _data = {}}
                setmetatable(obj._data, _DataStore_mt)
                setmetatable(obj, { __index = obj._data, __newindex = obj._data })
                t[id] = obj
                t.n = table.getn(_data_store) + 1
            else
                obj = t[id]
            end
        end
        return obj
    end,
    __metatable = function() return "addon", "utility", "ds" end
})

_DataStore_mt = {
    __index = DataStore,
    __metatable = function() return "addon", "utility", "ds" end,
    __newindex = function() error("Use Set() to add/change values") end
}

-- Static function to dump a serialization string
-- (Only a wrapper around the private _data_store function)
function DataStore.Serialize(value)
    return _data_store._serialize(value)
end

function DataStore:Get(key)
    local value = rawget(self._data, key)
    local t = type(value)
    if t == "GameObject" and not game:IsValid(value) then
        value = nil
    elseif t == "table" and getmetatable(value) == _data_store._special then
        local object = value.object
        if game:IsValid(object) then
            value = object
        elseif value.type == game.TYPE_PLAYER and value.type_id then
            value = game:FindPlayer(value.type_id)
            if value then
                self:Set(key, value)
            end
        end
    end
    return value
end

function DataStore:Set(key, value)
    assert(type(key) == 'string', "datastore keys must be strings")
    if type(value) == "GameObject" and value.count then
        local ref = {id = value.count, object = value}
        local t = value.type
        if t == game.TYPE_PLAYER then
            ref.type = t
            ref.type_id = value.name
        end
        setmetatable(ref, _data_store._special)
        value = ref
    end
    rawset(self._data, key, value)
    rawset(self, "_changed", os.time())
end

function DataStore:WasChanged()
    rawset(self, "_changed", os.time())
end

function DataStore:SetPersistence(persist)
    rawset(self, "_persist", persist)
end

function DataStore:GetPersistence()
    return rawget(self, "_persist")
end
