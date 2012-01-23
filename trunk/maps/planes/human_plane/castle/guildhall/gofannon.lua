--
-- Guildhall Smith. Copied from scripts/smith.lua and extended for 
-- a small quest. That is a bad idea, and should be fixed as soon as
-- we have a good way of extending generic scripts...
--

require("topic_list")
require("interface_builder")
require("quest_builder")

local pl = event.activator
local me = event.me
local qb = QuestBuilder()
local ib = InterfaceBuilder()

local function questGoal(questnr)
    qb:AddQuestTarget(questnr, 2, 3, "cat_black", "Wild Cat"):
       AddQuestItem(3, "quest_object", "skin.101", "Wild Cat Skin")
    pl:Sound(0, 0, 2, 0)
    pl:Write("You take the quest '"..qb:GetName(1).."'.", game.COLOR_NAVY)
end

-- quest names must be unique
qb:AddQuest("How to Skin a Cat", game.QUEST_KILLITEM, nil, nil, nil,
            nil, 1, questGoal, nil)

local questnr = qb:Build(pl)
local qstat = qb:GetStatus(1)

function repairCost(item)
    local cost = 5 + item:GetRepairCost()
    return cost
end

-- TODO: add in a sidenote about the cats bothering the chickens
local function quest1_body()   
    ib:SetHeader("st_003", me)
    ib:SetTitle(qb:GetName(1))
    ib:AddMsg("It is just a small thing.\n\n")
    ib:AddMsg("I have been awaiting a shipment of leather for some time, but it appears the trader has been delayed.\n\n")
    ib:AddMsg("I urgently need some feline skins for an item ordered by ^Chereth^.\n\n")
    ib:AddMsg("Obviously I can't get mountain leopard skins as ordered, but there are a few wild cats running around in the forest patch east of here. Could you please see if you can catch a few and return to me with at least three cat skins?\n\n")
    ib:SetDesc("Bring back 3 Wild Cat Skins to " .. me:GetName() .. ".")
end

function topicDefault(force)
    -- Autocomplete quest, if applicable
    if qstat == game.QSTAT_SOLVED and not force then
        topicQuestComplete()
        return
    end
    ib:SetHeader("st_001", me)
    ib:SetTitle("The Smithy")
    ib:SetMsg("Hello! I am " .. me:GetName() .. ".\n")
    ib:AddMsg("I can repair or identify your equipment.\n\n")

    if qstat == game.QSTAT_NO then
        ib:AddMsg("I could also use your help for a small task.\n\n")
        ib:AddLink("Tell me more about your quest", "explain quest")
    elseif qstat == game.QSTAT_ACTIVE then
        ib:SetHeader("st_003", me)
        ib:AddMsg("You still don't have all the cat skins I asked for.")
    end

    ib:AddMsg("\nWhat would you like to do?")
    ib:AddLink("Repair my Equipment", "repair")
    ib:AddLink("Identify Items", "identify")
end

function topicRepair()
    local flag = false
    ib:SetHeader("st_005", me)
    ib:SetTitle("Repair my Equipment")
    ib:SetMsg("Let me check your equipment...\nPerhaps an item needs a fix.\nI will tell you how much each will cost.")
    tmp = pl:FindMarkedObject()
    if tmp ~= nil and tmp.item_quality > 0 and tmp.item_condition < tmp.item_quality then
        local linkstring = "~*M*~ " .. tmp:GetName()

        if string.len(linkstring) >= 48 then
            linkstring = string.sub(linkstring, 1, 41) .. "..."
        end

        linkstring = linkstring .. " (".. tmp.item_condition .. "/" ..
                     tmp.item_quality .. "): " ..
                     pl:ShowCost(repairCost(tmp), game.COSTSTRING_SHORT)
        ib:AddLink(linkstring, "itemfix ".. -1)
        flag = true
    end
    for i=0,game.EQUIP_MAX-1 do
        tmp = pl:GetEquipment(i)
        if tmp ~= nil and tmp.item_quality > 0 and tmp.item_condition < tmp.item_quality then
            local linkstring = tmp:GetName()

            if string.len(linkstring) >= 48 then
                linkstring = string.sub(linkstring, 1, 41) .. "..."
            end

            linkstring = linkstring .. " (".. tmp.item_condition .. "/" ..
                         tmp.item_quality .. "): " ..
                         pl:ShowCost(repairCost(tmp), game.COSTSTRING_SHORT)
            ib:AddLink(linkstring, "itemfix ".. i)
            flag = true
        end
    end
    if flag == false then
        ib:AddMsg("\n\n~Your equipment doesn't need any repair~")
    end
    ib:SetButton("Back", "hi") 
