require ("topic_list.lua")

local me = event.me;
local activator = event.activator;
local msg = string.lower(event.message);
local words = string.split(msg);
local marked = activator:FindMarkedObject();

-- Tests for the lua backend and the object model
if msg == 'exception' then
    me:SayTo(activator, "I will now raise an exception...")
    error('hello world')
elseif msg == 'tostring' then
    me:SayTo(activator, "You are:  " .. tostring(activator) .. ".\nI am " .. tostring(me))
elseif msg == 'typesafe1' then
    me:SayTo(activator, "Testing type safety (wrapping strange objects)")
    other = me.map
    me:SayTo(other, "This will not be written nor crash the server")
elseif msg == 'typesafe2' then
    me:SayTo(activator, "Testing type safety (wrapping strange objects)")
    other = event.other -- # This is NULL for 'Say' scripts
    me:SayTo(other, "This will not be written nor crash the server")
elseif msg == 'globals' then
    me:SayTo(activator, "Value of a_global was " .. tostring(a_global))
    me:SayTo(activator, "Setting it to 42, please rerun this test")
    a_global=42
    
-- Test for bug #0000014 (Division by zero)
elseif msg == 'division' then
    me:SayTo(activator, "Trying to calculate x = 42 / 0. (Should give inf)")
    me:SayTo(activator, "x = " .. tostring(42 / 0))
    me:SayTo(activator, "Done calculating.")
    
-- Test some of the security features
elseif msg == 'security1' then
    if activator.f_wiz then
        me:SayTo(activator, "You are DM, trying to remove it (should fail).")
        activator.f_wiz = false;
    else
        me:SayTo(activator, "You aren't DM, trying to elevate you (should fail).")
        activator.f_wiz = true;
    end
elseif msg == 'security2' then
    me:SayTo(activator, "Trying to open /etc/passwd (should fail).")
    f = io.open("/etc/passwd", "r")
elseif msg == 'security3' then
    me:SayTo(activator, "Trying to open /etc/passwd (should fail).")
    require("/etc/passwd")

-- Test coroutines
elseif (msg == 'yield') then
    me:SayTo(activator, 'Taking a break...')
    coroutine.yield(5)
    me:SayTo(activator, '... tick ...')
    coroutine.yield(5)
    me:SayTo(activator, '... tack ...')
    coroutine.yield(5)
    me:SayTo(activator, '... terminate!')
elseif (msg == 'yield2') then
    me:SayTo(activator, 'Taking a break...')
    coroutine.yield()
    me:SayTo(activator, '... terminate!')
elseif (msg == 'yield3') then
    me:SayTo(activator, 'Taking a break...')
    return 5

-- Test food-related things
elseif (msg == 'food') then
    me:SayTo(activator,"Your food was " .. activator.food);
    activator.food = 500;
    me:SayTo(activator,'Your stomach is filled again.');
elseif (msg == 'food2') then
    me:SayTo(activator,'Trying to overfill: ');
    activator.food = 2000;

-- Misc tests
elseif (msg == 'messaging') then
    me:Say('This is a message for everybody on the map in a range of ' .. game.MAP_INFO_NORMAL .. ' tiles')
    me:SayTo(activator, 'This is SayTo just for you');
    me:SayTo(activator, 'This is SayTo again but in mode 1 (no xxx say)', 1);
    activator:Write('This is a non-verbal message just for you in red color.', game.COLOR_RED);
    activator:Write('This is a non-verbal message just for you in default color.');
    activator.map:Message(me.x, me.y, 8, 'This is a map-wide Message() for everybody in a range of 8 tiles');
    me:Communicate('/kiss');
    me:Communicate('/hug ' .. activator.name);
elseif (msg == 'matchstring') then
    me:SayTo(activator, "match(foo, bar) = " .. tostring(game:MatchString("foo", "bar")))
    me:SayTo(activator, "match(foo, foo) = " .. tostring(game:MatchString("foo", "foo")))
    me:SayTo(activator, "match(foo, FOO) = " .. tostring(game:MatchString("foo", "FOO")))
