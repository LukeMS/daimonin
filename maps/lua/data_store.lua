data_store = {n = 0}

--[[
function data_store.save()
	local time = os.time()
	for i in data_store do
		if i ~= "n" then
			local table = rawget(data_store, i)
			local changed = rawget(table, "_changed")
			if changed ~= 0 and os.difftime(time, changed) >= 120 then
				-- TODO: Save the table in its file --
			end
		end
	end
end
]]

DataTable = {}

function DataTable.get(table, key)
	local value = rawget(table, key)
	if type(value) == "table" and value.id and value.object then
		local object = value.object
		if object and object.count == value.id then
			value = object
		else
			value = nil
		end
	end
	return value
end

function DataTable.set(table, key, value)
	if value ~= nil and type(value) == "userdata" and value.count then
		value = {id = value.count, object = value}
	end
	rawset(table, key, value)
	rawset(table, "_changed", os.time())
end

DataTable_mt = {__index = DataTable, __newindex = function() error("Use set() to add/change values") end}

function DataTable.new(id)
	local obj = data_store[id]
	if obj == nil then
		obj = {_changed = 0}
		setmetatable(obj, DataTable_mt)
		data_store[id] = obj
		data_store.n = table.getn(data_store) + 1
	end
	return obj
end

setmetatable(DataTable, {__call = DataTable.new})
