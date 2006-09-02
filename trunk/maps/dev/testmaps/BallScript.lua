-- Example script to demonstrate coroutines
--
-- A ball that can be kicked around until it stops.
-- Then apply the ball to get it running again
--
-- Shows a few issues ideas of coroutines:
--  1. Need to store the state to avoid "double" execution
--  2. Need to validate object and map pointers after every yield

local me = event.me -- Use a local for shorter and quicker access

local max_speed = 9    -- squares / s (TODO: should actually be 8. Need to fix?)
local deceleration = 2 -- squares / s / s

local dx = {
    [0] = 0,
    [1] = 0, [2] =  1, [3] =  1, [4] =  1,
    [5] = 0, [6] = -1, [7] = -1, [8] = -1 
}

local dy = {
    [0] = 0,
    [1] = -1, [2] = -1, [3] = 0, [4] = 1,
    [5] =  1, [6] =  1, [7] = 0, [8] = -1 
}

function getPushDirection()
    local obj = me.map:GetFirstObjectOnSquare(me.x, me.y)
    while obj ~= nil do
        if obj.type == game.TYPE_PLAYER then
            return obj.facing
        end
        obj = obj.above
    end
    return 0
end

function absDirection(dir)
    while dir < 1 do dir = dir + 8 end
    while dir > 8 do dir = dir - 8 end
    return dir
end

--function getBounceItem(map,x,y)
--	local obj = map:GetFirstObjectOnSquare(x, y)
--	while obj ~= nil do
--		if obj ~= me then
--			return obj.direction
--		end
--		obj = obj.above
--	end
--	return 0
--end

function run()
    local direction = 0
    local speed = max_speed -- squares per second

    me.map:Message(me.x, me.y, 2, event.activator.name .. " kicks off!")
    me.terrain_flag = 255 -- lets us move anywhere
    me.grace = 1

    function bounce(dir)
        speed = speed - (1 / speed) * deceleration * 1
        -- TODO bounce more naturally off things
        return absDirection(dir + 4 + math.random(-1,1) * math.random(0, 1))
    end

    while game:IsValid(me) and speed > 0 do
        -- Look for player pushing us
        pushdir = getPushDirection()
        if pushdir ~= 0 then
            direction = pushdir
            -- TODO: Change visible item direction too
            speed = max_speed
        elseif me.map:IsWallOnSquare(me.x + dx[direction], me.y + dy[direction]) then
            direction = bounce(direction)
        end

        -- Move, or bounce if that fails
        if me:Move(direction) == 0 then direction = bounce(direction) end

        if(speed > 0) then
            coroutine.yield(1/speed)
            speed = speed - (1 / speed) * deceleration
        end
    end
    -- If we still have an object, mark it as stopped.
    if game:IsValid(me) then me.grace = 0 end
end

-- State is stored in me.grace (because that field isn't used for
-- anything else _in this case_ )
-- 0 - means stopped
-- 1 - means rolling

event.returnvalue = 1 -- tell Daimonin the apply was successful
if me.grace == 0 then
    -- Initial kickoff
    run()
end
