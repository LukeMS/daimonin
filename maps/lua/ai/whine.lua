-- Shows how to write action behaviours
if math.random(10) == 1 then
    local whines = {
        "I hate myself", "I don't like it here", "I have a bad feeling about this",
        "I'm cold", "I'm hungry", "I'm thirsty", "I'm bored",
        "You smell bad", "Is this the way to the cafeteria?",
        "Don't look at me that way", "Don't leave me alone", "Don't kill me",
        "Have you met Teddy?"
    }

    event.me:Say(whines[math.random(table.getn(whines))])

    -- Mark this as our single action for this tick
    event.returnvalue = 1
end
