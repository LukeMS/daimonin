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

tl = TopicList()
tl:setDefault([[

Welcome, you can test
^recursive^ scripts or the storage of ^data^.
If you want to test the second say
data <message>
This message will be the new message of the lost soul.]])

tl:addTopics("recursive", topicRecursive)
tl:addTopics("data.*", topicData)

tl:checkMessage()
