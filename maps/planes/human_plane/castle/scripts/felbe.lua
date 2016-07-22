require("topic_list")
require("interface_builder")

local pl = event.activator
local me = event.me
local ds = DataStore("felbe_letter_quest", pl)

local ib = InterfaceBuilder()
ib:SetHeader(me, me.name)

local ds_msg = ds:Get("felbe_msg")

local function topicDefault()
    if pl:CheckInventory(1, nil, "Felbe's Letter", nil, -1) == nil and pl:CheckInventory(1, nil, "Guard Captain Aldus's Letter", nil, -1) == nil then
        if ds_msg == "felbe_letter_done" then
            me:SayTo(pl, "Talk to Aldus for your reward!")
            pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
            return
        elseif ds_msg == "felbe_letter_really_done" then
            pl:Write(me.name.." thanks you for helping out.", 2)
            pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
            return
        else
            ib:SetTitle("Hello There") -- set title to whatever you want
            ib:SetMsg("Hello, the name's Felbe.\nThe forests seem quiet these days, but don't let that fool you. The forest is always quiet, but if you pay close attention you will begin to uncover its mysteries.") -- set a first message
            ib:AddMsg("\n\nEither the forest is playing a trick on me, or someone has been tampering with my letters. In any case, I need help getting a message safely to the castle.\nWill you help?") -- add another message to the first \n is a space in interface
            ib:SetAccept("I'll Help", "accept") -- change nil to something like "Sure" - be sure you set it with quotes!
            ib:SetDecline("Can't...", "quit") -- same as above but this is the bye button
        end
    elseif pl:CheckInventory(1, nil, "Guard Captain Aldus's Letter", nil, -1) then
        ib:SetTitle(me.name.." thanks to you.")
        local aldus_letter = pl:CheckInventory(1, nil, "Guard Captain Aldus's Letter", nil, -1)
        aldus_letter:Remove()
        ib:AddMsg("\nHe sent you back? Someone must be intercepting our letters then. I'll be on my guard, and I'd advise you to do the same. May your arrows fly for the heart and may you have a safe journey.") -- msg after you brang to Felbe the Aldus's Letter
        ds:Set("felbe_msg", "felbe_letter_done")
    elseif pl:CheckInventory(1, nil, "Felbe's Letter", nil, -1) then
        me:SayTo(pl, "You've already got the letter -- now go and bring it to Aldus!")
        pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
        return
    else
        pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
        return
    end
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

local function topicAccept()
    if pl:CheckInventory(1, nil, "Felbe's Letter", nil, -1) == nil or pl:CheckInventory(1, nil, "Guard Captain Aldus's Letter", nil, -1) == nil then
        ib:SetTitle("Instructions") -- set title to whatever you want again
        ib:SetMsg("Great. Take the letter to Stonehaven Castle and get it to the ^captain of the guard^, Aldus. We are supposed to be in contact in case I find something odd here or he needs my services elsewhere. \nI haven't heard from him in at least a week and I suspect we have someone intercepting our messages.") -- set a first message again
        ib:AddMsg("\n\nBe careful on your way. They've been intercepting our ^carriers^ and I suspect they might try to intercept even you. Aldus will handle it once he knows what's going on.") -- add another message ( remove if you don't need another message )
        if pl:CheckInventory(1, nil, "Felbe's Letter", nil, -1) then
            ib:AddMsg("\n\nYou already have the letter, so you best be heading northeast towards the castle.") -- msg that shows if you already have the Felbe's Letter
        elseif pl:CheckInventory(1, nil, "Felbe's Letter", nil, -1) == nil then
            local letter = pl:CreateObjectInside("letter", 1,1)
            letter.message = "" -- Set the message for letter
            letter.name = "Felbe's Letter"
            letter.f_identified = 1
            letter.f_no_drop = 1
            ds:Set("felbe_msg", "felbe_letter")
            ib:AddMsg("\n\nHere's the letter, now hurry and go.")
        end
    else
        pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
        return
    end
    ib:SetButton("Back", "hi")
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

local function topicCarriers()
    ib:SetTitle("Carriers")
    ib:SetMsg("My carriers? Oh, I use hawks, generally. Sometimes I use Theodore, my dog, because he's much more reliable in the weather.")
    ib:SetButton("Back", "hi")
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

local function topicAldus()
    ib:SetTitle("Aldus, Captain of the Guard")
    ib:SetMsg("Aldus is an honorable man; he fits his position nicely. I would never doubt his word, even if King <insert name here> himself spoke against him.")
    ib:SetButton("Back", "hi")
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

local function topicQuit()
    pl:Interface(game.GUI_NPC_MODE_NPC, ib:Build())
end

tl = TopicList()
tl:AddGreeting(nil, topicDefault)
tl:AddTopics("quit", topicQuit)
tl:AddTopics("accept", topicAccept)
tl:AddTopics("carriers", topicCarriers)
tl:AddTopics("captain of the guard", topicAldus)
tl:SetDefault(topicDefault)
tl:CheckMessage(event)
