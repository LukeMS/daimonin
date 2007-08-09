-- merc_taleus.lua
-- Teaches a single archery skill. Quest missing.
--
require("topic_list")
require("quest_manager")
require("interface_builder")

local pl        = event.activator
local me        = event.me

local q_mgr_1 = QuestManager(pl, "Rats in Stonehaven?")
local q_status_1 = q_mgr_1:GetStatus()

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local skill_bow = game:GetSkillNr("bow archery")
local skill_cbow = game:GetSkillNr("crossbow archery")
local skill_sling = game:GetSkillNr("sling archery")

-- Guild checks
local guild_tag = "Mercenary"
local guild_stat = game.GUILD_NO
local guild_force = pl:GetGuild(guild_tag)
if guild_force ~= nil then
    guild_stat = guild_force.sub_type_1
end

local function quest_reminder()
    ib:SetTitle(q_mgr_1.name)
    ib:AddMsg("Haven't you taken care of it yet? Come back when you have!\n")
    ib:AddQuestChecklist(q_mgr_1)
    ib:SetButton("Back", "hi") 
end
    
local function quest_icons1()
    ib:AddSelect("Learn Archery skill of Sling ", "sling_small.101", " ")
    ib:AddSelect("Learn Archery skill of Bow ", "bow_short.101", " ")
    ib:AddSelect("Learn Archery skill of Crossbow ", "crossbow_small.101", " ")
end

local function quest_body1()
    ib:SetTitle(q_mgr_1.name)
    ib:SetMsg("We have had reports about strange noises from around one of the buildings on the east side of the town.\n\n")
    ib:SetMsg("The castle guards think it is rats and won't look into it. I must remain here in case my shipment arrives, and most other warriors are otherwise occupied. It would be great if you could investigate and take care of any problems.\n\n")
    ib:SetMsg("It seems most of the noises come from the first building on the eastern road. The woman who lives there will tell you more. Just follow this road south to the intersection by the main gates. Then go east and follow the road to the first house on the right hand side.")
    ib:SetDesc("Investigate the noises and handle any problems", 0, 0, 0, 0)
    quest_icons1()
end

function topicDefault()
    if pl:FindSkill(skill_bow) ~= nil or pl:FindSkill(skill_cbow) ~= nil or pl:FindSkill(skill_sling) ~= nil or q_status_1 == game.QSTAT_DONE then
        ib:SetTitle("Hello again")
        ib:AddMsg("Your basic mercenery training is probably almost finished by now. The priests in Stonhaven Village will help you to learn the final skills that you need.\n\nThe road to the village has been destroyed, unfortunately. However, I bet that the priest here in Stonehaven can help you getting there.")
    elseif q_status_1 == game.QSTAT_NO then
        ib:SetTitle("Greetings")
        ib:AddMsg("\n\nHello, mercenary. I am Archery Commander Taleus.")
        ib:AddMsg("\n\nI have taken over from my mentor, ^Chereth^, since she lost her eyes.")
        ib:AddMsg("\n\nWell, I know a lot about ^archery^ and I ^train^ members of the mercenary guild.")
    elseif q_status_1 == game.QSTAT_SOLVED then
        topQuestComplete()
        return
    elseif q_status_1 == game.QSTAT_ACTIVE then
        quest_reminder()
    else
        pl:Write(me.name .." has nothing to say.", game.COLOR_NAVY)
        pl:Interface(-1, "") 
        return
    end
    pl:Interface(1, ib:Build())
end

-- Player asks for quests
function topQuest()
    if q_status_1 == game.QSTAT_ACTIVE then
        quest_reminder()
    elseif q_status_1 == game.QSTAT_NO then 
        quest_body1()
        ib:SelectOff()
        ib:SetAccept(nil, "accept quest") 
        ib:SetDecline(nil, "hi") 
    else
        topicDefault()
        return
    end

    pl:Interface(1, ib:Build())
end

-- accepted: start the quest
function topAccept()
    if q_status_1 == game.QSTAT_NO then
        quest_body1()
        ib:SelectOff()
        if q_mgr_1:RegisterQuest(game.QUEST_ITEM, ib) then
            q_mgr_1:AddQuestItem(1, "quest_object", "power_crystal.101", "Raiding Party Clue")
            pl:Sound(0, 0, 2, 0)
            pl:Write("You take the quest '".. q_mgr_1.name .."'.", game.COLOR_NAVY)
        end
    else
        topicDefault()
        return
    end
    pl:Interface(-1, ib:Build())
