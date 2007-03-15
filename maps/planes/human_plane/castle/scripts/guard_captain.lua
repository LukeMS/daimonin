  -- Guard Captain's Quests 
require("topic_list")
require("quest_check")
require("interface_builder")

local pl = event.activator
local me = event.me
local msg = string.lower(event.message)

local q_name_1 = "Kill The Ogres"
local q_step_1 = 0
local q_level_1 = 1
local q_skill_1 = game.ITEM_SKILL_NO
local q_obj_1 = pl:GetQuest(q_name_1)
local q_stat_1 = Q_Status(pl, q_obj_1, q_step_1, q_level_1, q_skill_1)

local q_name_2 = "Giant-Stolen Weapons"
local q_step_2 = 0
local q_level_2 = 1
local q_skill_2 = game.ITEM_SKILL_NO
local q_obj_2 = pl:GetQuest(q_name_2)
local q_stat_2 = Q_Status(pl, q_obj_2, q_step_2, q_level_2, q_skill_2, "Kill The Ogres")

local q_name_3 = "Felbe's Letter"
local q_step_3 = 0
local q_level_3 = 1
local q_skill_3 = game.ITEM_SKILL_NO
local q_obj_3 = pl:GetQuest(q_name_3)
local q_stat_3 = Q_Status(pl, q_obj_3, q_step_3, q_level_3, q_skill_3)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local function topicDefault()
  ib:SetTitle("Things Need Doing")
  ib:AddMsg("The ogres and giants have been attacking relentlessly lately. All of our free soldiers are out fighting them off or are down in Stoneglow helping THEM out. As a result, we have some things that need doing. I hope you're up to it.\n\n")
  if q_stat_1 < game.QSTAT_DONE then
    if q_stat_1 == game.QSTAT_NO then
      ib:AddLink("Assignment 1: Kill Some Ogres", "startq1")
      ib:AddMsg("\nBefore we send you on any large quests, we need to see your worth! The ogres are numerous and are just calling out for a good whipping. Kill some and then you can try out a level 2 quest.\n")
    else
      ib:AddLink("Finish Assignment 1", "checkq1")
      ib:AddMsg("\nIf you have indeed finished your first assignment, I'll let you in on a larger quest.\n")
    end
  end
  if q_stat_2 < game.QSTAT_DONE then
    if q_stat_2 == game.QSTAT_NO then
      ib:AddLink("Assignment 2: Recover Stolen Weapons", "startq2")
      ib:AddMsg("\nYour works have shown your worth--for now that is. Get on to your second assignment!\n")
    else
      ib:AddLink("Finish Assignment 2", "checkq2")
      ib:AddMsg("\nYou've recovered the weapons already? Lets see then!\n")
    end
  end

  if q_stat_3 < game.QSTAT_DONE then
    if q_stat_3 == game.QSTAT_NO then
      ib:AddLink("Special Assignment: Felbe's Letter", "startq3")
      ib:AddMsg("\nAlso, if your time isn't taken up by the main assignments, we have one that's been on the back burner for a while.")
    else
      ib:AddLink("I've got Felbe's letter", "checkq3")
    end
  end

  if q_stat_1 >= game.QSTAT_DONE and q_stat_2 >= game.QSTAT_DONE and q_stat_3 >= game.QSTAT_DONE then
    pl:Write("No talking--get out there and fight!", game.COLOR_NAVY)
    pl:Interface(-1, "")
    return
  end
  pl:Interface(1, ib:Build())
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
  if q_stat_1 ~= game.QSTAT_NO then
    topicDefault()
   return
  end
    ib:SetTitle(q_name_1)
    quest_body1()
    quest_icons1()
    ib:SetAccept(nil, "acceptq1")
    ib:SetDecline(nil, "hi")
    pl:Interface(1, ib:Build())
end

-- accepted: start the quest
local function topAcceptQ1()
  if q_stat_1 == game.QSTAT_NO then
    quest_body1()
    quest_icons1()
    q_obj_1 = pl:AddQuest(q_name_1, game.QUEST_KILL, q_step_1, q_step_1, q_level_1, q_skill_1, ib:Build())
    if q_obj_1 ~= null then
      q_obj_1:AddQuestTarget(0, 15, "ogre", "Ogre", "Ogre Leader", "Ogre Elite" )
      q_stat_1 = Q_Status(pl, q_obj_1, q_step_1, q_level_1, q_skill_1)
      pl:Sound(0, 0, 2, 0)
      pl:Write("You take the quest '"..q_name_1.."'.", game.COLOR_NAVY)
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
   return
  end
    ib:SetTitle("Killed 15 Ogres?")
    if q_stat_1 ~= game.QSTAT_SOLVED then
      ib:AddMsg("You still haven't killed at least 15 ogres. Get a move on!\n")
      Q_List (q_obj_1, ib)
      ib:SetButton("Back", "hi")
    else
      ib:AddMsg("Excellent work. That should keep the ogres off our backs for a while.\n")
      ib:SetDesc("Take your reward and get on to your second assignment!", 1, 2, 0, 0)
      quest_icons1()
      Q_List(q_obj_1, ib)
      ib:SetAccept(nil, "finishq1")
      ib:SetDecline(nil, "hi")
    end
    pl:Interface(1, ib:Build())