elseif (msg == 'getip') then
    me:SayTo(activator, "Your IP is " .. activator:GetIP())

elseif (words[1] == 'skill') then
    if (words[2] == nil) then
        me:SayTo(activator, "Say ^skill <skill>^, e.g. ^skill punching^");
    else
        local skillnr = game:GetSkillNr(words[2]);
        me:SayTo(activator, "Checking on skill "..words[2].." (nr "..skillnr..")");
        if activator:FindSkill(skillnr) == nil then
            me:SayTo(activator, "You do not know that skill!");
        else
            exp = activator:GetSkill(game.TYPE_SKILL, skillnr).experience;
            me:SayTo(activator, "Your experience in "..words[2].." is "..exp);
            activator:SetSkill(game.TYPE_SKILL, skillnr, 0, exp + 1000);
            me:SayTo(activator, "Tried to set it to " .. (exp + 1000));
            me:SayTo(activator, "Your new experience is " .. (activator:GetSkill(game.TYPE_SKILL,skillnr)).experience);
        end
    end
elseif (msg == 'god') then
-- TODO: change to god when finished
    god = activator:GetGod();
    me:SayTo(activator, "Your current god is " .. god);
    --    if god == 'Eldath') then
    --        SetGod(activator, 'Snafu')
    --    elseif (god == 'Snafu') then
    --        SetGod(activator, 'Eldath')
    --    me:SayTo(activator, "Your new god is " + GetGod(activator))
elseif (words[1] == 'findplayer') then
    local player;
    if (words[2] == nil) then
        me:SayTo(activator, "Trying to find you. You can find another player by saying) then  ^findplayer NAME^");
        player = game:FindPlayer(activator.name);
    else
        player = game:FindPlayer(words[2]);
    end
    if (player == nil) then
        me:SayTo(activator, "Sorry, couldn't find " .. words[2]);
    else
        me:SayTo(activator, "Found " .. player.name .. " on the map " .. tostring(player.map));
    end
elseif (words[1] == 'setgender') then
    if (words[2] == nil) then
        me:SayTo(activator, "Try ^setgender male^, ^setgender female^, ^setgender both^ or ^setgender neuter^");
    else
        me:SayTo(activator, "Setting your gender to " .. words[2]);
        if(words[2] == 'male') then
            activator:SetGender(game.MALE)
        elseif(words[2] == 'neuter') then
            activator:SetGender(game.NEUTER)
        elseif(words[2] == 'female') then
            activator:SetGender(game.FEMALE)
        elseif(words[2] == 'both') then
            activator:SetGender(game.HERMAPHRODITE)
        end
    end
elseif (msg == 'sound') then
    me:SayTo(activator, "I can imitate a teleporter");
    me.map:PlaySound(me.x, me.y, 32, 0);
elseif (msg == 'tod') then
    tod = game:GetTime()
    for k,v in tod do
        me:SayTo(activator, k .. ": " .. v);
    end

