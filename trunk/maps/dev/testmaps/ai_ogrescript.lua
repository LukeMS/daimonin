--[[
--  Shows how the AI object can be used to modify a mob's
--  friendship value in a normal chat script
--]]
require("topic_list");

function modify_friendship(op, friend, change)
    local ai = op:GetAI()
    ai:Register(friend, change, 0)
    op:Say("My new friendship towards " .. friend.name .. ": " .. ai:GetFriendship(friend))
end

function insult_func()
    modify_friendship(event.me, event.activator, -200)
end

function compliment_func()
    modify_friendship(event.me, event.activator, 200)
end

function dump_func()
    ai = event.me:GetAI():GetBehaviourlist()
    ds=DataStore('example_store')
    ds:Set('ai', ai)
    _data_store.save()
end

tl = TopicList()
tl:AddTopics("insult", insult_func)
tl:AddTopics("compliment", compliment_func)
tl:AddTopics("dump", dump_func)
tl:SetDefault("You can ^insult^ or ^compliment^ me to change my attitude towards you")
tl:CheckMessage(event)
