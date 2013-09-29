-------------------------------------------------------------------------------
-- script_context.lua | Utility
--
-- The script context is a sort of container for the helper functions, modules,
-- utilities, and associated objects (and their metadata) used in a script. By
-- storing this data in a central script context, different chunks (files) --
-- helper scripts, modules, utilities, the event chunk itself, etc -- can all
-- easily access the same copy.
--
-- Unlike most utilities, you do not need to create a working copy of the
-- script context -- in fact, attempting to do so will generate an error.
-- Instead, use ScriptContext directly.
-------------------------------------------------------------------------------
---------------------------------------
-- If ScriptContext already exists then check it is the correct addon type.
-- If it is not, this is an error. If it is, we (presumably) have already
-- required this chunk so no need to do it again.
---------------------------------------
if ScriptContext ~= nil then
    local a, b, c = type(ScriptContext)
    assert(a == "addon" and
        b == "utility" and
        c == "sc",
        "Global ScriptContext exists but is not an addon utility sc!")
    return
end

---------------------------------------
-- Assign the global ScriptContext table. Give it a metatable. The __call
-- metamethod means that calling ScriptContext() generates an error.
---------------------------------------
ScriptContext = { func = {}, module = {}, object = {}, utility = {} }
setmetatable(ScriptContext, {
    __call = function()
        error("Do not create an instance of ScriptContext, use the utility " ..
            "directly!")
    end,
    __metatable = function() return "addon", "utility", "sc" end
})

---------------------------------------
-- Generally, for each data type -- Function, Module, Object, Utility --
-- ScriptContext provides 3 methods for manipulating the data:
-- ScriptContext:Add... (adds a new entry); ScriptContext:Del... (removes an
-- entry); ScriptContext:Get... (reads an entry).
--
-- The first method takes two arguments: key (a string); value (the appropriate
-- type). The others only take the key argument. On failure the methods return
-- nil. On success they return the value associated with key (for Add...
-- methods this is the same value as passed in the value argument and for
-- Del... methods this value is the old value of key, which is then deleted).
---------------------------------------
local function add(component, key, value)
    if ScriptContext[component][key] == nil then
        ScriptContext[component][key] = value
    end
    return ScriptContext[component][key]
end
local function del(component, key)
    local value = ScriptContext[component][key]
    ScriptContext[component][key] = nil
    return value
end
local function get(component, key)
    return ScriptContext[component][key]
end

---------------------------------------
-- Functions.
---------------------------------------
function ScriptContext:AddFunction(key, value)
    assert(type(key) == "string", "Arg #1 must be string!")
    assert(type(value) == "function", "Arg #2 must be function!")
    return add("func", key, value)
end
function ScriptContext:DelFunction(key)
    assert(type(key) == "string", "Arg #1 must be string!")
    return del("func", key)
end
function ScriptContext:GetFunction(key)
    assert(type(key) == "string", "Arg #1 must be string!")
    return get("func", key)
end

---------------------------------------
-- Modules.
---------------------------------------
function ScriptContext:AddModule(key, value)
    assert(type(key) == "string", "Arg #1 must be string!")
    local a, b = type(value)
    assert(a == "addon" and
        b == "module", "Arg #2 must be addon module!")
    return add("module", key, value)
end
function ScriptContext:DelModule(key)
    assert(type(key) == "string", "Arg #1 must be string!")
    return del("module", key)
end
function ScriptContext:GetModule(key)
    assert(type(key) == "string", "Arg #1 must be string!")
    return get("module", key)
end

---------------------------------------
-- Objects.
---------------------------------------
function ScriptContext:AddObject(key, value)
    assert(type(key) == "string", "Arg #1 must be string!")
    assert(type(value) == "GameObject", "Arg #2 must be GameObject!")
    return add("object", key, value)
end
function ScriptContext:DelObject(key)
    assert(type(key) == "string", "Arg #1 must be string!")
    return del("object", key)
end
function ScriptContext:GetObject(key)
    assert(type(key) == "string", "Arg #1 must be string!")
    return get("object", key)
end

---------------------------------------
-- Utilities.
---------------------------------------
function ScriptContext:AddUtility(key, value)
    assert(type(key) == "string", "Arg #1 must be string!")
    assert(value ~= self,
        "Adding the script context to itself would be silly!")
    local a, b = type(value)
    assert(a == "addon" and
        b == "utility", "Arg #2 must be addon utility!")
    return add("utility", key, value)
end
function ScriptContext:DelUtility(key)
    assert(type(key) == "string", "Arg #1 must be string!")
    return del("utility", key)
end
function ScriptContext:GetUtility(key)
    assert(type(key) == "string", "Arg #1 must be string!")
    return get("utility", key)
end
