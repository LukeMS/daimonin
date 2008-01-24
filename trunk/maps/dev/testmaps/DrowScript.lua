require("topic_list")

string.split("")

-- Tests the storage of global data
function topicData(msg)
    local ds = DataStore("data_store_test")
    ds:Set("activator", event.activator)
    ds:Set("message", msg)
    local msg2 = ds:Get("message")
    if msg2 == msg then
        event.me:SayTo(event.activator, "Message '" .. msg .. "' saved.\nNow talk to the lost soul to see this message.")
    else
        event.me:SayTo(event.activator, "Error: Message wasn't saved!")
    end
end

-- Tests the storage of data for a player
function topicData2(player, msg)
    if player == nil or msg == nil then
        event.me:SayTo(event.activator, "Say '^data2^ <player> <message>'")
        return
    end
    local ds = DataStore("data_store_test", player)
    ds:Set("activator", event.activator)
    ds:Set("message", msg)
    local msg2 = ds:Get("message")
    if msg2 == msg then
        event.me:SayTo(event.activator, "Message '" .. msg .. "' saved.\nThe player will see this message if (s)he talks to the lost soul.")
    else
        event.me:SayTo(event.activator, "Error: Message wasn't saved!")
    end
end

-- Tests the storage of a function
function topicData3()
    local ds = DataStore("data_store_test")
    local function testfunc(event)
        -- The global event won't be the same when we are called
        event.me:Say("Hello world!")
    end

    ds:Set("function", testfunc)
    local f2 = ds:Get("function")
    if f2 == testfunc then
        event.me:SayTo(event.activator, "Function saved.\n Talk to the lost soul to run it.")
    else
        event.me:SayTo(event.activator, "Error: Function wasn't saved!")
    end
end

-- Tests the storage of special values
function topicData4()
    local ds = DataStore("data_store_test")

    ds:Set("special1", event.me)
    ds:Set("special2", event.me.map)
    
    local s1 = ds:Get("special1")
    local s2 = ds:Get("special2")
--    if s1 == event.me and s2 == event.me.map then
    if s1 == event.me and s2 == event.me.map then
        event.me:SayTo(event.activator, "Specials stored. Talk to lost soul to get them back")
    else
        event.me:SayTo(event.activator, "Error: Specials weren't saved! s1="..tostring(s1)..", s2="..tostring(s2))
    end
end

function topicInfo()
    local msg = "\nContents of data_store:\nGlobal:"
    for k1, v1 in pairs(_data_store._global) do
        if k1 ~= "n" then
            msg = msg .. "\n'" .. tostring(k1) .. "' => DataStore:\nLast change: " .. os.date("!%Y-%m-%d %H:%M:%S %Z", v1._changed) .. "\nContents:\n---"
            for k2, v2 in pairs(v1) do
                if k2 ~= "_changed" then
                    msg = msg .. "\n'" .. tostring(k2) .. "' => '" .. tostring(v2) .. "'"
                end
            end
            msg = msg .. "\n---"
        end
    end
    msg = msg .. "\nLocal:"
    for k1, v1 in pairs(_data_store._players) do
        msg = msg .. "\nPlayer '" .. tostring(k1) .. "':"
        for k2, v2 in pairs(v1) do
            if k2 ~= "n" then
                msg = msg .. "\n'" .. tostring(k2) .. "' => DataStore:\nLast change: " .. os.date("!%Y-%m-%d %H:%M:%S %Z", v2._changed) .. "\nContents:\n---"
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

function topicSave()
    _data_store.save(true)
    event.me:SayTo(event.activator, "Done.")
end

-- Tests whether the "event" global is untouched after
-- a recursive script trigger.
function topicRecursive()
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

-- Tests whether the "event" global is untouched after
-- a multi-level recursive script trigger.
-- Also tests external classes (topic list) for wierd globals changes
function topicRecursive2()
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
    tl:SetDefault( function() event.me:Say("lvl3 tl: something weird happened to the topiclist class") end)
    tl:AddTopics("recursive", function() event.me:Say("lvl3 tl: recursive ( WRONG )") end)
    tl:AddTopics("recursive2", function() event.me:Say("lvl3 tl: recursive2 ( WRONG )") end)
    tl:AddTopics("recursive3", function() event.me:Say("lvl3 tl: recursive3 ( CORRECT )") end)
    tl:CheckMessage(event)
end

tl = TopicList()
tl:SetDefault([[
Available tests/topics:
^recursive^
^recursive2^
^data^ <message>
^data2^ <player> <message>
^data3^
^data4^
^info^
^save^

^recursive^ and ^recursive2^ tests recursive scripts.
^data^ tests the storage of global data.
^data2^ tests the storage of data for a player.
^data3^ tests the storage of functions.
^data4^ tests the storage of objects and other special types.
^info^ dumps the contents of '_data_store'.
^save^ saves '_data_store'.]])

tl:AddTopics("recursive", topicRecursive)
tl:AddTopics("recursive2", topicRecursive2)
tl:AddTopics("recursive3", topicRecursive3)
tl:AddTopics("data4", topicData4)
tl:AddTopics("data3", topicData3)
tl:AddTopics("data2 ([^ ]*) (.*)", topicData2)
tl:AddTopics("data (.*)", topicData)
tl:AddTopics("info", topicInfo)
tl:AddTopics("save", topicSave)

tl:CheckMessage(event)
