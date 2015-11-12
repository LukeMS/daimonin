-------------------------------------------------------------------------------
-- guilds.lua | module to handle guilds.
-------------------------------------------------------------------------------
---------------------------------------
-- module_guildsLOADED can be checked to see if the module has previously
-- been loaded. If true, it has. If nil, it has not.
---------------------------------------
module_guildsLOADED = true
---------------------------------------
---------------------------------------
-- Use bitmasks for flags such as no_archery and no_magic.
---------------------------------------
require "bitmasks"

local F_NO_MAGIC = 1
local F_NO_PRAYER = 2
local F_NO_ARCHERY = 4
local F_NO_2H = 8
local F_NO_POLEARM = 16

---------------------------------------
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
-- and all guilds are rejoinable ATM).
---------------------------------------------------
_guilds = {
    [1] = {
        name = "Mercenary",
        primary = {
            group = game.SKILLGROUP_PHYSIQUE,
            value = 55,
        },
        secondary = {
            group = game.SKILLGROUP_AGILITY,
            value = 45,
        },
        tertiary = {
            group = -1,
            value = 0,
        },
        ranks = nil,
        rejoinable = true,
        dues = nil,
        excluded = {
            is = nil,
            was = nil,
        },
        stats = {
            Str = 3,
            Con = 3,
            Dex = 3,
            Int = -2,
            Pow = -3,
            Wis = -3,
            wc  = 3,
            dam = 15,
            hp = 20,
            sp = -10,
            grace = -10,
            resists = {
                slash = 5,
                impact = 5,
                cleave = 5,
                pierce = 5,
            },
            weapon_speed = 0.30,
            attacks = {
            },
            path_attuned = 0,
        },

        flags = Byte(8, {F_NO_MAGIC, F_NO_PRAYER}),
        weapon_max_level = 100,
    },
    [2] = {
        name = "Wizard",
        primary = {
            group = game.SKILLGROUP_MAGIC,
            value = 65,
        },
        secondary = {
            group = game.SKILLGROUP_MENTAL,
            value = 35,
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
        stats = {
            Int = 3,
            Pow = 3,
            Str = -3,
            Con = -3,
            sp = 10,
            maxsp = 30,
            hp = -5,
            grace = -5,
            resists = {
                cold = 5,
                fire = 5,
                electricity = 5,
                poison = 5,
                acid = 5,
            },
            attacks = {
            },
            path_attuned = 4,
        },
        flags = Byte(8, {F_NO_2H, F_NO_POLEARM, F_NO_PRAYER, F_NO_ARCHERY}),
        spell_max_difficulty = 2,
    },
    [3] = {
        name = "Priest",
        primary = {
            group = game.SKILLGROUP_WISDOM,
            value = 80,
        },
        secondary = {
            group = game.SKILLGROUP_PHYSIQUE,
            value = 20,
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
        stats = {
            Con = 2,
            Wis = 3,
            Cha = 2,
            Dex = -2,
            Int = -3,
            Pow = -5,
            grace = 15,
            maxgrace = 30,
            sp = -10,
            resists = {
                drain = 10,
                depletion = 10,
                corruption = 10,
            },
            attacks = {
                godpower = 5,
            },
            path_attuned = 3,
        },
        flags = Byte(8, {F_NO_ARCHERY, F_NO_MAGIC, F_NO_2H, F_NO_POLEARM}),
        weapon_max_level = 10,
        spell_max_difficulty = 2, -- TODO: Add a prayer max difficulty.
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

local function _GetGuildForce(guild, pl)
    for obj in obj_inventory(pl) do
        if obj.name == "GUILD_FORCE" and obj.f_sys_object then
            if obj.slaying == guild then
            -- We have confirmed that this obj is the guild force (or a clever impostor).
            -- So return it!
                return obj
            end
        end
    end
end
local function _AddGuildStats(guild, player)
-- guild is the name of the guild.
    local guildnr = _isvalid(guild)
    local guild_force = _GetGuildForce(guild, player)
    -- Go through all stats and change them appropriately.
if _guilds[guildnr].stats.Str ~= nil then guild_force.strength =  _guilds[guildnr].stats.Str end
if _guilds[guildnr].stats.Dex ~= nil then guild_force.dexterity          = _guilds[guildnr].stats.Dex end
if _guilds[guildnr].stats.Con ~= nil then guild_force.constitution       = _guilds[guildnr].stats.Con end
if _guilds[guildnr].stats.Int ~= nil then guild_force.intelligence       = _guilds[guildnr].stats.Int end
if _guilds[guildnr].stats.Wis ~= nil then guild_force.wisdom             = _guilds[guildnr].stats.Wis end
if _guilds[guildnr].stats.Pow ~= nil then guild_force.power              = _guilds[guildnr].stats.Pow end
if _guilds[guildnr].stats.Cha ~= nil then guild_force.charisma           = _guilds[guildnr].stats.Cha end
if _guilds[guildnr].stats.ac ~= nil then guild_force.armour_class       = _guilds[guildnr].stats.ac end
if _guilds[guildnr].stats.wc ~= nil then guild_force.weapon_class       = _guilds[guildnr].stats.wc end
if _guilds[guildnr].stats.dam ~= nil then guild_force.damage             = _guilds[guildnr].stats.dam end
if _guilds[guildnr].stats.thac0 ~= nil then    guild_force.thac0              = _guilds[guildnr].stats.thac0 end
if _guilds[guildnr].stats.thacm ~= nil then    guild_force.thacm        = _guilds[guildnr].stats.thacm end
if _guilds[guildnr].stats.weapon_speed ~= nil then    guild_force.weapon_speed = _guilds[guildnr].stats.weapon_speed end
if _guilds[guildnr].stats.maxhp ~= nil then    guild_force.max_hitpoints        = _guilds[guildnr].stats.maxhp end
if _guilds[guildnr].stats.hp ~= nil then    guild_force.hitpoints           = _guilds[guildnr].stats.hp end
if _guilds[guildnr].stats.maxsp ~= nil then    guild_force.max_spellpoints        = _guilds[guildnr].stats.maxsp end
if _guilds[guildnr].stats.sp ~= nil then    guild_force.spellpoints           = _guilds[guildnr].stats.sp end
if _guilds[guildnr].stats.maxgrace ~= nil then    guild_force.max_grace     = _guilds[guildnr].stats.maxgrace end
if _guilds[guildnr].stats.grace ~= nil then    guild_force.grace        = _guilds[guildnr].stats.grace end
if _guilds[guildnr].weapon_max_level ~= nil then guild_force.value        = _guilds[guildnr].weapon_max_level end
if _guilds[guildnr].spell_max_difficulty ~= nil then guild_force.level        = _guilds[guildnr].spell_max_difficulty end
if _guilds[guildnr].flags ~= nil then guild_force.weight_limit = _guilds[guildnr].flags:ToNumber() end
if _guilds[guildnr].stats.resists.impact ~= nil then    guild_force.resist_impact= _guilds[guildnr].stats.resists.impact end
if _guilds[guildnr].stats.resists.cleave ~= nil then    guild_force.resist_cleave= _guilds[guildnr].stats.resists.cleave end
if _guilds[guildnr].stats.resists.slash ~= nil then    guild_force.resist_slash = _guilds[guildnr].stats.resists.slash end
if _guilds[guildnr].stats.resists.pierce ~= nil then    guild_force.resist_pierce= _guilds[guildnr].stats.resists.pierce end
if _guilds[guildnr].stats.resists.cold ~= nil then    guild_force.resist_cold  = _guilds[guildnr].stats.resists.cold end
if _guilds[guildnr].stats.resists.fire ~= nil then    guild_force.resist_fire  = _guilds[guildnr].stats.resists.fire end
if _guilds[guildnr].stats.resists.electricity ~= nil then    guild_force.resist_electricity = _guilds[guildnr].stats.resists.electricity end
if _guilds[guildnr].stats.resists.poison ~= nil then    guild_force.resist_poison= _guilds[guildnr].stats.resists.poison end
if _guilds[guildnr].stats.resists.acid ~= nil then    guild_force.resist_acid  = _guilds[guildnr].stats.resists.acid end
if _guilds[guildnr].stats.resists.sonic ~= nil then    guild_force.resist_sonic = _guilds[guildnr].stats.resists.sonic end
if _guilds[guildnr].stats.resists.channelling ~= nil then    guild_force.resist_channelling = _guilds[guildnr].stats.resists.channelling end
if _guilds[guildnr].stats.resists.corruption ~= nil then    guild_force.resist_corruption = _guilds[guildnr].stats.resists.corruption end
if _guilds[guildnr].stats.resists.psionic ~= nil then    guild_force.resist_psionic = _guilds[guildnr].stats.resists.psionic end
if _guilds[guildnr].stats.resists.light ~= nil then    guild_force.resist_light = _guilds[guildnr].stats.resists.light end
if _guilds[guildnr].stats.resists.shadow ~= nil then    guild_force.resist_shadow= _guilds[guildnr].stats.resists.shadow end
if _guilds[guildnr].stats.resists.lifesteal ~= nil then    guild_force.resist_lifesteal = _guilds[guildnr].stats.resists.lifesteal end
if _guilds[guildnr].stats.resists.aether ~= nil then    guild_force.resist_aether = _guilds[guildnr].stats.resists.aether end
if _guilds[guildnr].stats.resists.nether ~= nil then    guild_force.resist_nether = _guilds[guildnr].stats.resists.nether end
if _guilds[guildnr].stats.resists.chaos ~= nil then    guild_force.resist_chaos = _guilds[guildnr].stats.resists.chaos end
if _guilds[guildnr].stats.resists.death ~= nil then    guild_force.resist_death = _guilds[guildnr].stats.resists.death end
if _guilds[guildnr].stats.resists.weaponmagic ~= nil then    guild_force.resist_weaponmagic = _guilds[guildnr].stats.resists.weaponmagic end
if _guilds[guildnr].stats.resists.godpower ~= nil then    guild_force.resist_godpower = _guilds[guildnr].stats.resists.godpower end
if _guilds[guildnr].stats.resists.drain ~= nil then    guild_force.resist_drain = _guilds[guildnr].stats.resists.drain end
if _guilds[guildnr].stats.resists.depletion ~= nil then    guild_force.resist_depletion = _guilds[guildnr].stats.resists.depletion end
if _guilds[guildnr].stats.resists.countermagic ~= nil then    guild_force.resist_countermagic = _guilds[guildnr].stats.resists.countermagic end
if _guilds[guildnr].stats.resists.cancellation ~= nil then    guild_force.resist_cancellation = _guilds[guildnr].stats.resists.cancellation end
if _guilds[guildnr].stats.resists.confusion ~= nil then    guild_force.resist_confusion = _guilds[guildnr].stats.resists.confusion end
if _guilds[guildnr].stats.resists.fear ~= nil then    guild_force.resist_fear = _guilds[guildnr].stats.resists.fear end
if _guilds[guildnr].stats.resists.slow ~= nil then    guild_force.resist_slow = _guilds[guildnr].stats.resists.slow end
if _guilds[guildnr].stats.resists.paralyze ~= nil then    guild_force.resist_paralyze = _guilds[guildnr].stats.resists.paralyze end
if _guilds[guildnr].stats.resists.snare ~= nil then    guild_force.resist_snare = _guilds[guildnr].stats.resists.snare end
if _guilds[guildnr].stats.attacks.impact ~= nil then    guild_force.attack_impact= _guilds[guildnr].stats.attacks.impact end
if _guilds[guildnr].stats.attacks.cleave ~= nil then    guild_force.attack_cleave= _guilds[guildnr].stats.attacks.cleave end
if _guilds[guildnr].stats.attacks.slash ~= nil then    guild_force.attack_slash = _guilds[guildnr].stats.attacks.slash end
if _guilds[guildnr].stats.attacks.pierce ~= nil then    guild_force.attack_pierce= _guilds[guildnr].stats.attacks.pierce end
if _guilds[guildnr].stats.attacks.cold ~= nil then    guild_force.attack_cold  = _guilds[guildnr].stats.attacks.cold end
if _guilds[guildnr].stats.attacks.fire ~= nil then    guild_force.attack_fire  = _guilds[guildnr].stats.attacks.fire end
if _guilds[guildnr].stats.attacks.electricity ~= nil then    guild_force.attack_electricity = _guilds[guildnr].stats.attacks.electricity end
if _guilds[guildnr].stats.attacks.poison ~= nil then    guild_force.attack_poison= _guilds[guildnr].stats.attacks.poison end
if _guilds[guildnr].stats.attacks.acid ~= nil then    guild_force.attack_acid  = _guilds[guildnr].stats.attacks.acid end
if _guilds[guildnr].stats.attacks.sonic ~= nil then    guild_force.attack_sonic = _guilds[guildnr].stats.attacks.sonic end
if _guilds[guildnr].stats.attacks.channelling ~= nil then    guild_force.attack_channelling = _guilds[guildnr].stats.attacks.channelling end
if _guilds[guildnr].stats.attacks.corruption ~= nil then    guild_force.attack_corruption = _guilds[guildnr].stats.attacks.corruption end
if _guilds[guildnr].stats.attacks.psionic ~= nil then    guild_force.attack_psionic = _guilds[guildnr].stats.attacks.psionic end
if _guilds[guildnr].stats.attacks.light ~= nil then    guild_force.attack_light = _guilds[guildnr].stats.attacks.light end
if _guilds[guildnr].stats.attacks.shadow ~= nil then    guild_force.attack_shadow= _guilds[guildnr].stats.attacks.shadow end
if _guilds[guildnr].stats.attacks.lifesteal ~= nil then    guild_force.attack_lifesteal = _guilds[guildnr].stats.attacks.lifesteal end
if _guilds[guildnr].stats.attacks.aether ~= nil then    guild_force.attack_aether = _guilds[guildnr].stats.attacks.aether end
if _guilds[guildnr].stats.attacks.nether ~= nil then    guild_force.attack_nether = _guilds[guildnr].stats.attacks.nether end
if _guilds[guildnr].stats.attacks.chaos ~= nil then    guild_force.attack_chaos = _guilds[guildnr].stats.attacks.chaos end
if _guilds[guildnr].stats.attacks.death ~= nil then    guild_force.attack_death = _guilds[guildnr].stats.attacks.death end
if _guilds[guildnr].stats.attacks.weaponmagic ~= nil then    guild_force.attack_weaponmagic = _guilds[guildnr].stats.attacks.weaponmagic end
if _guilds[guildnr].stats.attacks.godpower ~= nil then    guild_force.attack_godpower = _guilds[guildnr].stats.attacks.godpower end
if _guilds[guildnr].stats.attacks.drain ~= nil then    guild_force.attack_drain = _guilds[guildnr].stats.attacks.drain end
if _guilds[guildnr].stats.attacks.depletion ~= nil then    guild_force.attack_depletion = _guilds[guildnr].stats.attacks.depletion end
if _guilds[guildnr].stats.attacks.countermagic ~= nil then    guild_force.attack_countermagic = _guilds[guildnr].stats.attacks.countermagic end
if _guilds[guildnr].stats.attacks.cancellation ~= nil then    guild_force.attack_cancellation = _guilds[guildnr].stats.attacks.cancellation end
if _guilds[guildnr].stats.attacks.confusion ~= nil then    guild_force.attack_confusion = _guilds[guildnr].stats.attacks.confusion end
if _guilds[guildnr].stats.attacks.fear ~= nil then    guild_force.attack_fear = _guilds[guildnr].stats.attacks.fear end
if _guilds[guildnr].stats.attacks.slow ~= nil then    guild_force.attack_slow = _guilds[guildnr].stats.attacks.slow end
if _guilds[guildnr].stats.attacks.paralyze ~= nil then    guild_force.attack_paralyze = _guilds[guildnr].stats.attacks.paralyze end
if _guilds[guildnr].stats.attacks.snare ~= nil then    guild_force.attack_snare = _guilds[guildnr].stats.attacks.snare end
if _guilds[guildnr].stats.path_attuned ~= nil then    guild_force.path_attuned = _guilds[guildnr].stats.path_attuned end
    -- That wasn't fun. There has to be a better way.
end
local function _RemoveGuildStats(guild, player)
-- guild is the name of the guild.
    local guildnr = _isvalid(guild)
    local guild_force = _GetGuildForce(guild, player)
    -- Go through all stats and change them appropriately.

if _guilds[guildnr].stats.Str ~= nil then guild_force.strength =  0 end
if _guilds[guildnr].stats.Dex ~= nil then guild_force.dexterity          = 0 end
if _guilds[guildnr].stats.Con ~= nil then guild_force.constitution       = 0 end
if _guilds[guildnr].stats.Int ~= nil then guild_force.intelligence       = 0 end
if _guilds[guildnr].stats.Wis ~= nil then guild_force.wisdom             = 0 end
if _guilds[guildnr].stats.Pow ~= nil then guild_force.power              = 0 end
if _guilds[guildnr].stats.Cha ~= nil then guild_force.charisma           = 0 end
if _guilds[guildnr].stats.wc ~= nil then guild_force.armour_class       = 0 end
if _guilds[guildnr].stats.ac ~= nil then guild_force.weapon_class       = 0 end
if _guilds[guildnr].stats.dam ~= nil then guild_force.damage             = 0 end
if _guilds[guildnr].stats.thacnil ~= nil then    guild_force.thacnil              = 0 end
if _guilds[guildnr].stats.thacm ~= nil then    guild_force.thacm        = 0 end
if _guilds[guildnr].stats.weapon_speed ~= nil then    guild_force.weapon_speed = 0 end
if _guilds[guildnr].stats.maxhp ~= nil then    guild_force.max_hitpoints        = 0 end
if _guilds[guildnr].stats.hp ~= nil then    guild_force.hitpoints           = 0 end
if _guilds[guildnr].stats.maxsp ~= nil then    guild_force.max_spellpoints        = 0 end
if _guilds[guildnr].stats.sp ~= nil then    guild_force.spellpoints           = 0 end
if _guilds[guildnr].stats.maxgrace ~= nil then    guild_force.max_grace     = 0 end
if _guilds[guildnr].stats.grace ~= nil then    guild_force.grace        = 0 end
guild_force.value        = 0
guild_force.level        = 0
guild_force.weight_limit = 0
if _guilds[guildnr].stats.resists.impact ~= nil then    guild_force.resist_impact= 0 end
if _guilds[guildnr].stats.resists.cleave ~= nil then    guild_force.resist_cleave= 0 end
if _guilds[guildnr].stats.resists.slash ~= nil then    guild_force.resist_slash = 0 end
if _guilds[guildnr].stats.resists.pierce ~= nil then    guild_force.resist_pierce= 0 end
if _guilds[guildnr].stats.resists.cold ~= nil then    guild_force.resist_cold  = 0 end
if _guilds[guildnr].stats.resists.fire ~= nil then    guild_force.resist_fire  = 0 end
if _guilds[guildnr].stats.resists.electricity ~= nil then    guild_force.resist_electricity = 0 end
if _guilds[guildnr].stats.resists.poison ~= nil then    guild_force.resist_poison= 0 end
if _guilds[guildnr].stats.resists.acid ~= nil then    guild_force.resist_acid  = 0 end
if _guilds[guildnr].stats.resists.sonic ~= nil then    guild_force.resist_sonic = 0 end
if _guilds[guildnr].stats.resists.channelling ~= nil then    guild_force.resist_channelling = 0 end
if _guilds[guildnr].stats.resists.corruption ~= nil then    guild_force.resist_corruption = 0 end
if _guilds[guildnr].stats.resists.psionic ~= nil then    guild_force.resist_psionic = 0 end
if _guilds[guildnr].stats.resists.light ~= nil then    guild_force.resist_light = 0 end
if _guilds[guildnr].stats.resists.shadow ~= nil then    guild_force.resist_shadow= 0 end
if _guilds[guildnr].stats.resists.lifesteal ~= nil then    guild_force.resist_lifesteal = 0 end
if _guilds[guildnr].stats.resists.aether ~= nil then    guild_force.resist_aether = 0 end
if _guilds[guildnr].stats.resists.nether ~= nil then    guild_force.resist_nether = 0 end
if _guilds[guildnr].stats.resists.chaos ~= nil then    guild_force.resist_chaos = 0 end
if _guilds[guildnr].stats.resists.death ~= nil then    guild_force.resist_death = 0 end
if _guilds[guildnr].stats.resists.weaponmagic ~= nil then    guild_force.resist_weaponmagic = 0 end
if _guilds[guildnr].stats.resists.godpower ~= nil then    guild_force.resist_godpower = 0 end
if _guilds[guildnr].stats.resists.drain ~= nil then    guild_force.resist_drain = 0 end
if _guilds[guildnr].stats.resists.depletion ~= nil then    guild_force.resist_depletion = 0 end
if _guilds[guildnr].stats.resists.countermagic ~= nil then    guild_force.resist_countermagic = 0 end
if _guilds[guildnr].stats.resists.cancellation ~= nil then    guild_force.resist_cancellation =0 end
if _guilds[guildnr].stats.resists.confusion ~= nil then    guild_force.resist_confusion = 0 end
if _guilds[guildnr].stats.resists.fear ~= nil then    guild_force.resist_fear = 0 end
if _guilds[guildnr].stats.resists.slow ~= nil then    guild_force.resist_slow = 0 end
if _guilds[guildnr].stats.resists.paralyze ~= nil then    guild_force.resist_paralyze = 0 end
if _guilds[guildnr].stats.resists.snare ~= nil then    guild_force.resist_snare = 0 end
if _guilds[guildnr].stats.attacks.impact ~= nil then    guild_force.attack_impact= 0 end
if _guilds[guildnr].stats.attacks.cleave ~= nil then    guild_force.attack_cleave= 0 end
if _guilds[guildnr].stats.attacks.slash ~= nil then    guild_force.attack_slash = 0 end
if _guilds[guildnr].stats.attacks.pierce ~= nil then    guild_force.attack_pierce= 0 end
if _guilds[guildnr].stats.attacks.cold ~= nil then    guild_force.attack_cold  = 0 end
if _guilds[guildnr].stats.attacks.fire ~= nil then    guild_force.attack_fire  = 0 end
if _guilds[guildnr].stats.attacks.electricity ~= nil then    guild_force.attack_electricity = 0 end
if _guilds[guildnr].stats.attacks.poison ~= nil then    guild_force.attack_poison= 0 end
if _guilds[guildnr].stats.attacks.acid ~= nil then    guild_force.attack_acid  = 0 end
if _guilds[guildnr].stats.attacks.sonic ~= nil then    guild_force.attack_sonic = 0 end
if _guilds[guildnr].stats.attacks.channelling ~= nil then    guild_force.attack_channelling = 0 end
if _guilds[guildnr].stats.attacks.corruption ~= nil then    guild_force.attack_corruption = 0 end
if _guilds[guildnr].stats.attacks.psionic ~= nil then    guild_force.attack_psionic = 0 end
if _guilds[guildnr].stats.attacks.light ~= nil then    guild_force.attack_light = 0 end
if _guilds[guildnr].stats.attacks.shadow ~= nil then    guild_force.attack_shadow= 0 end
if _guilds[guildnr].stats.attacks.lifesteal ~= nil then    guild_force.attack_lifesteal = 0 end
if _guilds[guildnr].stats.attacks.aether ~= nil then    guild_force.attack_aether = 0 end
if _guilds[guildnr].stats.attacks.nether ~= nil then    guild_force.attack_nether = 0 end
if _guilds[guildnr].stats.attacks.chaos ~= nil then    guild_force.attack_chaos = 0 end
if _guilds[guildnr].stats.attacks.death ~= nil then    guild_force.attack_death = 0 end
if _guilds[guildnr].stats.attacks.weaponmagic ~= nil then    guild_force.attack_weaponmagic = 0 end
if _guilds[guildnr].stats.attacks.godpower ~= nil then    guild_force.attack_godpower = 0 end
if _guilds[guildnr].stats.attacks.drain ~= nil then    guild_force.attack_drain = 0 end
if _guilds[guildnr].stats.attacks.depletion ~= nil then    guild_force.attack_depletion = 0 end
if _guilds[guildnr].stats.attacks.countermagic ~= nil then    guild_force.attack_countermagic = 0 end
if _guilds[guildnr].stats.attacks.cancellation ~= nil then    guild_force.attack_cancellation = 0 end
if _guilds[guildnr].stats.attacks.confusion ~= nil then    guild_force.attack_confusion = 0 end
if _guilds[guildnr].stats.attacks.fear ~= nil then    guild_force.attack_fear = 0 end
if _guilds[guildnr].stats.attacks.slow ~= nil then    guild_force.attack_slow = 0 end
if _guilds[guildnr].stats.attacks.paralyze ~= nil then    guild_force.attack_paralyze = 0 end
if _guilds[guildnr].stats.attacks.snare ~= nil then    guild_force.attack_snare = 0 end
if _guilds[guildnr].stats.path_attuned ~= nil then    guild_force.path_attuned = 0 end
    -- That wasn't fun. There has to be a better way.
end
---------------------------------------
-- EXTERNAL FUNCTIONS: These functions are available to scripts which require
-- this module. They are written to be user-friendly, checking argument types
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
    -- local nr
    -- if type(guild) == "string" then
        -- nr = module_guildsGetNameOrNumber(guild)
    -- end 
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

function module_guildsPlayerGuildless(player)
-- See if the player is currently in a guild or not.
    assert(type(player) == "GameObject" and player.type == game.TYPE_PLAYER, "Arg #1 must be player GameObject!")
    for i=1, table.getn(_guilds) do
        if module_guildsGetStatus(_guilds[i].name, player) == game.GUILD_IN then return false end
    end
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
    _RemoveGuildStats(_guilds[nr].name, player)
    player:Write("You leave the " .. _guilds[nr].name .. " Guild!")
    player:LeaveGuild() --_guilds[nr].name)
    player:Fix()
    return true
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
    local CurGuild 
    for i=1, table.getn(_guilds) do
        if _getstatus(i, player) == game.GUILD_IN then CurGuild = i break end
    end
    if CurGuild ~= nil then
        module_guildsLeave(CurGuild, player)
    end
    player:Write("You join the " .. _guilds[nr].name .. " Guild!")
    player:JoinGuild(_guilds[nr].name,
                     _guilds[nr].primary.group, _guilds[nr].primary.value,
                     _guilds[nr].secondary.group, _guilds[nr].secondary.value,
                     _guilds[nr].tertiary.group, _guilds[nr].tertiary.value)
    -- Player should be aware just how restrictive the guild is. Then unapply
    -- any offending equipment. Does not break curses though!
    local flags = _guilds[nr].flags
    if flags ~= nil then
        if flags:Check(F_NO_MAGIC) then
            player:Write("Guild restrictions prevent casting spells!")
        end
        if flags:Check(F_NO_PRAYER) then
            player:Write("Guild restrictions prevent invoking prayers!")
        end
        if flags:Check(F_NO_2H) then
            player:Write("Guild restrictions prevent use of two-handed weapons!")
        end
        if flags:Check(F_NO_POLEARM) then
            player:Write("Guild restrictions prevent use of polearm weapons!")
        end
        if flags:Check(F_NO_ARCHERY) then
            player:Write("Guild restrictions prevent use of archery weapons!")
        end
        local weapon = player:GetEquipment(game.EQUIP_WEAPON1)
        if weapon then
            if weapon.sub_type_1 >= 4 and
                weapon.sub_type_1 <= 7 then
                if flags:Check(F_NO_2H) then
                    player:Apply(weapon, game.UNAPPLY_ALWAYS)
                end
            elseif weapon.sub_type_1 >= 8 and
                weapon.sub_type_1 <= 12 then
                if flags:Check(F_NO_POLEARM) then
                    player:Apply(weapon, game.UNAPPLY_ALWAYS)
                end
            end
        end
        local bow = player:GetEquipment(game.EQUIP_BOW)
        if bow then
            if flags:Check(F_NO_ARCHERY) then
                player:Apply(bow, game.UNAPPLY_ALWAYS)
            end
        end
    end
    _AddGuildStats(_guilds[nr].name, player)
    player:Fix()

    return true
end