end

function topicItemFix(what)
    local num = tonumber(what)
    if what=="-1" then 
        tmp = pl:FindMarkedObject()
    elseif num ~= nil then
        tmp = pl:GetEquipment(num)
    end
    if tmp == nil then
        topicRepair()
    else
        ib:SetHeader("st_005", me)
        ib:SetTitle("Repairing")
        if tmp.item_quality > 0 and tmp.item_condition < tmp.item_quality then
            ib:SetMsg("Will cost you " ..
                      pl:ShowCost(repairCost(tmp), game.COSTSTRING_SHORT) ..
                      ".\n\n")
            ib:AddMsg("You have " ..
                      pl:ShowCost(pl:GetMoney(), game.COSTSTRING_SHORT) .. ".\n\n")
            ib:AddMsg("Should i repair it now?")

            local icontitle = tmp:GetName()

            if string.len(icontitle) >= 48 then
                icontitle = string.sub(icontitle, 1, 41) .. "..."
            end

            ib:AddIcon(icontitle, tmp:GetFace(), 'Condition: ~'.. tmp.item_condition .. '~    Quality: ~' ..tmp.item_quality .. '~')
            ib:SetAccept("Repair", "fix " .. what)
            ib:SetDecline(nil, "repair") 
        else
            ib:SetMsg("The item doesn't need any repair.")
            ib:SetButton("Back", "repair") 
        end
    end
end

function topicFix(what)
    local num = tonumber(what)
    if what=="-1" then 
        tmp = pl:FindMarkedObject()
    else
        if num == nil then
            topicRepair()
            return
        end
        tmp = pl:GetEquipment(num)
    end
    ib:SetTitle("Pay and Repair")
    ib:SetHeader("st_005", me)
    if tmp == nil then
        ib:SetMsg("Hm, where is the item??")
    else
        if tmp.item_quality > 0 and tmp.item_condition < tmp.item_quality then
            local qua = tmp.item_quality
            if pl:PayAmount(repairCost(tmp)) == 1 then
                tmp:Repair()
                ib:SetMsg("|** ".. me.name .." takes your money and the item **|\n")
                ib:AddMsg("|** after some time he returns **|\n\n")
                if tmp.item_quality < qua then
                    ib:AddMsg("Here is your repaired equipment!\nSadly it has lost a bit in quality!\nAnything else I can do for you?\n")
                else
                    ib:AddMsg("Here is your repaired equipment!\nThank you. Anything else I can do for you?\n")
                end
            else
                ib:SetMsg("You don't have enough money!")
            end
        else
            ib:SetMsg("The item doesn't need any repair.")
        end

        local icontitle = tmp:GetName()

        if string.len(icontitle) >= 48 then
            icontitle = string.sub(icontitle, 1, 41) .. "..."
        end

        ib:AddIcon(icontitle, tmp:GetFace(), 'Condition: ~'.. tmp.item_condition .. '~    Quality: ~' .. tmp.item_quality .. '~')
    end
    ib:SetButton("Back", "repair") 
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

function topicIdentify()
    ib:SetHeader("st_005", me)
    ib:SetTitle("Item Identification")
    ib:SetMsg("Lets see what i can do for you.\nI can ~identify~ a single item or all.\nI can ~detect magic~ or ~detect curse~.\nRember you must mark the single item first.\n\n")
    ib:AddMsg(".You have " .. pl:ShowCost(pl:GetMoney(), game.COSTSTRING_SHORT) .. ".\n\n") 
    ib:AddMsg("What would you like to do?\n")
    tmp = pl:FindMarkedObject()
    if tmp ~= nil then
        if tmp.f_identified ~= true then
            local linkstring = "~*M*~ Identify " .. tmp:GetName()

            if string.len(linkstring) >= 48 then
                linkstring = string.sub(linkstring, 1, 41) .. "..."
            end

            ib:AddLink(linkstring .. ": 150c", "detect single")
        else
            ib:AddMsg("\n~*M*~ Your marked item is already identified.")
        end
    end
    ib:AddLink("Identify all for 5s", "detect all")
    ib:AddLink("Detect magic for 50c", "detect magic")
    ib:AddLink("Detect curse for 50c", "detect curse")
    ib:SetButton("Back", "hi") 
