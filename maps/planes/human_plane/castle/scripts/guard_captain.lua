  -- Guard Captain's Quests
require("topic_list")
require("quest_manager")
require("interface_builder")

local pl = event.activator
local me = event.me
local msg = string.lower(event.message)
local ds = DataStore("felbe_letter_quest", pl)
local ds_msg = ds:Get("felbe_msg")

local q_mgr_1   = QuestManager(pl,"Kill The Ogres")

local q_mgr_2   = QuestManager(pl,"Giant-Stolen Weapons")
      q_mgr_2:AddRequiredQuest(q_mgr_1)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
    ib:SetTitle("Things Need Doing")
    ib:AddMsg("The ogres and giants have been attacking relentlessly lately. All of our free soldiers are out fighting them off or are down in Stoneglow helping THEM out. As a result, we have some things that need doing. I hope you're up to it.\n\n")
    if pl:CheckInventory(1, nil, "Felbe's Letter", nil, -1) == nil then
        if ds_msg == "felbe_letter_really_done" or ds_msg == "felbe_letter_done" then
        else
            ib:AddMsg("My head scout is stationed in Nyrrwood forest, and I have complete confidence in his abilities. It still worries me that I haven't recieved contact from him in the last week...")
        end
    end
    if q_mgr_1:GetStatus() < game.QSTAT_DONE and ds_msg == "felbe_letter_really_done" then
        if ds_msg == "felbe_letter_really_done" and q_mgr_1:GetStatus() == game.QSTAT_NO then
            ib:AddLink("Assignment 1: Kill Some Ogres", "startq1")
            ib:AddMsg("\nBefore we send you on any large quests, we need to see your worth! The ogres are numerous and are just calling out for a good whipping. Kill some and then you can try out a level 2 quest.\n")
        elseif ds_msg == "felbe_letter_really_done" then
            ib:AddLink("Finish Assignment 1", "checkq1")
            ib:AddMsg("\nIf you have indeed finished your first assignment, I'll let you in on a larger quest.\n")
        end
    elseif q_mgr_2:GetStatus() < game.QSTAT_DONE and ds_msg == "felbe_letter_really_done" then
        if q_mgr_2:GetStatus() == game.QSTAT_NO and ds_msg == "felbe_letter_really_done" then
          ib:AddLink("Assignment 2: Recover Stolen Weapons", "startq2")
          ib:AddMsg("\nYour works have shown your worth--for now that is. Get on to your second assignment!\n")
        else
          ib:AddLink("Finish Assignment 2", "checkq2")
          ib:AddMsg("\nYou've recovered the weapons already? Lets see then!\n")
        end
  end

    if pl:CheckInventory(1, nil, "Felbe's Letter", nil, -1) then
      ib:AddLink("Yes, he's in Nyrrwood Forest", "felbe")
      ib:AddMsg("\nYou say you have a letter from Felbe? Where has he been? We haven't heard a report from him in a few weeks...")
    elseif pl:CheckInventory(1, nil, "Guard Captain Aldus's Letter", nil, -1) then
      me:SayTo(pl, "You've already got my letter -- now go and bring it to Felbe!")
      pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
      return
    elseif ds_msg == "felbe_letter_done" then
        -- done quest and give reward
        ib:SetTitle("Good job")
        ib:AddMsg("Well, that'll do for now. I think we can round up a troop or two to fill in for you now. In fact, I think we could use you in some more useful assignments...")
        -- pl:CreateObjectInside("item-arch-name", 1,1) if you want to create
        -- item in player's inv or use local reward = pl:CreateObjectInside("item-arch-name", 1,1)
        -- and then use reward.name = "set-name" or reward.slaying = "slaying for locked gates, doors etc, maybe for the locked guild halls doors?"
        ds:Set("felbe_msg", "felbe_letter_really_done")
    end

  if q_mgr_1:GetStatus() == game.QSTAT_DONE and q_mgr_2:GetStatus() == game.QSTAT_DONE and ds_msg == "felbe_letter_really_done" then
    pl:Write("No talking--get out there and fight!", game.COLOR_NAVY)
    pl:Interface(game.GUI_NPC_MODE_NO)
    return
  end
  pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

