require("topic_list")

function topicData()
	-- Tests the storage of global data

	local dt = DataTable("data_store_test")
	local msg = table.concat(string.split(event.message), " ", 2)
	dt:set("activator", event.activator)
	dt:set("message", msg)
	event.me:SayTo(event.activator, "Message '" .. dt:get("message") .. "' saved.")
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
	tl:setDefault( function() event.me:Say("lvl3 tl: something wierd happened to the topiclist class") end)
	tl:addTopics("recursive", function() event.me:Say("lvl3 tl: recursive ( WRONG )") end)
	tl:addTopics("recursive2", function() event.me:Say("lvl3 tl: recursive2 ( WRONG )") end)
	tl:addTopics("recursive3", function() event.me:Say("lvl3 tl: recursive3 ( CORRECT )") end)
	tl:checkMessage()
end

tl = TopicList()
tl:setDefault([[

Welcome, you can test
^recursive^, ^recursive2^ scripts or the storage of ^data^.
If you want to test the second say
data <message>
This message will be the new message of the lost soul.]])

tl:addTopics("recursive", topicRecursive)
tl:addTopics("recursive2", topicRecursive2)
tl:addTopics("recursive3", topicRecursive3)
tl:addTopics("data.*", topicData)

tl:checkMessage()
