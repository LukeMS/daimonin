require("topic_list")

string.split("")

function topicData()
	-- Tests the storage of global data

	local dt = DataTable("data_store_test")
	local msg = table.concat(string.split(event.message), " ", 2)
	dt:set("activator", event.activator)
	dt:set("message", msg)
	local msg2 = dt:get("message")
	if msg2 == msg then
		event.me:SayTo(event.activator, "Message '" .. msg .. "' saved.\nNow talk to the lost soul to see this message.")
	else
		event.me:SayTo(event.activator, "Error: Message wasn't saved!")
	end
end

function topicData2()
	-- Tests the storage of data for a player

	local msg = string.split(event.message)
	if table.getn(msg) < 3 then
		event.me:SayTo(event.activator, "Say '^data2^ <player> <message>'")
		return
	end
	local dt = DataTable("data_store_test", msg[2])
	msg = table.concat(msg, " ", 3)
	dt:set("activator", event.activator)
	dt:set("message", msg)
	local msg2 = dt:get("message")
	if msg2 == msg then
		event.me:SayTo(event.activator, "Message '" .. msg .. "' saved.\nThe player will see this message if (s)he talks to the lost soul.")
	else
		event.me:SayTo(event.activator, "Error: Message wasn't saved!")
	end
end

function topicInfo()
	local msg = "\nContents of data_store:\nGlobal:"
	for k1, v1 in pairs(data_store) do
		if k1 ~= "n" and k1 ~= "_players" then
			msg = msg .. "\n'" .. tostring(k1) .. "' => DataTable:\nLast change: " .. os.date("!%Y-%m-%d %H:%M:%S %Z", v1._changed) .. "\nContents:\n---"
			for k2, v2 in pairs(v1) do
				if k2 ~= "_changed" then
					msg = msg .. "\n'" .. tostring(k2) .. "' => '" .. tostring(v2) .. "'"
				end
			end
			msg = msg .. "\n---"
		end
	end
	msg = msg .. "\nLocal:"
	for k1, v1 in pairs(data_store._players) do
		msg = msg .. "\nPlayer '" .. tostring(k1) .. "':"
		for k2, v2 in pairs(v1) do
			if k2 ~= "n" then
				msg = msg .. "\n'" .. tostring(k2) .. "' => DataTable:\nLast change: " .. os.date("!%Y-%m-%d %H:%M:%S %Z", v2._changed) .. "\nContents:\n---"
				for k3, v3 in pairs(v2) do
					if k3 ~= "_changed" then
						msg = msg .. "\n'" .. tostring(k3) .. "' => '" .. tostring(v3) .. "'"
					end
				end
				msg = msg .. "\n---"
			end
		end
	end
	event.me:SayTo(event.activator, msg)
end

function topicRecursive()
	-- Tests whether the "event" global is untouched after
	-- a recursive script trigger.
	--
	-- The Communicate() call below triggers LuaTester's script
	-- before continuing execution.

	-- print ("Drow before: " .. event.me.name)

	local name = event.me.name
	event.me:Communicate("This is a test, my name is " .. event.me.name)

	-- print ("Drow after: " .. event.me.name)

	local newname = event.me.name
	if newname == name then
    	event.me:Say("Test successful, I'm still me");
	else
    	event.me:Say("Test failed! I'm not " .. name .. " anymore ");
	end
end

function topicRecursive2()
	-- Tests whether the "event" global is untouched after
	-- a multi-level recursive script trigger.
	-- Also tests external classes (topic list) for wierd globals changes

	local message = event.message
	event.me:Say("Ok, initiating recursion... (message was "..message..")");
	event.me:Communicate("recursive_part_2")

	local newmessage = event.message
	if newmessage == message then
    	event.me:Say("Test successful, the global event.message didn't change from " .. message);
	else
    	event.me:Say("Test failed! Event.message changed from "..message.." to "..newmessage);
	end
end

function topicRecursive3()
	event.me:Say("Third level of recursion. Wee! (message = "..event.message..")");
	local tl = TopicList()
	tl:setDefault( function() event.me:Say("lvl3 tl: something weird happened to the topiclist class") end)
	tl:addTopics("recursive", function() event.me:Say("lvl3 tl: recursive ( WRONG )") end)
	tl:addTopics("recursive2", function() event.me:Say("lvl3 tl: recursive2 ( WRONG )") end)
	tl:addTopics("recursive3", function() event.me:Say("lvl3 tl: recursive3 ( CORRECT )") end)
	tl:checkMessage()
end

tl = TopicList()
tl:setDefault([[

Available tests/topics:
^recursive^
^recursive2^
^data^ <message>
^data2^ <player> <message>
^info^

^recursive^ and ^recursive2^ tests recursive scripts.
^data^ tests the storage of global data.
^data2^ tests the storage of data for a player.
^info^ dumps the contents of 'data_store'.]])

tl:addTopics("recursive", topicRecursive)
tl:addTopics("recursive2", topicRecursive2)
tl:addTopics("recursive3", topicRecursive3)
tl:addTopics("data2.*", topicData2)
tl:addTopics("data.*", topicData)
tl:addTopics("info", topicInfo)

tl:checkMessage()