end

-- try to finish: check the quest
function topQuestComplete()
    ib:SetTitle(q_mgr_1.name)
    if q_status_1 == game.QSTAT_ACTIVE then
        quest_reminder()
    else
        ib:AddMsg("Very well done! It was worse than I thought. I wonder where those kobolds came from--I thought we'd driven most of them out.\n\n")
        ib:AddMsg("I will take the crystal you found to Jahrlen, he might be able to undestand it.\n\n")
        ib:AddMsg("Now you must choose your archery skill:")
        quest_icons1()
        ib:AddQuestChecklist(q_mgr_1)
        ib:SetAccept(nil, "reward") 
        ib:SetDecline(nil, "hi") 
	end
	pl:Interface(1, ib:Build())
end

-- done: finish quest and give reward
function topQuestReward(reward)
    local rewards = {
        ["#1"] = { skill = skill_sling, weapon = "sling_small", ammo = "sstone" },
        ["#2"] = { skill = skill_bow, weapon = "bow_short", ammo = "arrow" },
        ["#3"] = { skill = skill_cbow, weapon = "crossbow_small", ammo = "bolt" }
    }
    
    if q_status_1 ~= game.QSTAT_SOLVED or not rewards[reward] then
        topicDefault()
        return
    end
    
    pl:AcquireSkill(rewards[reward].skill, game.LEARN)
    pl:CreateObjectInsideEx(rewards[reward].weapon, game.IDENTIFIED,1)
    pl:CreateObjectInsideEx(rewards[reward].ammo, game.IDENTIFIED,20)

    q_mgr_1:RemoveQuestItems()
    q_mgr_1:Finish()
    pl:Sound(0, 0, 2, 0)
    ib:SetMsg("Very well done! Here is your reward!")
    ib:SetButton("Ok", "hi") 
    pl:Interface(-1, ib:Build())
end

--
-- Rumors
--

local function topChereth()
    ib:SetTitle("Chereth")
    ib:SetMsg("\n\nYes, Chereth lost her eyes in a major battle. When you find her, she can tell you more.")
    ib:SetButton("Back", "hi")
    pl:Interface(1, ib:Build())
end

local function topArchery()
    ib:SetTitle("Archery Info")
    ib:SetMsg("\n\nYes, there are three archery skills:\n")
    ib:AddMsg("\nBow Archery is the most common, firing arrows.\n")
    ib:AddMsg("\nSling Archery allows fast firing stones with less damage.\n")
    ib:AddMsg("\nCrossbow Archery uses x-bows and bolts. Slow but powerful.\n")
    if guild_stat ~= game.GUILD_IN then
        ib:AddMsg("\nI teach archery for the mercenary guild, but I can see that you're not a member.\n")
        ib:AddMsg("You should talk to Cashin, who is in the Guildhall southeast of this castle, about joining the mercenary guild.")
    else
        ib:AddMsg("\nWell, there are the three different archery skills.\n\nI can teach you only ~*ONE*~ of them.\n")
        ib:AddMsg("You have to stick with it, so |choose wisely|.\n")
        ib:AddMsg("But before I teach you, I have a little ^quest^ for you.")
        ib:AddLink("Could you tell me what this 'quest' is?", "explain quest")
    end
    ib:SetButton("Back", "hi")
    pl:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
if q_mgr_1:GetStatus() < game.QSTAT_DONE then
    -- Can only take the quest if guildmember
    if guild_stat == game.GUILD_IN then
        tl:AddTopics({"quest", "explain%s+quest"}, topQuest)
        tl:AddTopics({"accept", "accept%s+quest"}, topAccept) 
    end
    tl:AddTopics({"complete", "quest%s+complete"}, topQuestComplete) 
    tl:AddTopics({"reward%s+(#%d+)"}, topQuestReward)
end
tl:AddTopics({"archery", "train"}, topArchery)
tl:AddTopics("chereth", topChereth)

tl:CheckMessage(event)
