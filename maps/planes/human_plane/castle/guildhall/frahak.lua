-- Ogre Chief Frah'aks Letter Quest using template for a "item quest" script
require("topic_list")
require("quest_manager")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)
local skill = game:GetSkillNr('find traps')
-- quest names must be unique
local q_mgr_1   = QuestManager(pl,"Ogre Chief Frah'aks Letter")
local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
    if q_mgr_1:GetStatus() < game.QSTAT_DONE and pl:FindSkill(skill) == nil then
        ib:AddMsg("The quest status is: ".. q_mgr_1:GetStatus() .."\n\n")
        if q_mgr_1:GetStatus() == game.QSTAT_NO then
            ib:SetTitle("Ogre Chief Frah'ak")
            ib:AddMsg("\nYo shut up.\nYo grack zhal hihzuk alshzu...\nMe mighty ogre chief.\nMe ^warrior^ ,will destroy yo. They come.\nGuard and ^kobolds^ will die then.")          
        else
            ib:SetTitle("Ogre Chief Frah'aks Letter Quest solved?")
            ib:AddMsg("You have the letter?")
            ib:AddLink("Finish Ogre Chief Frah'aks Letter Quest", "checkq1")
        end
    else
        ib:SetTitle("Yo Want More Teaching")
        ib:AddMsg("\nAshahk! Yo want me teaching yo more ^find traps^?\nWill teach for money.\n")
        ib:AddLink("Buy More Find Trap Skill", "find traps")
    end
    pl:Interface(1, ib:Build())
end

-- quest body (added to player quest obj for quest list)
local function quest_icons1()
    ib:AddIcon("Find Traps Skill", "skill.101", " ") 
end

local function quest_body1()
    ib:SetMsg("Get Frah'aks Letter.")
    ib:SetDesc("Bring Frah'ak his note from kobolds", 0, 0, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ1()
    if q_mgr_1:GetStatus() ~= game.QSTAT_NO then
        topicDefault()
        return
    end		
    ib:SetTitle(q_mgr_1.name)
    quest_body1()
    quest_icons1()
    ib:SetAccept(nil, "acceptq1") 
    ib:SetDecline(nil, "hi") 
    pl:Interface(1, ib:Build())
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
        q_mgr_1:AddQuestItem(1, "quest_object", "letter.101", "Frah'aks Letter")
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
    ib:SetTitle("Ogre Chief Frah'aks Letter Quest")
    --ib:SetMsg("The quest status is: ".. q_mgr_1:GetStatus() .."\n\n")
    if q_mgr_1:GetStatus() ~= game.QSTAT_SOLVED then
        ib:AddMsg("\nNah, bring Frah'ak note from ^kobolds^ first!\n")
        ib:AddQuestChecklist(q_mgr_1)
        ib:SetButton("Back", "hi") 
    else
        ib:AddMsg("\nAshahk! Yo bring me note!\nKobold chief bad time now, ha?\nNow me will teach you!\n")
        ib:SetDesc("here it is...", 0, 0, 0, 0)
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
    end
    q_mgr_1:RemoveQuestItems()
    q_mgr_1:Finish()
    pl:Sound(0, 0, 2, 0)
    pl:AcquireSkill(skill, game.LEARN)
    
    ib:SetTitle("Quest Completed")
    ib:SetMsg("Frah'ak teach an ancient skill.")
    ib:SetButton("Ok", "hi") 
    pl:Interface(1, ib:Build())
end

local function topFindtraps()
    local sobj = pl:GetSkill(game.TYPE_SKILL, skill)
    local slevel = sobj.level + 1
    local eobj = pl:GetSkill(game.TYPE_EXPERIENCE, game.EXP_AGILITY)
    if  eobj ~= nil and eobj.level >= slevel then
        ib:SetTitle("Find Traps Skill Cost")
        ib:SetMsg("\nYou have " .. pl:ShowCost(pl:GetMoney()) .. "\n ")
        ib:AddMsg("\nFind traps lvl "..slevel.." will cost you \n" .. pl:ShowCost( slevel * slevel * (50+slevel) * 3).." .")
        ib:AddLink("Pay For More Find Trap Skill", "teach traps")
        ib:SetButton("Ok", "hi") 
        pl:Interface(1, ib:Build())
    else
        ib:SetTitle("Find Traps Skill Cost")
        ib:SetMsg("Ho, yo agility to low to teach!!" )
        ib:SetButton("Ok", "hi") 
        pl:Interface(1, ib:Build())
    end
end

local function topTeachtraps()
    local sobj = pl:GetSkill(game.TYPE_SKILL, skill)
    local slevel = sobj.level + 1
    local eobj = pl:GetSkill(game.TYPE_EXPERIENCE, game.EXP_AGILITY)
    ib:SetTitle("Teach Find	Traps")
    ib:SetMsg("\nYou have " .. pl:ShowCost(pl:GetMoney()) .. "\n ")
    if  eobj == nil or eobj.level < slevel then
        ib:AddMsg("\nHo, yo agility too low to teach!!\n" )
    else
        local amount = slevel * slevel * (50+slevel) * 3
        if pl:PayAmount(amount) == 1 then
            ib:AddMsg("\nYou pay Frah'ak" .. pl:ShowCost(amount) .. "\n")
            ib:AddMsg("Here we go!\n")
            ib:AddMsg("Frah'ak teach some ancient skill.\n") 
            pl:SetSkill(game.TYPE_SKILL, skill, slevel, 0)
        else
            ib:AddMsg("\nHo, yo not enough money to pay Frah'ak!!" )
        end
    end
    ib:SetButton("Ok", "hi") 
    pl:Interface(1, ib:Build())
end

local function topWarrior()
    ib:SetTitle("Warrior")
    ib:AddMsg("\nMe big chief. Me ogre destroy you.\nStomp on. Dragon kakka." )
    ib:SetButton("Back", "Hi") 
    pl:Interface(1, ib:Build())
end

local function topKobolds()
    ib:SetTitle("Kolbolds")
    ib:AddMsg("\nKobolds traitors!\nGive gold for note, kobolds don't bring note to ogres.\nMe tell you: Kill kobold chief!\nMe will teach you find traps skill!\nShow me note i will teach you.\nKobolds in hole next room. Secret entry in wall." )
    ib:AddLink("Start the Ogre Chief Frah'aks Letter Quest", "startq1")
    ib:SetButton("Back", "Hi")
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

tl:AddTopics("find traps", topFindtraps)
tl:AddTopics("teach traps", topTeachtraps)
tl:AddTopics("warrior", topWarrior)
tl:AddTopics("kobolds", topKobolds)
tl:CheckMessage(event)
