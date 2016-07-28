-- Ogre Chief Frah'aks Letter Quest using template for a "item quest" script
require("topic_list")
require("quest_builder")
require("interface_builder")

local pl        = event.activator
local me        = event.me
local msg       = string.lower(event.message)
local qb        = QuestBuilder()
local skill     = game:GetSkillNr('find traps')

local function questGoal(questnr)
    qb:AddQuestItem(questnr, 1, "quest_object", "letter.101", "Frah'aks Letter")
    pl:Sound(0, 0, 2, 0)
    pl:Write("You take the quest '".. qb:GetName(questnr) .."'.", game.COLOR_NAVY)
end

local function questReward(questnr)
    pl:Sound(0, 0, 2, 0)
    pl:AcquireSkill(skill, game.LEARN)
end

-- quest names must be unique
qb:AddQuest("Ogre Chief Frah'aks Letter", game.QUEST_ITEM, nil, nil, nil,
            nil, 1, questGoal, questReward)

local questnr = qb:Build(pl)
local qstat   = qb:GetStatus(1)

local ib = InterfaceBuilder()

local function topicDefault()
    if qstat < game.QSTAT_DONE and pl:FindSkill(skill) == nil then
        if qstat == game.QSTAT_NO then
            ib:SetHeader("st_001", me)
            ib:SetTitle("Ogre Chief Frah'ak")
            ib:AddMsg("\nYo shut up.\nYo grack zhal hihzuk alshzu...\nMe mighty ogre chief.\nMe ^warrior^ ,will destroy yo. They come.\nGuard and ^kobolds^ will die then.")          
        else
            ib:SetHeader("st_001", me)
            ib:SetTitle("Ogre Chief Frah'aks Letter Quest solved?")
            ib:AddMsg("You have the letter?")
            ib:AddLink("Finish Ogre Chief Frah'aks Letter Quest", "checkq1")
        end
    else
        ib:SetHeader("st_005", me)
        ib:SetTitle("Yo Want More Teaching")
        ib:AddMsg("\nAshahk! Yo want me teaching yo more ^find traps^?\nWill teach for money.\n")
        ib:AddLink("Buy More Find Trap Skill", "find traps")
    end
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
    if qstat ~= game.QSTAT_NO then
        topicDefault()
        return
    end
    ib:SetHeader("st_003", me)
    ib:SetTitle(qb:GetName(questnr))
    quest_body1()
    quest_icons1()
    ib:SetAccept(nil, "acceptq1") 
    ib:SetDecline(nil, "hi") 
end

-- accepted: start the quest
local function topAcceptQ1()
    if qstat ~= game.QSTAT_NO then
        topicDefault()
        return
    end
    ib:SetHeader("st_003", me)
    quest_body1()
    quest_icons1()
    qb:RegisterQuest(questnr, me, ib)
end

-- try to finish: check the quest
local function topCheckQ1()
    if qstat == game.QSTAT_NO then
        topicDefault()
        return
    end
    ib:SetHeader("st_003", me)
    ib:SetTitle("Ogre Chief Frah'aks Letter Quest")
    --ib:SetMsg("The quest status is: ".. qstat .."\n\n")
    if qstat ~= game.QSTAT_SOLVED then
        ib:AddMsg("\nNah, bring Frah'ak note from ^kobolds^ first!\n")
        ib:SetButton("Back", "hi") 
    else
        ib:AddMsg("\nAshahk! Yo bring me note!\nKobold chief bad time now, ha?\nNow me will teach you!\n")
        ib:SetDesc("here it is...", 0, 0, 0, 0)
        quest_icons1()
        ib:SetAccept(nil, "finishq1") 
        ib:SetDecline(nil, "hi") 
    end
end

-- done: finish quest and give reward
local function topFinishQ1()
    if qstat ~= game.QSTAT_SOLVED then
        topicDefault()
        return
    end
    qb:Finish(1)
    
    ib:SetTitle("Quest Completed")
    ib:SetMsg("Frah'ak teaches you an ancient skill.")
    ib:SetButton("Ok", "hi") 