-- Setting/Getting of stats
-- TODO: this part is completely outdated...
elseif (words[1] == 'stat') then
    local id = -1;
    local modded = '';

    if (words[2] == nil) then
        me:SayTo(activator, "I need a stat to change, e.g. ^stat int^.")
    elseif (words[2] == 'con') then
        id = game.CONSTITUTION
    elseif (words[2] == 'int') then
        id = game.INTELLIGENCE
    elseif (words[2] == 'str') then
        id = game.STRENGTH
    elseif (words[2] == 'pow') then
        id = game.POWER
    elseif (words[2] == 'wis') then
        id = game.WISDOM
    elseif (words[2] == 'dex') then
        id = game.DEXTERITY
    elseif (words[2] == 'cha') then
        id = game.CHARISMA
    elseif (words[2] == 'hp') then
        id = game.HITPOINTS
    elseif (words[2] == 'sp') then
        id = game.SPELLPOINTS
    elseif (words[2] == 'grace') then
        id = game.GRACE
    elseif (words[2] == 'maxhp') then  -- set shouldn't work...
        id = game.MAX_HITPOINTS
    elseif (words[2] == 'maxsp') then  -- set shouldn't work...
        id = game.MAX_SPELLPOINTS
    elseif (words[2] == 'maxgrace') then  -- set shouldn't work...
        id = game.MAX_GRACE
    elseif (words[2] == 'ac') then -- set shouldn't work...
        id = game.ARMOUR_CLASS
    elseif (words[2] == 'wc') then  -- set shouldn't work...
        id =  game.WEAPON_CLASS
    elseif (words[2] == 'damage') then  -- set shouldn't work...
        id = game.DAMAGE
    elseif (words[2] == 'luck') then  -- set shouldn't work...
        id = game.LUCK
    else
        me:SayTo(activator, "Unknown stat: " .. words[2])
    end

    if (id ~= -1) then
        if (id == game.CONSTITUTION or id == game.INTELLIGENCE or id == game.STRENGTH or id == game.POWER or id == game.WISDOM or id == game.DEXTERITY or id == game.CHARISMA) then
            old = activator.GetUnmodifiedAttribute(id)
            modded = " (modded to " .. activator.id .. ")"
        else
            old = activator.GetAttribute(id)
        end

        me:SayTo(activator, "Your current "..words[2]..modded.." stat: ".. old);
        me:SayTo(activator, "Increasing to " .. (old+1));
        activator.id = old+1;
   end

-- misc attributes
elseif (words[1] == 'alignment') then
    align = activator:GetAlignmentForce();
    me:SayTo(activator, "Your old alignment was " .. align.title);
    if (words[2] == nil) then
        me:SayTo(activator, "Try ^alignment ALIGNMENT^")
    else
        me:SayTo(activator, "Setting your alignment to " .. words[2]);
        activator:SetAlignment(words[2]);
        me:SayTo(activator, "Your new alignment is " .. activator:GetAlignmentForce().title);
    end
elseif (words[1] == 'experience') then
    me:SayTo(activator, "Your overall exp is " .. activator.experience)
elseif (words[1] == 'direction') then
    me:SayTo(activator, "You are facing direction " .. activator.direction)
elseif (msg == 'player_force') then
    me:SayTo(activator, "Adding a temporary boosting force to you");
    force = activator:CreatePlayerForce("test force", 5);
    if (force ~= nil) then
        force.intelligence = 1;
        force.damage = 10;
        force.armour_class = 10;
        force.weapon_class = 10;
        force.luck = 10;
        force.max_hitpoints = 10;
        force.max_spellpoints = 10;
        force.max_grace = 10 ;
    end
elseif (words[1] == 'flags') then
    conf = activator.f_confused;
    if (conf == true) then
        me:SayTo(activator, "You are confused. I'm removing it...");
        activator.f_confused = false;
    else
        me:SayTo(activator, "You aren't confused. Fixing...");
        activator.f_confused = true;
    end
elseif (words[1] == 'archname') then
    if (marked == nil) then
        me:SayTo(activator, "Your archname is " .. activator:GetArchName())
    else
        me:SayTo(activator, "Your item is a " .. marked:GetArchName())
    end

-- Some object-property-changing tests
-- TODO: create more tests for the "inventory-update" problem
elseif (msg == 'weight') then
    if (marked == nil) then
        me:SayTo(activator, "You must mark an item first.");
    else
        me:SayTo(activator, "Your item weighed " .. marked.weight .. "g");
        marked.weight = marked.weight - 5;
        me:SayTo(activator, "Your item now weighs " .. marked.weight .. "g");
    end
elseif (msg == 'quantity') then
    if (marked == nil) then
        me:SayTo(activator, "You must mark an item first.");
    else
        me:SayTo(activator, "There were " .. marked.quantity .. " items");
        marked.quantity = marked.quantity + 1;
        me:SayTo(activator, "Now there are " .. marked.quantity);
    end
