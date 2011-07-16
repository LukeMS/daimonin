-------------------------------------------------------------------------------
-- guilds.lua | module to handle guilds.
-------------------------------------------------------------------------------
---------------------------------------
-- module_guildsLOADED can be checked to see if the module has previously
-- been loaded. If true, it has. If nil, it has not.
---------------------------------------
module_guildsLOADED = true

---------------------------------------
-- _guilds{} contains details about all guilds in the game.
-- name is the guild name.
-- primary, secondary, and tertiary are the base_skill_groups for members of
-- that guild (when the player gets exp by being in a group but himself did
-- nothing, the exp is shared between his skill groups in the percentages given
-- in value).
-- ranks is a table of rank names within a guild (eg, Member, Acolyte, Fellow,
-- Lieutenant, Grand Poobah, and so on).
-- dues will contain information on how much guild members must contribute to
-- the guild, and how regularly payments should be made, and so on.
-- rejoinable is whether or not this guild allows ex-members to rejoin.
-- excluded.is is a table of numbers (elements of other guilds in this table),
-- current members of which are excluded from joining this guild, and
-- excluded.was is the same but members past and current are excluded from
-- joining.
-- TODO: Obviously most of it. For example, excluded and ranks are undeveloped
-- and all (both!) guilds are rejoinable ATM).
---------------------------------------
local _guilds = {
    [1] = {
        name = "Mercenary",
        primary = {
            group = game.SKILLGROUP_PHYSIQUE,
            value = 40,
        },
        secondary = {
            group = game.SKILLGROUP_AGILITY,
            value = 25,
        },
        tertiary = {
            group = game.SKILLGROUP_WISDOM,
            value = 15,
        },
        ranks = nil,
        rejoinable = true,
        dues = nil,
        excluded = {
            is = nil,
            was = nil,
        },
    },
    [2] = {
        name = "Wizard",
        primary = {
            group = game.SKILLGROUP_MAGIC,
            value = 50,
        },
        secondary = {
            group = game.SKILLGROUP_MENTAL,
            value = 30,
        },
        tertiary = {
            group = -1,
            value = 0,
        },
        ranks = nil,
        dues = nil,
        rejoinable = true,
        excluded = {
            is = nil,
            was = nil,
        },
    },
}

---------------------------------------
-- INTERNAL FUNCTIONS: These functions are ONLY to be used within the module
-- (indeed, being local they are not even visible externally). They are written
-- more for speed of execution than for user-friendliness, so there are no
-- argument checks for example -- valid values are assumed to have been passed.
-- Moreover, all functions other than _isvalid(), will only accept a number for
-- the guilds argument (which is the element number in _guilds{}).
---------------------------------------
-------------------
-- _isvalid() checks that guild is a valid guild in Daimonin and
-- that it is correctly mapped in the table above, generating an error if
-- either test fails.
-- On success it returns the guild number (1-n) if guild is a string or the
-- guild name if guild is a number.
-- guild is the name or number of the guild in question.
-------------------
local function _isvalid(guild)
    local nr

    if type(guild) == "number" then
        nr = guild
    else
        for i, v in ipairs(_guilds) do
            if game:MatchString(v.name, guild) then
                nr = i

                break
            end
        end
    end

    assert(nr >= 1 and
           nr <= table.getn(_guilds), "Unrecognised guild: " .. guild .. "!")

    return nr
end

-------------------
-- _getstatus() returns the status of player in guild.
-- guild is the number of the guild in question.
-- player is the player in question.
-- The first return is one of game.GUILD_NO, game.GUILD_OLD, or game.GUILD_IN.
-- The second return is the guild force (if player is not a member this will be
-- nil).
-------------------
local function _getstatus(guild, player)
    local force = player:GetGuild(_guilds[guild].name)
    local status = game.GUILD_NO

    if force then
        status = force.sub_type_1
    end

    return status, force
end

-------------------
-- _isexcluded() returns true or false to indicate whether player is excluded
-- from joining guild due to his current or past membership of other guilds.
-- guild is the number of the guild in question.
-- player is the player in question.
-------------------
local function _isexcluded(guild, player)
    if type(_guilds[guild].excluded) == "table" then
        if type(_guilds[guild].excluded.is) == "table" then
            for i, v in ipairs(_guilds[guild].excluded.is) do
                if _getstatus(v, player) == game.GUILD_IN then
                    return true
                end
            end
        end

        if type(_guilds[guild].excluded.was) == "table" then
            for i, v in ipairs(_guilds[guild].excluded.was) do
                if _getstatus(v, player) ~= game.GUILD_NO then
                    return true
                end
            end
        end
    end

    return false
end