-- Quest 1 parts
-- quest body (added to player quest obj for quest list)
local function quest_icons1()
  ib:AddIcon("Kite Shield", "shield_kite.101", "Level 3 req ...ISCP 10,5,5,10")
end
local function quest_body1()
  ib:SetMsg("Those ogres don't slay themselves!")
  ib:SetDesc("Kill 15 ogres so I can see your worth.", 1, 2, 0, 0)
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
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

-- accepted: start the quest
local function topAcceptQ1()
    if q_mgr_1:GetStatus() ~= game.QSTAT_NO then
    topicDefault()
    return
  end
  quest_body1()
    quest_icons1()
    if q_mgr_1:RegisterQuest(game.QUEST_KILL, ib) then
    q_mgr_1:AddQuestTarget(0, 15, "ogre", "Ogre", "Ogre Leader", "Ogre Elite" )
        pl:Sound(0, 0, 2, 0)
        pl:Write("You take the quest '".. q_mgr_1.name .."'.", game.COLOR_NAVY)
    end
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
  end

-- try to finish: check the quest
local function topCheckQ1()
  if q_mgr_1:GetStatus() == game.QSTAT_NO then
    topicDefault()
  return
  end
    ib:SetTitle("Killed 15 Ogres?")
    if q_mgr_1:GetStatus() ~= game.QSTAT_SOLVED then
      ib:AddMsg("You still haven't killed at least 15 ogres. Get a move on!\n")
      ib:AddQuestChecklist(q_mgr_1)
      ib:SetButton("Back", "hi")
    else
      ib:AddMsg("Excellent work. That should keep the ogres off our backs for a while.\n")
      ib:SetDesc("Take your reward and get on to your second assignment!", 1, 2, 0, 0)
      quest_icons1()
      ib:AddQuestChecklist(q_mgr_1)
      ib:SetAccept(nil, "finishq1")
      ib:SetDecline(nil, "hi")
    end
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

-- done: finish quest and give reward
local function topFinishQ1()
  if q_mgr_1:GetStatus() ~= game.QSTAT_SOLVED then
    topicDefault()
  return
  end
  q_mgr_1:Finish()
  pl:Sound(0, 0, 2, 0)
  pl:CreateObjectInsideEx("shield_kite", 1,1)
  pl:AddMoneyEx(1,2,0,0)
  ib:SetTitle("Assignment 1: Completed")
  ib:SetMsg("There you go.")
  ib:SetButton("Ok", "hi")
  pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

-- Quest 2 parts
-- quest body (added to player quest obj for quest list)
local function quest_icons2()
  ib:AddIcon("Eye Shield", "shield_eye.101", "Level 9 req ...ISCP 10,5,10,5 ")