elseif (msg == 'value') then
    if (marked == nil) then
        me:SayTo(activator, "You must mark an item first.");
    else
        me:SayTo(activator, "The old value of your object was "..marked.value);
        marked.value = marked.value + 100;
        me:SayTo(activator, "The new value of your object is "..marked.value);
    end
elseif (msg == 'cursed') then
    if (marked == nil) then
        me:SayTo(activator, "You must mark an item first.");
    else
        if (marked.f_cursed) then
            marked.f_cursed = false;
            me:SayTo(activator, "Your item is now uncursed");
        else
            marked.f_cursed = true;
            me:SayTo(activator, "Your item is now cursed");
        end
    end
elseif (words[1] == 'name') then
    if (marked == nil) then
        me:SayTo(activator, "You must mark an item first.");
    else
        if (words[2] == nil) then
            me:SayTo(activator, "Say ^name NEWNAME^")
        else
            marked.name = words[2];
            me:SayTo(activator, "Your item is now called " .. marked.name);
        end
    end
elseif (words[1] == 'title') then
    if (marked == nil) then
        me:SayTo(activator, "You must mark an item first.")
    else
        if (words[2] == nil) then
            me:SayTo(activator, "Say ^title NEWNAME^")
        else
            if (marked.title == nil) then
                me:SayTo(activator, "Your item had no title ")
            else
                me:SayTo(activator, "Your item had the title " .. marked.title)
            end
            marked.title = words[2]
            me:SayTo(activator, "Your item is now titled " .. marked.title)
        end
    end
elseif (words[1] == 'slaying') then
    if (marked == nil) then
        me:SayTo(activator, "You must mark an item first.")
    else
        if (words[2] == nil) then
            me:SayTo(activator, "Say ^slaying NEWNAME^");
        else
            if (marked.slaying == nil) then
                me:SayTo(activator, "Your item had no slaying")
            else
                me:SayTo(activator, "Your item slays " .. marked.slaying);
            end
            marked.slaying = words[2];
            me:SayTo(activator, "Your item now slays " .. marked.slaying);
       end 
    end

-- Test rank
elseif (msg == 'rank1') then
    me:SayTo(activator, 'Setting rank to "Mr"...');
    activator:SetRank('Mr');
elseif (msg == 'rank2') then
    me:SayTo(activator, 'Setting rank to "President"...');
    activator:SetRank('President')

-- Test cloning
elseif (msg == 'clone') then
    me:SayTo(activator, 'Meet your evil twin')
    clone = activator:Clone(game.CLONE_WITHOUT_INVENTORY)
    clone:SetPosition(10, 10)
    clone.f_friendly = false
    clone.f_monster = true
    clone.max_hitpoints = 1
    clone.level = 1
    clone.name = clone.name .. " II"
    clone:Fix()
elseif (msg == 'enemy') then
    enemy = activator.enemy
    if (enemy == nil) then
        me:SayTo(activator, 'You have no enemy currently');
    else
        me:SayTo(activator, 'Your enemy is ' .. enemy.name)
        if (enemy.enemy == nil) then
            me:SayTo(activator, enemy.name .. " has no enemy.")
        else
            me:SayTo(activator, enemy.name .. "'s enemy is " .. enemy.enemy.name)
        end
    end

-- Test {Set|Get}{Face|Animation}
elseif words[1] == 'anim' then
    if words[2] == nil then
        me:SayTo(activator, "Say ^anim <name>^ to make me switch animation")
        if me:GetAnimation() ~= "monk" then
            me:SayTo(activator, "Switching back to monk animation")
            me:SetAnimation("monk")
            me.f_is_animated = true
        end
    else
        me:SayTo(activator, "Switching to animation " .. words[2])
        me:SetAnimation(words[2])
        me.f_is_animated = true
    end
