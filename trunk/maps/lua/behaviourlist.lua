--
-- Support functions for inspecting and maniulating behaviourlists
--

--
-- The Behaviourlist object will behave as a proxy for the 
-- behaviourtable it is created with. That means you can access
-- table entries directly through the Behaviourlist:
--
-- list = Behaviourlist(obj:GetAI():GetBehaviourlist())
-- first_move = list["moves"][1]
-- print(first_move["name"])
--

Behaviourlist = {}

-- Constructor
--
-- source should be a "behaviourtable" like the one returned by 
-- ai:GetBehaviourlist() (see end of file for example). It can of course
-- also be constructed from scratch if one is careful about the table
-- layout.
-- source can also be a GameObject (a monster) or an AI object
function Behaviourlist:New(source)
    local obj = {}
    
    local function idx(table, key)
        local f = rawget(Behaviourlist, key)
        if f ~= nil then return f end
        return rawget(table["behaviourtable"], key)
    end

    setmetatable(obj,
        {
            __index = idx,
            __tostring = Behaviourlist.ToString
        }
    )

    local t = type(source)
    if t == "table" then
        obj["behaviourtable"] = source
    elseif t == "AI" then
        obj["behaviourtable"] = source:GetBehaviourlist()
    elseif t == "GameObject" and source.type == game.TYPE_MONSTER then
        obj["behaviourtable"] = source:GetAI():GetBehaviourlist()
    else
        error("Behaviourlist:New() currently needs a table created with ai:GetBehaviourlist(), a monster object or an ai object as input.")
    end

    return obj
end

setmetatable(Behaviourlist, { __call = Behaviourlist.New })

-- Convert to a string representation fit for insertion into AI objects
function Behaviourlist:ToString()
    local ret = ""
    for classname, behaviours in pairs(self.behaviourtable) do
        ret = ret .. classname .. ":\n"
        for i,behaviour in ipairs(behaviours) do            
            ret = ret .. behaviour["name"]
            if behaviour["parameters"] then
                for paramname, values in behaviour["parameters"] do
                    for i,value in values do
                        ret = ret .. " " .. paramname .. "=" .. value
                    end
                end
            end
            ret = ret .. "\n"
        end
        ret = ret .. "\n"
    end
    return ret
end

-- Replace a mob's AI with the this behaviourlist
function Behaviourlist:ReplaceAI(mob)
    local ai_obj

    assert(mob.type == game.TYPE_MONSTER, "Can only replace AIs of monsters")

    -- Try finding an existing AI
    for ai_obj in obj_inventory(mob) do
        if ai_obj.type == game.TYPE_AI then 
            break
        end
    end

    -- Create new if none found
    if ai_obj == nil then
        ai_obj = mob:CreateObjectInside("empty_ai",0,0,0)
        assert(ai_obj ~= nil, "Couldn't create a new AI from arch 'empty_ai'")
    end

    ai_obj.message = self:ToString()
    mob:GetAI():ReloadBehaviourlist()
end

--
-- This is an example "behaviourtable" from ai:GetBehaviourlist()
-- Note that the behaviours in each class are indexed both by their
-- position number (for exact ordering), and by their names (for
-- quick and dirty access).
--
-- stringint parameters are written as this: "<string>:<int>", e.g.:
-- ["name"] = {"michtoen:-1000"}
--
--[[
  ai = {
    processes = {
      {
        name = "look_for_other_mobs",
        parameters = {},
      },
      {
        name = "friendship",
        parameters = {
          arch = { },
          same_alignment = { 100 },
          opposite_alignment = { -100 },
        },
      },
      look_for_other_mobs = ai["processes"][1],
      friendship = ai["processes"][2],
    },
    moves = {
    }
    actions = {
    }
  }
--]]
