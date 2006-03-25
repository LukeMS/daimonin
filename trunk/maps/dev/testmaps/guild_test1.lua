-- guild template script 1 + guild item quest
require("topic_list")
require("quest_check")
require("interface_builder")

local pl 		= event.activator
local me        = event.me
local msg       = string.lower(event.message)

local q_name_1  = "guild test quest1"
local q_step_1  = 0
local q_level_1  = 1
local q_skill_1  = game.ITEM_SKILL_NO

local q_obj_1  = nil
local q_stat_1 = game.QSTAT_DONE

local guild_tag = "Test Guild 1"
local guild_rank = ""
local guild_stat = game.GUILD_NO
local guild_force = nil

local function setGuild()
guild_force = pl:GetGuild(guild_tag)
if guild_force == nil then
q_obj_1   = pl:GetQuest(q_name_1)
q_stat_1  = Q_Status(pl, q_obj_1, q_step_1, q_level_1, q_skill_1)
else
guild_stat = guild_force.sub_type_1
end
end

setGuild()
local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
if q_stat_1 < game.QSTAT_DONE then
if q_stat_1 == game.QSTAT_NO then
ib:SetTitle("Test Guild 1")
ib:AddMsg("[intro] You can join when you fetch me the guild hammer (in the chest there)")
ib:AddLink("Start the Guild Quest", "startq1")
else
ib:SetTitle("Test Guild 1 Quest solved?")
ib:AddMsg("[pending] You has done the quest?")
ib:AddLink("Join the Test Guild 1", "checkq1")
end
else
if guild_stat == game.GUILD_OLD then
ib:AddLink("Rejoin the Test Guild 1", "askjoing1")
ib:SetTitle("Hello Friend")
else
ib:SetTitle("Hello Guildmember")
end
ib:AddMsg("Good to see you back. How are you?")
end
ib:AddMsg("\n\n[DEVMSG] The guild status is: ".. guild_stat .."\n")
ib:AddMsg("[DEVMSG] The quest status is: ".. q_stat_1)
pl:Interface(1, ib:Build())
end

-- quest body (added to player quest obj for quest list)
local function quest_icons1()
ib:AddIcon("Test Guild 1 Membership", "guild_merc", "") 
end
local function quest_body1()
ib:SetMsg("[WHY] Get the item this test quest needs.")
ib:SetDesc("[WHAT] Bring me the 'test guild 1 helm' - its in the chest. Then you can join.", 0, 0, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ1()
if q_stat_1 ~= game.QSTAT_NO then
topicDefault()
else
ib:SetTitle("START: Test Guild 1 Quest")
quest_body1()
quest_icons1()
ib:SetAccept(nil, "acceptq1") 
ib:SetDecline(nil, "hi") 
pl:Interface(1, ib:Build())
end
end

-- accepted: start the quest
local function topAcceptQ1()
if q_stat_1 == game.QSTAT_NO then
quest_body1()
quest_icons1()
q_obj_1 = pl:AddQuest(q_name_1, game.QUEST_ITEM, q_step_1, q_step_1, q_level_1, q_skill_1, ib:Build())
if q_obj_1 ~= null then
q_obj_1:AddQuestItem(1, "quest_object", "helm_leather.101", "test guild 1 helm")
q_stat_1 = Q_Status(pl, q_obj_1, q_step_1, q_level_1, q_skill_1)
pl:Sound(0, 0, 2, 0)
pl:Write("You take the quest 'Test Guild 1 Quest'.", game.COLOR_NAVY)
end
ib = InterfaceBuilder()
ib:SetHeader(me, me.name)
end
topicDefault()
end

-- try to finish: check the quest
local function topCheckQ1()
if q_stat_1 == game.QSTAT_NO then
topicDefault()
else
ib:SetTitle("FINAL CHECK: Test Guild 1 Quest")
ib:SetMsg("[DEVMSG] The quest status is: ".. q_stat_1 .."\n\n")
if q_stat_1 ~= game.QSTAT_SOLVED then
ib:AddMsg("[not-done-text] Come back if you have it!\n")
Q_List(q_obj_1, ib)
ib:SetButton("Back", "hi") 
else
ib:AddMsg("[final-text] Very well done! You found the helm.\n")
ib:SetDesc("here it is...", 0, 0, 0, 0)
quest_icons1()
Q_List(q_obj_1, ib)
ib:SetAccept(nil, "finishq1") 
ib:SetDecline(nil, "hi") 
end
pl:Interface(1, ib:Build())
end
end

-- done: finish quest and give reward
local function topFinishQ1()
if q_stat_1 ~= game.QSTAT_SOLVED then
topicDefault()
else
q_obj_1:RemoveQuestItem()
q_obj_1:SetQuestStatus(-1)
q_stat_1 = game.QSTAT_DONE
pl:Sound(0, 0, 2, 0)
pl:JoinGuild(guild_tag, game.SKILLGROUP_PHYSIQUE, 100, game.SKILLGROUP_AGILITY, 100, game.SKILLGROUP_WISDOM, 100)
setGuild()
ib:SetTitle("QUEST END: Test Guild 1 Quest")
ib:SetMsg("Very well done! You are now a member of...!")
ib:SetButton("Ok", "hi") 
pl:Interface(1, ib:Build())
end
end

local function topAskjoinG1()
if guild_stat ~= game.GUILD_OLD then
topicDefault()
end
ib:SetTitle("Rejoin our Guild?")
ib:SetMsg("You really want rejoin our guild?\n\nWe would be proud to have you back in our ranks!")
ib:SetAccept(nil, "rejoing1") 
ib:SetDecline(nil, "hi") 
pl:Interface(1, ib:Build())
end

local function topRejoinG1()
if guild_stat ~= game.GUILD_OLD then
topicDefault()
end
pl:Sound(0, 0, 2, 0)
pl:JoinGuild(guild_tag)
setGuild()
-- perhaps rejoin has bad effects. Then manipulate the guild force now HERE
ib:SetTitle("Welcome Back!")
ib:SetMsg("Welcome back to our guild!")
ib:SetButton("Ok", "hi") 
pl:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
if q_stat_1 < game.QSTAT_DONE then
tl:AddTopics("startq1", topStartQ1) 
tl:AddTopics("acceptq1", topAcceptQ1) 
tl:AddTopics("checkq1", topCheckQ1)
tl:AddTopics("finishq1", topFinishQ1)
end 
if guild_stat == game.GUILD_OLD then
tl:AddTopics("askjoing1", topAskjoinG1)
tl:AddTopics("rejoing1", topRejoinG1)
end
tl:CheckMessage(event)
