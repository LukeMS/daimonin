-- Merc Guild Script
require("topic_list")
require("quest_check")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)

local q_name_1  = "The Mercenary Guild Quest"
local q_step_1  = 0
local q_level_1  = 1
local q_skill_1  = game.ITEM_SKILL_NO
local q_iname1 = "Cashin's leather cap"

local q_obj_1  = nil
local q_stat_1 = game.QSTAT_DONE

local guild_tag = "Mercenary"
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
ib:SetTitle("Welcome to the Mercenary Guild")
ib:AddMsg("Hello!\nWelcome to the Mercenary Guild of Thraal!\n\nYou need a job?\nYou really would be welcome in our ranks. Our ^troops^ are an important part of the imperial defence.\n\nYou want join our guild?")
ib:AddLink("Start the Mercenry Guild Quest", "startq1")
else
ib:SetTitle("You Solved the Quest?")
ib:AddMsg("You has done the task?\n\nShow me the helm and you can join our guild.")
ib:AddLink("Show Cashin the Helm", "checkq1")
end
else
if guild_stat == game.GUILD_OLD then
ib:AddLink("Rejoin the Mercenary Guild", "askjoing1")
ib:SetTitle("Welcome Back")
else
ib:SetTitle("Hello Guildmember")
end
ib:AddMsg("Good to see you back. How are you?\n\nNice that you have joined our ^troops^.")
end
pl:Interface(1, ib:Build())
end

-- quest body (added to player quest obj for quest list)
local function quest_icons1()
ib:AddIcon("Mercenary Guild Membership", "guild_merc", "") 
end
local function quest_body1()
ib:SetMsg("Before you can join the guild I have a small task for you.\n\nWe have a problem with giant ants in the last time.\nSee this hole to the old guild cellar on my side!\n\nOne of those silly ants has stolen my old helmet!\nEnter the hole and kill the ants there!\nNo fear, they are weak.\n\nBring me the helm back and you can join the guild.")
ib:SetDesc("You can join the mercenary guild when you recover the helm for Cashin.", 0, 0, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ1()
if q_stat_1 ~= game.QSTAT_NO then
topicDefault()
else
ib:SetTitle(q_name_1)
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
q_obj_1:AddQuestItem(1, "quest_object", "helm_leather.101", q_iname1)
q_stat_1 = Q_Status(pl, q_obj_1, q_step_1, q_level_1, q_skill_1)
pl:Sound(0, 0, 2, 0)
pl:Write("You take the quest '".. q_name_1 .."'.", game.COLOR_NAVY)
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
ib:SetTitle("Join the Mercenary Guild")
if q_stat_1 ~= game.QSTAT_SOLVED then
ib:AddMsg("Where is my helm???\n\nCome back if you have found it!\n")
Q_List(q_obj_1, ib)
ib:SetButton("Back", "hi") 
else
ib:AddMsg("Very well done! You found the helm.\n\nYou can keep the helm.\nIt will protect you well.\n")
ib:SetDesc("Join the Mercenary Guild and take the Helm.", 0, 0, 0, 0)
ib:AddIcon(q_iname1, "helm_leather.101", "") 
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
local q_i1 = pl:CreateObjectInside("helm_leather", 1,1) 
q_i1.name = q_iname1
pl:Write("You got ".. q_iname1..".", game.COLOR_WHITE) 
pl:JoinGuild(guild_tag, game.SKILLGROUP_PHYSIQUE, 100, game.SKILLGROUP_AGILITY, 100, game.SKILLGROUP_WISDOM, 100)
setGuild()
ib:SetTitle("Welcome Mercenary!")
ib:SetMsg("Very well done!\n\nYou are now a member of the Mercenary Guild!")
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
ib:SetTitle("Welcome Back!")
ib:SetMsg("Welcome back to our guild!")
ib:SetButton("Ok", "hi") 
pl:Interface(1, ib:Build())
end

local function topTroops()
ib:SetTitle("The Mercenary Troops")
ib:SetMsg("We, as part of the the Thraal army corps, are invading the abandomed areas after the defeat of Moroch.\n\nWell, the chronomancers ensured us after they created the portal that we are still in the galactic main sphere.\n\nBut it seems to me that these lands have many wormholes to other places...\n\nPerhaps the long time under Morochs influence has weakened the borders between the planes. You should ask ^Jahrlen^ about it.")
ib:SetButton("Ok", "hi") 
pl:Interface(1, ib:Build())
end

local function topJahrlen()
ib:SetTitle("Jahrlen the Chronomancer")
ib:SetMsg("Jahrlen is our guild mage.\n\nWell, normally we don't have a guild mage.\nBut we are at war here and he was assigned to us.\n\nIn fact, he is a high level chronomancer and we are honored he helps us.\n\nHe is in our guild rooms. Talk to him when you meet him! He often has tasks and quests for newbies.")
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
tl:AddTopics("troops", topTroops)
tl:AddTopics("jahrlen", topJahrlen)
tl:CheckMessage(event)
