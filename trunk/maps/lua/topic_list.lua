TopicList = {}
_TopicList_mt = {__index = TopicList}

function TopicList:new()
	local obj = {topics = {}}
	setmetatable(obj, _TopicList_mt)
	return obj
end

function TopicList:addGreeting(topics, ...)
	if type(topics) ~= "table" then
		topics = {topics}
	end
	table.insert(topics, "greetings")
	table.insert(topics, "hello")
	table.insert(topics, "hey")
	table.insert(topics, "hi")
	self:addTopics(topics, unpack(arg))
end

function TopicList:addTopics(topics, ...)
	if type(topics) ~= "table" then
		topics = {topics}
	end
	table.insert(self.topics, {topics = topics, actions = arg})
end

function TopicList:_doActions(event, actions, captures)
	table.foreach(actions, function(k,v)
		if type(v) == "function" then
			v(unpack(captures))
		elseif type(v) == "string" then
			event.me:SayTo(event.activator, v)
		end
	end)
end

function TopicList:checkMessage()
	local msg = string.lower(event.message)

    for i in pairs(self.topics) do
		local topics = self.topics[i]
		for j in pairs(topics.topics) do
			local captures = {string.find(msg, "^".. topics.topics[j] .."$")}
			if captures[1] then
				table.remove(captures,1) -- get rid of indices
				table.remove(captures,1)
				self:_doActions(event, topics.actions, captures)
				return
			end
		end
	end

	-- if no match
	if self.default ~= nil then
		self:_doActions(event, self.default, {})
	end
end

function TopicList:setDefault(...)
	self.default = arg
end

setmetatable(TopicList, {__call = TopicList.new})
