-------------------------------------------------------------------------------
-- topic_list.lua | Utility
-- 
-- The topic list is for TALK Event scripts.
-------------------------------------------------------------------------------
---------------------------------------
-- If TopicList already exists then check it is the correct addon type.
-- If it is not, this is an error. If it is, we (presumably) have already
-- required this chunk so no need to do it again.
---------------------------------------
if TopicList ~= nil then
    local a, b, c = type(TopicList)
    assert(a == "addon" and
        b == "utility" and
        c == "tl",
        "Global TopicList exists but is not an addon utility tl!")
    return
end

---------------------------------------
-- Assign the global TopicList table. Give it a metatable. The __call
-- metamethod means that calling TopicList() returns a new instance of
-- the addon (as specified by the __metatable metamethod).
---------------------------------------
TopicList = { topics = {} }
setmetatable(TopicList, {
    __call = function()
        ---------
        -- t is just a table so we give it a metatable. The __index event means
        -- that when we index t we treat it like TopicList.
        ---------
        local t = {}
        setmetatable(t, {
            __index = TopicList,
            __metatable = function() return "addon", "utility", "tl" end,
        })
        return t
    end,
    __metatable = function() return "addon", "utility", "tl" end
})

-------------------
-- tl:CheckMessage() compares what was typed by the player to all triggers.
-- If it finds a match it executes any associated action(s).
-- If it doesn't find a match it executes any default action(s).
-- The second argument (quiet) is passed to doactions() (local function) and
-- may thus cause this function to return a string as its second return.
-------------------
function TopicList:CheckMessage(ev, quiet)
    assert(type(ev) == "Event" or
           ev == nil, "Arg #1 must be Event or nil!")
    assert(type(quiet) == "boolean" or
           quiet == nil, "Arg #2 must be boolean or nil!")

    if ev == nil then
        ev = event
    end

    ---------
    -- doactions() runs through all the actions it is given.
    ---------
    local function doactions(action, captures)
        for i, v in ipairs(action) do
            local t = type(v)

            if t == "function" then
                local r = v(unpack(captures))

                if r then
                    r = true
                end

                if r ~= nil then
                    return r
                end
            elseif t == "string" then
                if not quiet then
                    ev.me:SayTo(ev.activator, v)

                    return true
                else
                    return v
                end
            end
        end
    end

    ---------
    -- Normalise and lowercase the text typed by the player.
    ---------
    --local msg = string.lower(table.concat(string.split(ev.message), " "))

    ---------
    -- Compare message to all known topics, executing associated action(s) if a
    -- match is found.
    ---------
    for i, topic in ipairs(self.topics) do
        for j, trigger in ipairs(topic.triggers) do
            local captures = {
                string.find(ev.message, "^%s*".. trigger .."%s*$")
            }
            if captures[1] ~= nil then
                ---------
                -- Get rid of indices.
                ---------
                table.remove(captures, 1)
                table.remove(captures, 1)

                return ev, doactions(topic.actions, captures)
            end
        end
    end

    ---------
    -- If no match was found, execute the default action(s), if any.
    ---------
    if type(self.default) == "table" then
        return ev, doactions(self.default, { message })
    end

    ---------
    -- No default action(s) either? Fine, you win, let bail out.
    ---------
    return ev
end

---------------------------------------
-- Default response.
---------------------------------------
-------------------
-- tl:SetDefault() sets the default action(s)
-------------------
function TopicList:SetDefault(...)
    local action = { }

    for i = 1, arg.n do
        if type(arg[i]) == "table" then
            for j in ipairs(arg[i]) do
                assert(type(arg[i][j]) == "string" or
                       type(arg[i][j]) == "function",
                       "Arg #1+ must be string(s), function(s), or " ..
                       "table(s) of string(s) and/or function(s)!")
                table.insert(action, arg[i][j])
            end
        else
            assert(type(arg[i]) == "string" or
                   type(arg[i]) == "function",
                   "Arg #1+ must be string(s), function(s), or table(s) " ..
                   "of string(s) and/or function(s)!")
            table.insert(action, arg[i])
        end
    end 

    self.default = action
end

---------------------------------------
-- Topics.
---------------------------------------
-------------------
-- tl:AddTopics adds a custom topic.
-------------------
function TopicList:AddTopics(trigger, ...)
    local action = { }

    if type(trigger) ~= "table" then
        trigger = {
            trigger
        }
    end

    for i, v in ipairs(trigger) do
        assert(type(v) == "string" or
               v == nil, "Arg #1 must be string, table of strings, or nil!")
    end

    for i = 1, arg.n do
        if type(arg[i]) == "table" then
            for j in ipairs(arg[i]) do
                assert(type(arg[i][j]) == "string" or
                       type(arg[i][j]) == "function",
                       "Arg #2+ must be string(s), function(s), or " ..
                       "table(s) of string(s) and/or function(s)!")
                table.insert(action, arg[i][j])
            end
        else
            assert(type(arg[i]) == "string" or
                   type(arg[i]) == "function",
                   "Arg #2+ must be string(s), function(s), or table(s) " ..
                   "of string(s) and/or function(s)!")
            table.insert(action, arg[i])
        end
    end 

    table.insert(self.topics, 1, { triggers = trigger,
                                   actions = action })
end

-------------------
-- tl:AddGreeting() adds the 'greetings' standard topic.
-------------------
function TopicList:AddGreeting(trigger, ...)
    local action = {
        unpack(arg, 1, arg.n)
    }

    if type(trigger) ~= "table" then
        trigger = {
            trigger
        }
    end

    table.insert(trigger, "greetings")
    table.insert(trigger, "hello")
    table.insert(trigger, "hey")
    table.insert(trigger, "hi")
    self:AddTopics(trigger, action)
end

-------------------
-- tl:AddBackground() adds the 'background' standard topic.
-------------------
function TopicList:AddBackground(trigger, ...)
    local action = {
        unpack(arg, 1, arg.n)
    }

    if type(trigger) ~= "table" then
        trigger = {
            trigger
        }
    end

    table.insert(trigger, "background")
    table.insert(trigger, "you")
    table.insert(trigger, "yourself")
    self:AddTopics(trigger, action)
end

-------------------
-- tl:AddQuest() adds the 'quest' standard topic.
-------------------
function TopicList:AddQuest(trigger, ...)
    local action = {
        unpack(arg, 1, arg.n)
    }

    if type(trigger) ~= "table" then
        trigger = {
            trigger
        }
    end

    table.insert(trigger, "quests?")
    self:AddTopics(trigger, action)
end

-------------------
-- tl:AddRumours() adds the 'rumours' standard topic.
-------------------
function TopicList:AddRumours(trigger, ...)
    local action = {
        unpack(arg, 1, arg.n)
    }

    if type(trigger) ~= "table" then
        trigger = {
            trigger
        }
    end

    table.insert(trigger, "rumours?")
    table.insert(trigger, "rumors?")
    self:AddTopics(trigger, action)
end

function TopicList:AddRumors(...)
    self:AddRumours(unpack(arg))
end

-------------------
-- tl:AddServices adds the (optional) 'services' standard topic.
-------------------
function TopicList:AddServices(trigger, ...)
    local action = {
        unpack(arg, 1, arg.n)
    }

    if type(trigger) ~= "table" then
        trigger = {
            trigger
        }
    end

    table.insert(trigger, "services?")
    self:AddTopics(trigger, action)
end
