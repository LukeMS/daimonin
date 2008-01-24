-- Merc Guild Script
require("topic_list")
require("quest_manager")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)
local q_iname1 = "Cashin's leather cap"
local guild_tag = "Mercenary"
local guild_rank = ""
local guild_stat = game.GUILD_NO
local guild_force = nil
local q_mgr_1   = QuestManager(pl,"The Mercenary Guild Quest")

local function setGuild()
    guild_force = pl:GetGuild(guild_tag)
    if guild_force ~= nil then
        guild_stat = guild_force.sub_type_1
    end
end
setGuild()
local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
    if q_mgr_1:GetStatus() < game.QSTAT_DONE then
        if q_mgr_1:GetStatus() == game.QSTAT_NO then
            ib:SetTitle("Welcome to the Mercenary Guild")
            ib:AddMsg("Hello!\nWelcome to the Mercenary Guild of Thraal!\n")
            ib:AddMsg("\nDo you need a job?")
            ib:AddMsg("\nYou really would be welcome in our ranks. ")
            ib:AddMsg("Our ^troops^ are an important part of the imperial defence.\n")
            ib:AddMsg("\nDo you want to join our guild?")
            ib:AddLink("Start the Mercenary Guild Quest", "startq1")
        else
            ib:SetTitle("You Solved the Quest?")
            ib:AddMsg("Have you done the task?\n\nShow me the helmet and you can join our guild.")
            ib:AddLink("Show Cashin the Helm", "checkq1")
        end
    else
        if guild_stat == game.GUILD_OLD then
            ib:AddLink("Rejoin the Mercenary Guild", "askjoing1")
            ib:SetTitle("Welcome Back")
        else
            ib:SetTitle("Hello Guildmember")
        end
        ib:AddMsg("Good to see you back. How are you?\n\nIt's nice that you have joined our ^troops^.")
    end
    pl:Interface(1, ib:Build())
end

-- quest body (added to player quest obj for quest list)
local function quest_icons1()
    ib:AddIcon("Mercenary Guild Membership", "guild_merc", "") 
end

