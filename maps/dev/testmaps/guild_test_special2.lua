-- guid special command test script 2
require("topic_list")
require("quest_check")
require("interface_builder")

local activator = event.activator
local me        = event.me
local msg       = string.lower(event.message)

local guild_tag = "Test Guild Special 2"
local guild_rank = ""
local guild_stat = game.GUILD_NO
local guild_force = nil

local function setGuild()
guild_force = activator:GetGuild(guild_tag)
if guild_force ~= nil then
guild_stat = guild_force.sub_type_1
end
end

setGuild()
local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
ib:SetTitle("Test Guild Special *2*")
ib:AddToMessage("\n\n[DEVMSG] The guild status is: ".. guild_stat .."\n")
ib:AddLink("LEAVE current guild!", "leaveg1")
ib:AddLink("JOIN / REJOIN the Test Guild Special 2", "joing1")
activator:Interface(1, ib:Build())
end

local function topJoinG1()
-- for a rejoin its enough to give the guild_tag
-- activator:JoinGuild(guild_tag)
activator:JoinGuild(guild_tag, game.SKILLGROUP_PHYSIQUE, game.SKILLGROUP_AGILITY, game.SKILLGROUP_WISDOM, 100, 100, 100)
setGuild()
topicDefault()
end

local function topLeaveG1()
activator:LeaveGuild()
setGuild()
topicDefault()
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("joing1", topJoinG1) 
tl:AddTopics("leaveg1", topLeaveG1) 
tl:CheckMessage(event)