---------------------------------------
-- EXTERNAL FUNCTIONS: These functions are available to scripts which require
-- this module. They are writteni to be user-friendly, checking argument types
-- and validating guilds for example.
-- Much of the 'low level' work common to multiple functions, is done by an
-- internal function (for example, they all call _isvalid()).
---------------------------------------
-------------------
-- module_guildsGetNameOrNumber() returns the name or number of guild in
-- _guilds{}.
-- guild is the name or number of the guild in question.
-------------------
function module_guildsGetNameOrNumber(guild)
    assert(type(guild) == "number" or
           type(guild) == "string",
           "Arg #1 must be number or string!")

    local nr = _isvalid(guild)

    if type(guild) == "number" then
        return _guilds[nr].name
    else
        return nr
    end
end

-------------------
-- module_guildsGetStatus() returns the membership status of player within
-- guild. The status is simply whether he is, used to be, or never has been a
-- guild member. It is not his rank within the guild.
-- guild is the name or number of the guild in question.
-- player is the player in question.
-- The return is one of game.GUILD_NO, game.GUILD_OLD, or game.GUILD_IN.
-------------------
function module_guildsGetStatus(guild, player)
    assert(type(guild) == "number" or
           type(guild) == "string",
           "Arg #1 must be number or string!")
    assert(type(player) == "GameObject" and
           player.type == game.TYPE_PLAYER,
           "Arg #2 must be player GameObject!")

    local nr = _isvalid(guild)
    local status = _getstatus(nr, player)

    return status
end

-------------------
-- module_guildsIsRejoinable() returns true or false according to whether guild
-- is rejoinable.
-- guild is the name or number of the guild in question.
-------------------
function module_guildsIsRejoinable(guild)
    assert(type(guild) == "number" or
           type(guild) == "string",
           "Arg #1 must be number or string!")

    local nr = _isvalid(guild)

    return _quests[nr].rejoinable == true
end

-------------------
-- module_guildsIsExcluded() returns true or false according to whether player
-- is excluded from joining guild because of other guild memberships, past or
-- present.
-- guild is the name or number of the guild in question.
-- player is the player in question.
-------------------
function module_guildsIsExcluded(guild, player)
    assert(type(guild) == "number" or
           type(guild) == "string",
           "Arg #1 must be number or string!")
    assert(type(player) == "GameObject" and
           player.type == game.TYPE_PLAYER,
           "Arg #2 must be player GameObject!")

    local nr = _isvalid(guild)

    return _isexcluded(nr, player)
end

-------------------
-- module_guildsJoin() attempts to make player a member of guild, returning
-- nil or a boolean to indicate whether it was successful.
-- guild is the name or number of the guild in question.
-- player is the player in question.
-- The return is nil if player is already a member (nothing happens, it was
-- pretty pointless calling this function), false if player was disallowed from
-- joining (could mean guild does not allow old members to rejoin, player is
-- excluded from joining this guild because of his current membership of a
-- conflicting guild, or player is individually banned for some reason), or
-- true if player was successful (and the joining takes place).
-------------------
function module_guildsJoin(guild, player)
    assert(type(guild) == "number" or
           type(guild) == "string",
           "Arg #1 must be number or string!")
    assert(type(player) == "GameObject" and
           player.type == game.TYPE_PLAYER,
           "Arg #2 must be player GameObject!")

    local nr = _isvalid(guild)
    local status = _getstatus(nr, player)

    if status == game.GUILD_IN then
        return nil
    elseif status == game.GUILD_OLD and
           not _guilds[nr].rejoinable then
        return false
    elseif _isexcluded(nr, player) then
        return false
    ---------
    -- TODO: check if player is temp/perm banned from guild.
    ---------
    end

    player:JoinGuild(_guilds[nr].name,
                     _guilds[nr].primary.group, _guilds[nr].primary.value,
                     _guilds[nr].secondary.group, _guilds[nr].secondary.value,
                     _guilds[nr].tertiary.group, _guilds[nr].tertiary.value)
    player:Write("You join the " .. _guilds[nr].name .. " Guild!")

    return true
end

-------------------
-- module_guildsLeave() makes player leave guild, returning boolean to
-- indicate success.
-- guild is the name or number of the guild in question.
-- player is the player in question.
-------------------
function module_guildsLeave(guild, player)
    assert(type(guild) == "number" or
           type(guild) == "string",
           "Arg #1 must be number or string!")
    assert(type(player) == "GameObject" and
           player.type == game.TYPE_PLAYER,
           "Arg #2 must be player GameObject!")

    local nr = _isvalid(guild)
    local status = _getstatus(nr, player)

    if status ~= game.GUILD_IN then
        return false
    end

    player:LeaveGuild() --_guilds[nr].name)
    player:Write("You leave the " .. _guilds[nr].name .. " Guild!")

    return true
end