local function quest_body1()
    ib:SetMsg("Before you can join the guild I have a small task for you.\n")
    ib:AddMsg("\nWe have had a problem with giant ants for some time now. See this hole to the old guild cellar beside " ..
              "me? One of those silly ants down there has stolen my old helmet!\n")
    ib:AddMsg("\nEnter the hole and kill the ants! Don't worry, they are weak.\n")
    ib:AddMsg("\nBring back the helmet and you can join the guild.\n")
    ib:AddMsg("\nThere are four rooms in the cellar. I think the ant scout leader in the fourth room has my helmet.\n")
    ib:AddMsg("\n|Hints|\n")
    ib:AddMsg("\nHm, you don't look too experienced with your weapon... Well, here is a tip from an old wardog.\n")
    ib:AddMsg("\nWhen you get down there, try fighting a few ants in the first three rooms before going for the scout " ..
              "leader. They won't attack you until you attack them.\n")
    ib:AddMsg("\nWhen you're ready, go for the leader. He is a lot more aggressive and will chase you on sight. Don't " ..
              "show him any fear!\n")
    ib:AddMsg("\nIf you run away like a coward he'll have time to recover from his wounds, so it is important that you " ..
              "keep on the offensive. Don't forget what Fanrir told you about eating food to recover health. You can " ..
	      "even do that during a fight!")
    ib:SetDesc("You can join the mercenary guild when you recover the helmet for Cashin.", 0, 0, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ1()
    if q_mgr_1:GetStatus() ~= game.QSTAT_NO then
        topicDefault()
    else
    ib:SetTitle(q_mgr_1.name)
    quest_body1()
    quest_icons1()
    ib:SetAccept(nil, "acceptq1") 
    ib:SetDecline(nil, "hi") 
    pl:Interface(1, ib:Build())
    end
end

-- accepted: start the quest
local function topAcceptQ1()
    if q_mgr_1:GetStatus() ~= game.QSTAT_NO then
        topicDefault()
        return
    end
    quest_body1()
    quest_icons1()
    if q_mgr_1:RegisterQuest(game.QUEST_ITEM, ib) then
        q_mgr_1:AddQuestItem(1, "quest_object", "helm_leather.101", q_iname1)
        pl:Sound(0, 0, 2, 0)
        pl:Write("You take the quest '".. q_mgr_1.name .."'.", game.COLOR_NAVY)
    end
    pl:Interface(-1, ib:Build())
end
-- try to finish: check the quest
local function topCheckQ1()
    if q_mgr_1:GetStatus() == game.QSTAT_NO then
        topicDefault()
        return
    end
    ib:SetTitle("Join the Mercenary Guild")
    -- ib:SetMsg("The quest status is: ".. q_mgr_1:GetStatus() .."\n\n")
    if q_mgr_1:GetStatus() ~= game.QSTAT_SOLVED then
        ib:AddMsg("Where is my helmet???\n\nCome back if you have found it!\n")
        ib:AddQuestChecklist(q_mgr_1)
        ib:SetButton("Back", "hi") 
    else
        ib:AddMsg("Very well done! You found the helmet.\n")
        ib:AddMsg("\nSince it is my old one, you can keep it.\n")
        ib:AddMsg("It will protect you well, as it did me.\n")
        ib:SetDesc("Join the Mercenary Guild and take the Helmet.", 0, 0, 0, 0)
        quest_icons1()
        ib:AddQuestChecklist(q_mgr_1)
        ib:SetAccept(nil, "finishq1") 
        ib:SetDecline(nil, "hi") 
    end
    pl:Interface(1, ib:Build())
end
-- done: finish quest and give reward
local function topFinishQ1()
    if q_mgr_1:GetStatus() ~= game.QSTAT_SOLVED then
        topicDefault()
        return
    else
        q_mgr_1:RemoveQuestItems()
        q_mgr_1:Finish()
        pl:Sound(0, 0, 2, 0)
        local q_i1 = pl:CreateObjectInside("helm_leather", 1,1) 
        q_i1.name = q_iname1
        pl:Write("You got ".. q_iname1..".", game.COLOR_WHITE) 
        pl:JoinGuild(guild_tag, game.SKILLGROUP_PHYSIQUE, 100, game.SKILLGROUP_AGILITY, 100, game.SKILLGROUP_WISDOM, 100)
        setGuild()
        ib:SetTitle("Welcome, Mercenary!")
        ib:SetMsg("Very well done!\n")
        ib:AddMsg("\nYou are now a member of the Mercenary Guild!")
        ib:SetButton("Ok", "hi") 
        pl:Interface(1, ib:Build())
    end
end

local function topAskjoinG1()
    if guild_stat ~= game.GUILD_OLD then
    topicDefault()
    return
    end
    ib:SetTitle("Rejoin our Guild?")
    ib:SetMsg("You really want rejoin our guild?\n")
    ib:AddMsg("\nWe would be proud to have you back in our ranks!")
    ib:SetAccept(nil, "rejoing1") 
    ib:SetDecline(nil, "hi") 
    pl:Interface(1, ib:Build())
end

local function topRejoinG1()
    if guild_stat ~= game.GUILD_OLD then
    topicDefault()
    return
    end
    pl:Sound(0, 0, 2, 0)
    pl:JoinGuild(guild_tag)
    setGuild()
    ib:SetTitle("Welcome Back!")
    ib:SetMsg("\nWelcome back to our guild!")
    ib:SetButton("Ok", "hi") 
    pl:Interface(1, ib:Build())
end

local function topTroops()
    ib:SetTitle("The Mercenary Troops")
    ib:SetMsg("\nWe, as part of the the Thraal army corps, are invading the abandoned areas after the defeat of Moroch.\n")
    ib:AddMsg("\nWell, the chronomancers ensured us after they created the portal that we would still be in the galactic main sphere.\n")
    ib:AddMsg("\nHowever, it seems to me that these lands have many wormholes to other places...\n")
    ib:AddMsg("\nPerhaps the long time under Morochs influence has weakened the borders between the planes. ")
    ib:AddMsg("You should ask ^Jahrlen^ about it.")
    ib:AddMsg("\n\nYou should also talk to our senior members, like ^Chereth^ downstairs; ")
    ib:AddMsg("they can provide valuable assistance to you.\n")
    ib:SetButton("Ok", "hi") 
    pl:Interface(1, ib:Build())
end

local function topJahrlen()
    ib:SetTitle("Jahrlen the Chronomancer")
    ib:SetMsg("\nJahrlen is our guild mage.\n")
    ib:AddMsg("\nWell, normally we don't have a guild mage, since we are mercenaries.")
    ib:AddMsg("\nBut we are at war here and he was assigned to us, so we treat him as one of our own.\n")
    ib:AddMsg("\nIn fact, he is a high level chronomancer and we are honored to have him help us.\n")
    ib:AddMsg("\nHe is in our guild rooms, or maybe in one of our branch guildhalls.\n")
    ib:AddMsg("\nTalk to him when you meet him! He often has tasks and quests for newbies.\n")
    ib:SetButton("Ok", "hi") 
    pl:Interface(1, ib:Build())
end

local function topChereth()
    ib:SetTitle("Supply Chief Chereth")
    ib:SetMsg("\nFormer Achery Commander Chereth is our Supply Chief.\n")
    ib:AddMsg("\nAlthough she was seriously wounded in battle, she is still a great expert in archery.\n")
    ib:SetButton("Ok", "hi") 
    pl:Interface(1, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)
if q_mgr_1:GetStatus() < game.QSTAT_DONE then
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
tl:AddTopics("chereth", topChereth)
tl:CheckMessage(event)