elseif words[1] == 'face' then
    if words[2] == nil then
        me:SayTo(activator, "Say ^face <name>^ to make me switch face")
        me:SayTo(activator, "My current face is " .. tostring(me:GetFace()))
        me:SayTo(activator, "My current inventory face is " .. tostring(me:GetInvFace()))
    else
        me:SayTo(activator, "Switching to face " .. words[2]);
        me:SetFace(words[2])
        me.f_is_animated = false
    end

elseif (msg == 'invisible') then
    -- FIXME: Seems to have some problems if the player is made invisible (UP_INV_XXX flag ????)
    if (activator.f_is_invisible) then
        me:SayTo(activator, "Where are you? I can't see you! Aha, making you visible!");
        activator.f_is_invisible = false;
    else
        me:SayTo(activator, "Making you invisible...");
        activator.f_is_invisible = true;
    end

-- Test some crash-prone object functions
elseif (msg == 'setposition1') then
    me:SayTo(activator, "Trying to move one step west")
    me:SetPosition(me.x-1, me.y)
elseif (msg == 'setposition2') then
    me:SayTo(activator, "Dropping a note one step to the west")
    obj = me:CreateObjectInside("note", 0, 1)
    me:Drop("note")
    obj:SetPosition(me.x - 1 ,me.y)
elseif (msg == 'setposition3') then
    me:SayTo(activator, "Putting a note from my inventory one step to the west (probably won't work...)")
    obj = me:CreateObjectInside("note", 0, 1)
    obj:SetPosition(me.x-1, me.y)
elseif (msg == 'setposition4') then
    me:SayTo(activator, "Putting a note from my inventory one step to the west (using the new API)")
    obj = me:CreateObjectInside("note", 0, 1)
    obj:SetPosition(me.map, me.x-1, me.y)
elseif (msg == 'setposition5') then
    me:SayTo(activator, "Moving you out of here")
    activator:SetPosition(game:ReadyMap("/dev/testmaps/testmap_main"), 10, 10)

-- Pickup and drop objects
elseif (msg == 'drop1') then
    me:SayTo(activator, "Dropping a note")
    me:CreateObjectInside("note", 0, 1)
    me:Drop("note")
elseif (msg == 'drop2') then
    me:SayTo(activator, "Oops, you dropped a note")
    activator:CreateObjectInside("note", 0, 1)
    activator:Drop("note")

elseif (msg == 'pickup1') then
    tmp = me.map:GetFirstObjectOnSquare(me.x, me.y)
while (tmp ~= nil) do
    if (tmp.name == 'note') then break end
        tmp = tmp.above
    end
    if (tmp == nil) then
        me:SayTo(activator, "Sorry, couldn't find anything to pick up. Try using ^drop1^ first.")
    else
        me:PickUp(tmp)
    end
elseif (msg == 'pickup2') then
    tmp = activator.map:GetFirstObjectOnSquare(activator.x, activator.y)
    while (tmp ~= nil) do
    if (tmp.name == 'note') then break end
       tmp = tmp.above
    end

    if (tmp == nil) then
        me:SayTo(activator, "Nothing to pick up. Try using ^drop2^ first.")
    else
        activator:PickUp(tmp)
    end

elseif (words[1] == "getattribute") then
    me:SayTo(activator, "Name: " .. tostring(activator.name))
    me:SayTo(activator, "Race: " .. tostring(activator.race))
    me:SayTo(activator, "Weight: " .. tostring(activator.weight))
    me:SayTo(activator, "Exp: " .. tostring(activator.experience))
    me:SayTo(activator, "Speed: " .. tostring(activator.speed))
elseif (words[1] == "setattribute") then
    activator.title = "the elite"
    activator.speed = 2.0 -- # Shouldn't work on player...

--  Test map object model, functions and object queries
elseif msg == 'map1' then
    me:SayTo(activator, "We are now on map " .. tostring(me.map))
elseif msg == 'map2' then
    me:SayTo(activator, "This map ("..me.map.path..") is "..me.map.width.."x"..me.map.height)