end

function topicDetect(what)
    ib:SetHeader("st_005", me)
    ib:SetTitle("It will cost you")
    if what=="magic" then
        ib:SetMsg("I can cast ~Detect Magic~ for 50c")
    elseif what == "all" then
        ib:SetMsg("I can ~Identify all~ for 5s")
    elseif what == "curse" then
        ib:SetMsg("I can cast ~Detect Curse~ for 50c")
    else
        ib:SetMsg("I can ~Identify One Item~ for 150c")
    end
    ib:AddMsg(" coins.\n\nYou have " .. pl:ShowCost(pl:GetMoney(), game.COSTSTRING_SHORT) .. ".\n\nYou want me to do it now?")
    ib:SetAccept(nil, "cast " .. what) 
    ib:SetDecline(nil, "identify") 
end

function topicCast(what)
    if what == "magic" then
        sum = 50
        spell = "detect magic"
    elseif what == "curse" then
        sum = 50
        spell = "detect curse"
    elseif what == "all" then
        sum = 500
    else
        sum = 150
        mark = pl:FindMarkedObject(); 
        if mark == nil then
            topicIdentify()
            return
        end
    end
    ib:SetHeader("st_005", me)
    ib:SetTitle("Identification...")
    if pl:PayAmount(sum) == 1 then
        if sum == 500 then
            me:IdentifyItem(pl, nil, game.IDENTIFY_ALL); 
        elseif sum == 150 then
            me:IdentifyItem(pl, mark, game.IDENTIFY_MARKED)
        else
            me:CastSpell(pl, game:GetSpellNr(spell), 1, 0, "")
        end
        ib:SetMsg("|**" .. me.name .. " takes your money **|\n\ndone!")
    else
        ib:SetMsg("You don't have enough money!")
    end
    ib:SetButton("Back", "identify") 
end

-- The player asks about available quests
function topicQuest()
    ib:SetHeader("st_003", me)
    if qstat == game.QSTAT_ACTIVE then
        ib:SetTitle(qb:GetName(1))
        ib:AddMsg("You still don't have all the cat skins I asked for.")
    elseif qstat == game.QSTAT_NO then
        quest1_body()
        ib:SetAccept("Accept", "accept quest")
    else
        topicGreeting()
        return
    end
end

-- The player wants to accept a quest. Activate the next accessible one.
function topicAccept()
    if qstat == game.QSTAT_NO then
        ib:SetHeader("st_003", me)
        quest1_body()
        qb:RegisterQuest(questnr, me, ib)
    else
        topicDefault()
        return
    end
end