end

-- done: finish quest and give reward
local function topFinishQ1()
  if q_stat_1 ~= game.QSTAT_SOLVED then
    topicDefault()
   return
  end
  q_obj_1:RemoveQuestItem()
  q_obj_1:SetQuestStatus(-1)
  q_stat_1 = game.QSTAT_DONE
  pl:Sound(0, 0, 2, 0)
  pl:CreateObjectInsideEx("shield_kite", 1,1)
  pl:AddMoneyEx(1,2,0,0)
  ib:SetTitle("Assignment 1: Completed")
  ib:SetMsg("There you go.")
  ib:SetButton("Ok", "hi")
  pl:Interface(1, ib:Build())
end

-- Quest 2 parts
-- quest body (added to player quest obj for quest list)
local function quest_icons2()
  ib:AddIcon("Eye Shield", "shield_eye.101", "Level 9 req ...ISCP 10,5,10,5 ")
end
local function quest_body2()
  ib:SetMsg("The giants broke through the walls a few months ago and took quite a bit of our weapons from our weapons stash. We need those back.")
  ib:SetDesc("We are missing °5 Guard Spears° and °10 Stonehaven Swords°.\nThe Giants should be wearing them (if they are the smart kind, that is). Kill em and bring the weapons back.", 1, 2, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ2()
  if q_stat_2 ~= game.QSTAT_NO then
    topicDefault()
   return
  end
    ib:SetTitle(q_name_2)
    quest_body2()
    quest_icons2()
    ib:SetAccept(nil, "acceptq2")
    ib:SetDecline(nil, "hi")
    pl:Interface(1, ib:Build())
end

-- accepted: start the quest
local function topAcceptQ2()
  if q_stat_2 == game.QSTAT_NO then
    quest_body2()
    quest_icons2()
    q_obj_2 = pl:AddQuest(q_name_2, game.QUEST_KILLITEM, q_step_2, q_step_2, q_level_2, q_skill_2, ib:Build())
    if q_obj_2 ~= null then
      local tobj = q_obj_2:AddQuestTarget(2, 10, "giant_hill", "Hill Giant", "Elite Hill Giant")
      tobj:AddQuestItem(10, "quest_object", "bastardsword.101", "Stonehaven Sword")
      tobj = q_obj_2:AddQuestTarget(3, 5, "giant_stone", "Stone Giant", "Elite Stone Giant")
      tobj:AddQuestItem(5, "quest_object", "partisan.101", "Guard Spear")
      q_stat_2 = Q_Status(pl, q_obj_2, q_step_2, q_level_2, q_skill_2, "Kill The Ogres")
      pl:Sound(0, 0, 2, 0)
      pl:Write("You take the quest '"..q_name_2.."'.", game.COLOR_NAVY)
    end
    ib = InterfaceBuilder()
    ib:SetHeader(me, me.name)
  end
  topicDefault()
end

-- try to finish: check the quest
local function topCheckQ2()
  if q_stat_2 == game.QSTAT_NO then
    topicDefault()
   return
  end
  ib:SetTitle("Recovered The Weapons?")
  if q_stat_2 ~= game.QSTAT_SOLVED then
    ib:AddMsg("You already have the weapons? Ah, I see you are still a few short. Hurry up; your weapon in the hand of the enemy is another against you!\n")
    Q_List(q_obj_2, ib)
    ib:SetButton("Back", "hi")
  else
    ib:AddMsg("Great work. Those weapons will be sent back to the stash.\n")
    ib:SetDesc("Your reward is waiting, I expect you'll want it.", 1, 2, 0, 0)
    quest_icons2()
    Q_List(q_obj_2, ib)
    ib:SetAccept(nil, "finishq2")
    ib:SetDecline(nil, "hi")
  end
    pl:Interface(1, ib:Build())
end

-- done: finish quest and give reward
local function topFinishQ2()
  if q_stat_2 ~= game.QSTAT_SOLVED then
    topicDefault()
   return
  end
  Q_Remove(q_obj_2)
  q_obj_2:SetQuestStatus(-1)
  q_stat_2 = game.QSTAT_DONE
  pl:Sound(0, 0, 2, 0)
  pl:CreateObjectInsideEx("shield_eye", 1,1)
  pl:AddMoneyEx(1,2,0,0)
  ib:SetTitle("Assignment 2: Finished")
  ib:SetMsg("Perfect, you've got your reward and I've got the weapons back.")
  ib:SetButton("Ok", "hi")
  pl:Interface(1, ib:Build())
end

-- Quest 3 parts
-- quest body (added to player quest obj for quest list)
local function quest_icons3()
  ib:AddIcon("quest-reward-name", "shield_high.101", "i am the stats/description line")
end
local function quest_body3()
  ib:SetMsg("[WHY] Deliver the item for this multi quest example.")
  ib:SetDesc("[WHAT] Bring me a °Item Test Helm Multi 3°.\nOpen the chest #3.", 1, 2, 0, 0)
end

-- start: accept or decline the quest
local function topStartQ3()
  if q_stat_3 ~= game.QSTAT_NO then
    topicDefault()
   return
  end
  ib:SetTitle(q_name_1)
  quest_body3()
  quest_icons3()
  ib:SetAccept(nil, "acceptq3")
  ib:SetDecline(nil, "hi")
  pl:Interface(1, ib:Build())
end
-- accepted: start the quest
local function topAcceptQ3()
  if q_stat_3 == game.QSTAT_NO then
    quest_body3()
    quest_icons3()
    q_obj_3 = pl:AddQuest(q_name_3, game.QUEST_ITEM, q_step_3, q_step_3, q_level_3, q_skill_3, ib:Build())
    if q_obj_3 ~= null then
      q_obj_3:AddQuestItem(1, "quest_object", "helm_leather.101", "Item Test Helm Multi 3")
      q_stat_3 = Q_Status(pl, q_obj_3, q_step_3, q_level_3, q_skill_3)
      pl:Sound(0, 0, 2, 0)
      pl:Write("You take the quest '"..q_name_3.."'.", game.COLOR_NAVY)
    end
    ib = InterfaceBuilder()
    ib:SetHeader(me, me.name)
  end
  topicDefault()
end

-- try to finish: check the quest
local function topCheckQ3()
  if q_stat_3 == game.QSTAT_NO then
    topicDefault()
   return
  end
  ib:SetTitle("FINAL CHECK: Item Test Quest Multi 3")
  ib:SetMsg("[DEVMSG] The quest status is: ".. q_stat_3 .."\n\n")
  if q_stat_3 ~= game.QSTAT_SOLVED then
    ib:AddMsg("[not-done-text] Come back if you have it!\n")
    Q_List(q_obj_3, ib)
    ib:SetButton("Back", "hi")
  else
    ib:AddMsg("[final-text] Very well done! You found the helm.\n")
    ib:SetDesc("here it is...", 1, 2, 0, 0)
    quest_icons3()
    Q_List(q_obj_3, ib)
    ib:SetAccept(nil, "finishq3")
    ib:SetDecline(nil, "hi")
  end
  pl:Interface(1, ib:Build())
end

-- done: finish quest and give reward
local function topFinishQ3()
  if q_stat_3 ~= game.QSTAT_SOLVED then
    topicDefault()
    return
  end
  q_obj_3:RemoveQuestItem()
  q_obj_3:SetQuestStatus(-1)
  q_stat_3 = game.QSTAT_DONE
  pl:Sound(0, 0, 2, 0)
  pl:CreateObjectInsideEx("shield", 1,1)
  pl:AddMoneyEx(1,2,0,0)
  ib:SetTitle("QUEST END: Item Test Quest Multi 3")
  ib:SetMsg("Very well done! Here is your reward!")
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
if q_stat_2 < game.QSTAT_DONE then
  tl:AddTopics("startq2", topStartQ2)
  tl:AddTopics("acceptq2", topAcceptQ2)
  tl:AddTopics("checkq2", topCheckQ2)
  tl:AddTopics("finishq2", topFinishQ2)
end
if q_stat_3 < game.QSTAT_DONE then
  tl:AddTopics("startq3", topStartQ3)
  tl:AddTopics("acceptq3", topAcceptQ3)
  tl:AddTopics("checkq3", topCheckQ3)
  tl:AddTopics("finishq3", topFinishQ3)
end
tl:CheckMessage(event)