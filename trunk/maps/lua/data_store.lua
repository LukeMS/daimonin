_data_store = {_ignore = {"_ignore", "_ignore_player", "_load", "_players", "_save", "_serialize", "_unserialize", "n", "save"}, _ignore_player = {"n"}, _players = {}, n = 0}

function _data_store._serialize(v, ignore)
	local t = type(v)
	if t == "number" then
		return "n" .. v .. "\0"
	elseif t == "string" then
		return "s" .. v .. "\0"
	elseif t == "table" then
		if v.type == game.TYPE_PLAYER and v.type_id then
			return "p" .. v.type_id .. "\0"
		else
			local b_proceed, ret = true, "t"
			for k, val in pairs(v) do
				if ignore then
					for k2, val2 in pairs(ignore) do
						if val2 == k then
							b_proceed = false
							break
						end
					end
				end
				if b_proceed then
					ret = ret .. _data_store._serialize(k) .. _data_store._serialize(val)
				end
			end
			return ret .. "\0"
		end
	end
end

function _data_store._unserialize(s)
	local data, t = "", string.sub(s, 1, 1)
	if t == "s" or t == "p" or  t == "n" then
		local a, b
		a, b, data = string.find(s, "([^%z]*)", 2)
	elseif t ~= "t" then
		error("Unknown type '" .. t .. "'")
	end
	if t == "n" then
		return tonumber(data)
	elseif t == "s" then
		return tostring(data)
	elseif t == "p" then
		return {object = nil, ["type"] = game.TYPE_PLAYER, type_id = tostring(data)}
	elseif t == "t" then
		local b_new, k, len, n, ret = true, nil, string.len(s), 0, {}
		for i = 2, len do
			local c = string.sub(s, i, i)
			data = data .. c
			if b_new then
				b_new = false
				if c == "\0" then
					if n == 0 then
						break
					else
						n = n - 1
						if n == 0 then
							if k then
								ret[k], k = _data_store._unserialize(data), nil
							else
								k = _data_store._unserialize(data)
							end
							data = ""
						end
					end
				elseif c == "t" then
					b_new = true
					n = n + 1
				elseif c ~= "n" and c ~= "p" and c ~= "s" then
					error("Unkown type '" .. c .. "'")
				end
			elseif c == "\0" then
				if n == 0 then
					if k then
						ret[k], k = _data_store._unserialize(data), nil
					else
						k = _data_store._unserialize(data)
					end
					data = ""
				end
				b_new = true
			end
		end
		return ret
	end
end

function _data_store._load(id, player)
	local dir, t
	if player then
		dir = "players/" .. player
		t = _data_store._players[player]
		if not t then
			_data_store[player] = {n = 0}
			t = _data_store[player]
		end
	else
		dir = "global"
		t = _data_store
	end
	if not t[id] then
		local f = io.open("data/" .. dir .. "/" .. id .. ".dst", "rb")
		if not f then
			return false
		end
		t[id] = _data_store._unserialize(f:read("*a"))
		f:close()
		if t[id] then
			setmetatable(t[id], _DataStore_mt)
			return true
		end
	end
	return false
end

function _data_store._save(time, player, b_force)
	local ignore, t
	if player then
		ignore = _data_store._ignore_player
		t = _data_store._players[player]
	else
		ignore = _data_store._ignore
		t = _data_store
	end
	for k, v in pairs(t) do
		local b_proceed = true
		for k2, v2 in pairs(ignore) do
			if k == v2 then
				b_proceed = false
				break
			end
		end
		if b_proceed then
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
				local filename = "data/" .. dir .. "/" .. k .. ".dst"
				local f = io.open(filename, "wb")
				if f == nil then
					error("Couldn't open " .. filename)
				end
				f:write(_data_store._serialize(v))
				f:close()
			end
		end
	end
end

function _data_store.save(b_force)
	local ignore, players, time = _data_store._ignore, _data_store._players, os.time()
	_data_store._save(time)
	for player in players do
		local b_proceed = true
		for k, v in pairs(ignore) do
			if player == v then
				b_proceed = false
				break
			end
		end
		if b_proceed then
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
	if key == "_changed" then
		error("You can't change '_changed'")
	end
	if value ~= nil and type(value) == "userdata" and value.count then
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
		t = _data_store
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