end
local function quest_body2()
  ib:SetMsg("The giants broke through the walls a few months ago and took quite a bit of our weapons from our weapons stash. We need those back.")
  ib:SetDesc("We are missing °10 Guard Spears° and °5 Stonehaven Swords°.\nThe Giants should be wearing them (if they are the smart kind, that is). Kill em and bring the weapons back.", 1, 2, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ2()
  if q_mgr_2:GetStatus() ~= game.QSTAT_NO then
    topicDefault()
  return
  end
    ib:SetTitle(q_mgr_2.name)
    quest_body2()
    quest_icons2()
    ib:SetAccept(nil, "acceptq2")
    ib:SetDecline(nil, "hi")
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

-- accepted: start the quest
local function topAcceptQ2()
  if q_mgr_2:GetStatus() == game.QSTAT_NO then
    quest_body2()
    quest_icons2()
    if q_mgr_2:RegisterQuest(game.QUEST_KILLITEM, ib) then
       q_mgr_2:AddQuestTarget(2, 10, "giant_hill", "Hill Giant", "Elite Hill Giant"):
             AddQuestItem(10, "quest_object", "bastardsword.101", "Stonehaven Sword")
       q_mgr_2:AddQuestTarget(3, 5, "giant_stone", "Stone Giant", "Elite Stone Giant"):
               AddQuestItem(3, "quest_object", "partisan.101", "Guard Spear")
       pl:Sound(0, 0, 2, 0)
       pl:Write("You take the quest '"..q_mgr_2.name.."'.", game.COLOR_NAVY)
    end
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
  end
  topicDefault()
end

-- try to finish: check the quest
local function topCheckQ2()
  if q_mgr_2:GetStatus() == game.QSTAT_NO then
    topicDefault()
  return
  end
  ib:SetTitle("Recovered The Weapons?")
  if q_mgr_2:GetStatus() ~= game.QSTAT_SOLVED then
    ib:AddMsg("You already have the weapons? Ah, I see you are still a few short. Hurry up; your weapon in the hand of the enemy is another against you!\n")
    ib:AddQuestChecklist(q_mgr_2)
    ib:SetButton("Back", "hi")
  else
    ib:AddMsg("Great work. Those weapons will be sent back to the stash.\n")
    ib:SetDesc("Your reward is waiting, I expect you'll want it.", 1, 2, 0, 0)
    quest_icons2()
    ib:AddQuestChecklist(q_mgr_2)
    ib:SetAccept(nil, "finishq2")
    ib:SetDecline(nil, "hi")
  end
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

-- done: finish quest and give reward
local function topFinishQ2()
  if q_mgr_2:GetStatus() ~= game.QSTAT_SOLVED then
    topicDefault()
  return
  end
  q_mgr_2:RemoveQuestItems()
  q_mgr_2:Finish()
  pl:Sound(0, 0, 2, 0)
  pl:CreateObjectInsideEx("shield_eye", 1,1)
  pl:AddMoneyEx(1,2,0,0)
  ib:SetTitle("Assignment 2: Finished")
  ib:SetMsg("Perfect, you've got your reward and I've got the weapons back.")
  ib:SetButton("Ok", "hi")
  pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

-- Quest 3 parts
local function topicFelbe()
    if ds_msg == "felbe_letter" then
        ib:SetTitle("Felbe's Letter")
        ib:AddMsg("You have a letter from Felbe? Indeed, you do! Did he say why he's not been reporting back to us? But lets talk about that later--the message, please.") -- Something like Ah so! You have found Felbe... And he got a message for me?!
        ib:AddLink("Give Felbe's Letter to Aldus", "letter1")
    end
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
    ib:SetButton("Back", "hi")
end

local function topicLetter1()
    -- This is topic where player gives Felbe's Letter to guard and receives a letter for Felbe.
    ib:SetTitle("Felbe's Letter")
    ib:SetMsg("So, you say he's been wondering why he has not recieved any contact from us? It sounds to me like we've got someone intercepting our messages. It appears that we need a skilled carrier. We are terribly short on troops at the moment, so it looks like you're our temporary carrier!")
    if pl:CheckInventory(1, nil, "Guard Captain Aldus's Letter", nil, -1) then
        ib:AddMsg("\n\nGive the letter to Felbe and report back.") -- something like Bring the letter to Felbe now!
    else
        local felbe_letter = pl:CheckInventory(1, nil, "Felbe's Letter", nil, -1)
        felbe_letter:Remove()
        local letter = pl:CreateObjectInside("letter", 1,1)
        letter.name = "Guard Captain Aldus's Letter"
        letter.message = ""
        letter.f_identified = 1
        letter.f_no_drop = 1
        ib:AddMsg("\n\nSo, give this letter to Felbe.")
    end
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
    ib:SetButton("Back", "felbe")
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
if q_mgr_2:GetStatus() < game.QSTAT_DONE then
  tl:AddTopics("startq2", topStartQ2)
  tl:AddTopics("acceptq2", topAcceptQ2)
  tl:AddTopics("checkq2", topCheckQ2)
  tl:AddTopics("finishq2", topFinishQ2)
end
if ds_msg == "felbe_letter_really_done" then
    -- add no topics because the quest has been solved.
else
    tl:AddTopics("letter1", topicLetter1)
    tl:AddTopics("felbe", topicFelbe)
end
tl:CheckMessage(event)
