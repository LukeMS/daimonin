_data_store = { _global = {}, _players = {}}

function _data_store._object(objtype, id)
    return { ['type'] = objtype, ['type_id'] = id }
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
            if value.type == game.TYPE_PLAYER and value.type_id then
                return '_data_store._object(game.TYPE_PLAYER, ' .. string.format("%q", value.type_id) .. ')'
            else
                if tables[value] ~= nil then
                    -- We have seen this table referenced before
                    table.insert(tables[value]["refs"], path)
                    return "{}"
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
            end
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
    return "players/" .. string.lower(string.sub(player,1,1) .. "/" .. string.sub(player, 1, 2)) .. "/" .. player
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
        path = "global"
        t = _data_store._global
    end
    path = "data/" .. path .. "/" .. id .. ".dsl"

    if not t[id] then
        local f = loadfile(path)
        if f == nil then
            return false
        end
        t[id] = f()
        assert(t[id], "Empty datastore file: "..path)
        setmetatable(t[id], _DataStore_mt)
    end

    return t[id]
end

function _data_store._save(time, player, b_force)
    local t
    if player then
        t = _data_store._players[player]
    else
        t = _data_store._global
    end
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
            if b_save then
                local dir
                if player then
                    dir = _data_store._player_path(player)
                else
                    dir = "global"
                end
                local filename = "data/" .. dir .. "/" .. k .. ".dsl"
                local f = io.open(filename, "wb")
                assert(f, "Couldn't open " .. filename)
                f:write(_data_store._serialize(v))
                f:close()
            end
        end
    end
end

function _data_store.save(b_force)
    local players, time = _data_store._players, os.time()
    _data_store._save(time)
    for player in players do
        if player ~= 'n' then
            _data_store._save(time, player, b_force)
        end
    end
end

DataStore = {}

-- Static function to dump a serialization string
-- (Only a wrapper around the private _data_store function)
function DataStore.Serialize(value)
    return _data_store._serialize(value)
end

function DataStore:Get(key)
    local value = rawget(self, key)
    local t = type(value)
    if t == "GameObject" and not game:IsValid(value) then
        value = nil
    elseif t == "table" and value.type == game.TYPE_PLAYER and value.type_id then
        local object = value.object
        if game:IsValid(object) then
            value = object
        else
            value = game:FindPlayer(value.type_id)
            if value then
                self:Set(key, value)
            end
        end
    end
    return value
end

function DataStore:Set(key, value)
    assert(key ~= "_changed", "You can't change '_changed'")
    assert(type(key) == 'string', "datastore keys must be strings")
    if type(value) == "GameObject" and value.count then
        local ref = {id = value.count, object = value}
        local t = value.type
        if t == game.TYPE_PLAYER then
            ref.type = t
            ref.type_id = value.name
        end
        value = ref
    end
    rawset(self, key, value)
    rawset(self, "_changed", os.time())
end

function DataStore:WasChanged()
    rawset(self, "_changed", os.time())
end

_DataStore_mt = {__index = DataStore, __newindex = function() error("Use Set() to add/change values") end}

function DataStore:New(id, player)
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
            obj = {_changed = 0}
            setmetatable(obj, _DataStore_mt)
            t[id] = obj
            t.n = table.getn(_data_store) + 1
        else
            obj = t[id]
        end
    end
    return obj
end

setmetatable(DataStore, {__call = DataStore.New})