end

local function topFindtraps()
    local sobj = pl:GetSkill(game.TYPE_SKILL, skill)
    local slevel = sobj.level + 1
    local eobj = pl:GetSkill(game.TYPE_SKILLGROUP, game.SKILLGROUP_AGILITY)
    if  eobj ~= nil and eobj.level >= slevel then
        ib:SetHeader("st_005", me)
        ib:SetTitle("Find Traps Skill Cost")
        ib:SetMsg("\nYou have " .. pl:ShowCost(pl:GetMoney()) .. "\n ")
        ib:AddMsg("\nFind traps lvl "..slevel.." will cost you \n" .. pl:ShowCost( slevel * slevel * (50+slevel) * 3).." .")
        ib:AddLink("Pay For More Find Trap Skill", "teach traps")
        ib:SetButton("Ok", "hi") 
    else
        ib:SetHeader("st_005", me)
        ib:SetTitle("Find Traps Skill Cost")
        ib:SetMsg("Ho, yo agility to low to teach!!" )
        ib:SetButton("Ok", "hi") 
    end
end

local function topTeachtraps()
    local sobj = pl:GetSkill(game.TYPE_SKILL, skill)
    local slevel = sobj.level + 1
    local eobj = pl:GetSkill(game.TYPE_SKILLGROUP, game.SKILLGROUP_AGILITY)
    ib:SetHeader("st_005", me)
    ib:SetTitle("Teach Find	Traps")
    ib:SetMsg("\nYou have " .. pl:ShowCost(pl:GetMoney()) .. "\n ")
    if  eobj == nil or eobj.level < slevel then
        ib:AddMsg("\nHo, yo agility too low to teach!!\n" )
    else
        local amount = slevel * slevel * (50+slevel) * 3
        if pl:PayAmount(amount) == 1 then
            ib:AddMsg("\nYou pay Frah'ak " .. pl:ShowCost(amount) .. "\n")
            ib:AddMsg("Here we go!\n")
            ib:AddMsg("Frah'ak teach some ancient skill.\n") 
            pl:SetSkill(game.TYPE_SKILL, skill, slevel, 0)
        else
            ib:AddMsg("\nHo, yo not enough money to pay Frah'ak!!" )
        end
    end
    ib:SetButton("Ok", "hi") 
end

local function topWarrior()
    ib:SetHeader("st_002", me)
    ib:SetTitle("Warrior")
    ib:AddMsg("\nMe big chief. Me ogre destroy you.\nStomp on. Dragon kakka." )
    ib:SetButton("Back", "Hi") 
end

local function topKobolds()
    ib:SetHeader("st_002", me)
    ib:SetTitle("Kolbolds")
    ib:AddMsg("\nKobolds traitors!\nGive gold for note, kobolds don't bring note to ogres.\nMe tell you: Kill kobold chief!\nMe will teach you find traps skill!\nShow me note i will teach you.\nKobolds in hole next room. Secret entry in wall." )
    ib:AddLink("Start the Ogre Chief Frah'aks Letter Quest", "startq1")
    ib:SetButton("Back", "Hi")
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:SetDefault(topicDefault)

if qb:GetStatus(1) < game.QSTAT_DONE then
    tl:AddTopics("startq1", topStartQ1) 
    tl:AddTopics("acceptq1", topAcceptQ1) 
    tl:AddTopics("checkq1", topCheckQ1) 
    tl:AddTopics("finishq1", topFinishQ1) 
end

tl:AddTopics("find traps", topFindtraps)
tl:AddTopics("teach traps", topTeachtraps)
tl:AddTopics("warrior", topWarrior)
tl:AddTopics("kobolds", topKobolds)
ib:ShowSENTInce(game.GUI_NPC_MODE_NPC, tl:CheckMessage(event, true))