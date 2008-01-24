TopicList = {}
_TopicList_mt = {__index = TopicList}

-- Create a new topiclist object
function TopicList:New()
    local obj = {topics = {}}
    setmetatable(obj, _TopicList_mt)
    return obj
end

-- Add responses for the standard greeting phrases
-- (and for possibly additional greeting topics)
function TopicList:AddGreeting(topics, ...)
    if type(topics) ~= "table" then
        topics = {topics}
    end
    table.insert(topics, "greetings")
    table.insert(topics, "hello")
    table.insert(topics, "hey")
    table.insert(topics, "hi")
    self:AddTopics(topics, unpack(arg))
end

-- Add responses to the given topics
function TopicList:AddTopics(topics, ...)
    if type(topics) ~= "table" then
        topics = {topics}
    end
    table.insert(self.topics, {topics = topics, actions = arg})
end

-- Check the given message for topics
function TopicList:CheckMessage(event_param)
    if(event_param == nil) then
        print "TopicList - Warning: no event object to tl:CheckMessage(event)";
        event_param = event -- this will be deprecated
    end

    -- Internal function to execute the reponses tied to
    -- certain topics
    local function doactions(_action, _captures)
        for i, v in ipairs(_action) do
            local t = type(v)
            if t == "function" then
                v(unpack(_captures))
            elseif t == "string" then
                event_param.me:SayTo(event_param.activator, v)
            end
        end
    end

    local msg = string.lower(event_param.message)

    for i in pairs(self.topics) do
        local topics = self.topics[i]
        for j in pairs(topics.topics) do
            local captures = {string.find(msg, "^%s*".. topics.topics[j] .."%s*$")}
            if captures[1] then
                table.remove(captures,1) -- get rid of indices
                table.remove(captures,1)
                doactions(topics.actions, captures)
                return
            end
        end
    end

    -- if no match, execute the default actions (if any)
    if self.default ~= nil then
        doactions(self.default, {msg})
    end
end

-- Set the default reponse(s)
function TopicList:SetDefault(...)
    self.default = arg
end

-- Sets up the topiclist class
setmetatable(TopicList, {__call = TopicList.New})