-- The player claims to have completed a quest. Double check and
-- possibly give out rewards
function topicQuestComplete(reward)
    if qstat == game.QSTAT_ACTIVE then
        ib:SetHeader("st_005", me)
        ib:SetTitle(qb:GetName(1))
        ib:AddMsg("You still don't have all the cat skins I asked for.")
    elseif qstat == game.QSTAT_SOLVED then
		-- We give the item a 5% weapon speed bonus.
		local item_enhance = 0.95
        ib:SetMsg("Many thanks! This will work well, and the chickens will be safe.\n\n")
        
        if reward == nil then
            ib:SetTitle("Select Reward")
            ib:AddMsg("Please select the weapon you want me to rebalance for you:")
			local t = 0
            for item in obj_inventory(pl) do
				-- Check for weapons that have weapon speeds that are able to be enhanced up to a certain level (this case is a 5% enhancement).
                if not item.f_sys_object and item.type == game.TYPE_WEAPON and item.item_level <= 7 then
					t = t + 1 -- Variable t is used for topics.
                    -- Create temporary base arch of item to compare so that its weapon speed can be compared to the item.
					local temp_item = me:CreateObjectInside(item:GetArchName(), game.IDENTIFIED, 1)
					local item_base_ws = temp_item.weapon_speed
					temp_item:Remove()
					-- Check for weapons that have weapon speeds that are able to be enhanced up to a certain level (this case is a 5% enhancement).
					if item_base_ws * item_enhance < item.weapon_speed then
						-- Is the item applied?
						if item.f_applied then
							is_item_applied = " ~(applied)~"
						else
							is_item_applied = ""
						end
						if item.f_identified then
							-- The math.floor() is used to combat rounding errors. We make weapons very slightly better than the check so they won't appear again if the quest is redone.
							ib:AddSelect(item:GetName()..is_item_applied, "quest complete "..t, item:GetFace(), "WS: "..string.format('%0.2f', item.weapon_speed).." sec -> |"..string.format('%0.2f', math.floor(item_base_ws * item_enhance * 100) / 100).." sec|")
						else
							ib:AddSelect(item:GetName().." (unidentified)"..is_item_applied, item:GetFace(), "WS: ??? -> |???|")
						end
					end
                end
            end
            --ib:SetAccept(nil, "quest complete")
            --ib:SetDecline(nil, "greetings force")
        else
            -- Find the previously selected item
            local item
            local i=1
			local obj_base_ws = 0
            for obj in obj_inventory(pl) do
                if not obj.f_sys_object and obj.type == game.TYPE_WEAPON and obj.item_level <= 7 then
					-- Create temporary base arch of item to compare so that its weapon speed can be compared to the item.
					local temp_obj = me:CreateObjectInside(obj:GetArchName(), game.IDENTIFIED, 1)
					obj_base_ws = temp_obj.weapon_speed
					temp_obj:Remove()
					
					if obj_base_ws * item_enhance < obj.weapon_speed then
						if ""..i == reward then 
							item = obj
							break
						else
							i = i + 1
						end
					end
                end
            end

            if item == nil then 
                topicDefault("force")
                return
            end

            ib:SetTitle(string.capitalize(item.name))
            ib:SetMsg("Hmm... Yes, I can do something about that.\n\n")
            ib:AddMsg("Just give me a minute...\n\n")

            ib:AddMsg("|** ".. me.name .." takes the item **|\n")
            ib:AddMsg("|** After some time he returns with it **|\n\n")

            -- Need to unstack if stacked. FIXME: TEST!
            if item.quantity > 1 then
                local newitem = item:Clone()
                newitem.DecreaseNrOf(item.quantity - 1)
                item:DecreaseNrOf()
                newitem:InsertInside(pl)
                item = newitem
            end
			
            -- We give it a 5% speed bonus. Seems to be ok.
			-- We base it on the weapon speed of the base arch, preventing exploits with alts.
            item.weapon_speed = math.floor(obj_base_ws * item_enhance * 100) / 100
            pl:Fix()
                
            pl:Write("|** Your "..item.name.." seems slightly better balanced now. **|")

            ib:SetButton("Back", "hello")
            qb:Finish(1)
            pl:Sound(0, 0, 2, 0)
        end
    else
        topicDefault()
        return
    end
end

local function topicChereth()
    ib:SetHeader("st_002", me)
    ib:SetTitle("Chereth")
    ib:SetMsg("\n\nChereth was the guild's master archer until she unfortunately lost her sight in battle.")
    ib:AddMsg("\n\nNowadays she usually hangs out downstairs in the guild.")
    ib:SetButton("Back", "hi")
end


local function topicRebalance(itemnum)

end


tl = TopicList()
tl:AddGreeting({"greetings (force)"}, topicDefault)
tl:SetDefault(topicDefault)
tl:AddTopics("detect (.*)", topicDetect) 
tl:AddTopics("cast (.*)", topicCast) 
tl:AddTopics("repair", topicRepair) 
tl:AddTopics("itemfix (.*)", topicItemFix) 
tl:AddTopics("fix (.*)", topicFix)
tl:AddTopics("identify", topicIdentify) 

tl:AddTopics("chereth", topicChereth)

tl:AddTopics({"quest", "explain%s+quest"}, topicQuest)
tl:AddTopics({"accept", "accept%s+quest"}, topicAccept)
tl:AddTopics({"complete", "quest%s+complete%s*#?(%d*)"}, topicQuestComplete)

ib:ShowSENTInce(game.GUI_NPC_MODE_NPC, tl:CheckMessage(event, true))
