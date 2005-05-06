_data_store = { _global = {}, _players = {}}

function _data_store._object(objtype, id)
    return { ['type'] = objtype, ['type_id'] = id }
end

function _data_store._serialize(key, value, depth)
    local prefix
    if depth == nil then 
        depth = '' 
        prefix = 'return '
    else
        if type(key)=="string" then
            prefix = depth .. '[' .. string.format("%q", key) .. '] = '
        else
            prefix = depth .. '[' .. tostring(key) .. '] = '
        end
    end

    local t = type(value)
    if t == 'string' then
        return prefix .. string.format("%q", value) 
    elseif t == 'function' then
        return prefix .. 'loadstring('.. string.format("%q", string.dump(value)) ..')'
    elseif t == 'number' or t == 'boolean' then
        return prefix .. tostring(value)
    elseif t == 'table' then
        if value.type == game.TYPE_PLAYER and value.type_id then
			return prefix .. '_data_store._object(game.TYPE_PLAYER, ' .. string.format("%q", value.type_id) .. ')'
        else            
            local ret = ""
            for k,v in value do
                ret = ret .. _data_store._serialize(k, v, depth .. '  ') .. ",\n"
            end
            return prefix .. "{\n" .. 
                    ret .. 
                   depth .. '}'
        end
    end
end

function _data_store._load(id, player)
	local path, t
	if player then
		path = "players/" .. player
		t = _data_store._players[player]
		if not t then
			_data_store[player] = {n = 0}
			t = _data_store[player]
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
					dir = "players/" .. player
				else
					dir = "global"
				end
				local filename = "data/" .. dir .. "/" .. k .. ".dsl"
				local f = io.open(filename, "wb")
				assert(f, "Couldn't open " .. filename)
                f:write(_data_store._serialize(k, v))
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

function DataStore:Get(key)
	local value = rawget(self, key)
	local t = type(value)
	if t == "userdata" and not game:IsValid(value) then
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
	if type(value) == "userdata" and value.count then
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
		if type(player) == "userdata" then
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