elseif msg == 'map3' then
    saywhat = "Object list for square (1,1): "
    object = me.map:GetFirstObjectOnSquare(1,1)
    while (object ~= nil) do
        saywhat = saywhat .. "\n  " .. tostring(object)
        object = object.above
    end
    me:SayTo(activator, saywhat)
elseif msg == 'createobject' then
    note = me.map:CreateObject("note", me.x-1, me.y-1)
    note.message = "I was created out here"
elseif msg == 'brightness' then
    b1 = me.map:GetBrightnessOnSquare(me.x, me.y, 0)
    b2 = me.map:GetBrightnessOnSquare(me.x, me.y, 1)
    me:SayTo(activator, "Global map brightness is "..me.map.darkness.." or "..me.map.light_value.." depending on scale")
    me:SayTo(activator, "Brightness here is "..b1.." or "..b2.." depending on scale")

-- Test beacons
elseif msg == 'beacon' then
    b1 = game:LocateBeacon('script_tester_beacon_1')
    b2 = game:LocateBeacon('script_tester_beacon_2')
    b3 = game:LocateBeacon('script_tester_beacon_3')

    if b1 == nil then
        me:SayTo(activator, "FAIL: Couldn't find script_tester_beacon_1")
    else
        me:SayTo(activator, "Found beacon 1 inside "..tostring(b1.environment.name))
    end

    if b2 == nil then
        me:SayTo(activator, "FAIL: Couldn't find script_tester_beacon_2")
    else
        me:SayTo(activator, "Found beacon 2 on "..tostring(b2.map.name))
    end
    
    if b3 == nil then
        me:SayTo(activator, "OK: Couldn't find beacon 3")
    else
        me:SayTo(activator, "FAIL: Found beacon 3")
    end

-- Help the Drow to test recursion (see Drowscript.lua)
elseif msg == 'recursive_part_2' then
    tl = TopicList()
    tl:SetDefault( function() event.me:Say("lvl2 tl: something wierd happened to the topiclist class") end)
    tl:AddTopics("recursive_part_2", function() event.me:Say("lvl2 tl ( CORRECT )") end)
    tl:CheckMessage(event)

    me:Say("Hey, the drow wants me to recurse a little...")
    me:Communicate("recursive3")

    tl2 = TopicList()
    tl2:SetDefault( function() event.me:Say("lvl2 tl: something weird happened to the topiclist class") end)
    tl2:AddTopics("recursive_part_2", function() event.me:Say("lvl2 tl ( CORRECT )") end)
    tl2:CheckMessage(event)

elseif msg == 'recursive' or msg == 'recursive2' then
    -- me:Say("I'm keeping quiet")
    
     
else
    me:SayTo(activator,
        "Available tests:\n" ..
        "^exception^ ^tostring^ ^typesafe1^ ^globals^ ^division^\n" ..
        "^security1^ ^security2^ ^security3^\n" ..
        "^food^ ^food2^\n" ..
        "^invisible^ ^messaging^ ^getip^\n" ..
        "^rank1^ ^rank2^ ^clone^ ^enemy^\n" ..
        "^map1^ ^map2^ ^map3^ ^createobject^\n" ..
        "^value^ ^cursed^ ^quantity^ ^weight^\n" ..
        "^name^ ^title^ ^slaying^ ^archname^\n" ..
        "^skill^ ^matchstring^ ^god^\n" ..
        "^findplayer^ ^setgender^ ^sound^ ^tod^\n" ..
        "^player_force^ ^flags^\n" ..
        "^setposition1^ ^setposition2^ ^setposition3^\n" ..
        "^setposition4^ ^setposition5^\n" ..
        "^drop1^ ^pickup1^ ^drop2^ ^pickup2^\n" ..
        "^alignment^ ^experience^ ^direction^\n" ..
        "^anim^ ^anim2^\n" ..
        "^stat {str|dex|con|pow|wis|cha|hp|sp|grace|maxhp|maxsp|maxgrace|ac|wc|luck}^\n" ..
        "^setface^ ^getattribute^ ^setattribute^")
end
