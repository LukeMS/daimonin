--
-- Proof of concept script of coroutines
-- A kickball that can be kicked around until it stops.
-- Then apply the ball to get it running again
--
local me = event.me

local max_speed = 9    -- squares / s
local deceleration = 2 -- squares / s / s

local dx = { 
	[0] = 0, 
	[1] = 0, [2] =  1, [3] =  1, [4] =  1, 
	[5] = 0, [6] = -1, [7] = -1, [8] = -1 }

local dy = { 
	[0] = 0, 
	[1] = -1, [2] = -1, [3] = 0, [4] = 1, 
	[5] =  1, [6] =  1, [7] = 0, [8] = -1 }

function getPushDirection()
	obj = me.map:GetFirstObjectOnSquare(me.x, me.y)
	while obj ~= nil do
		if obj.type == game.TYPE_PLAYER then
			return obj.direction
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

function getBounceItem(map,x,y)
	obj = map:GetFirstObjectOnSquare(x, y)
	while obj ~= nil do
		if obj ~= me then
			return obj.direction
		end		
		obj = obj.above
	end
	return 0
end

function run()
	local direction = getPushDirection()
	local speed = max_speed -- squares per second
	local nx, ny
	
	me.map:Message(me.x, me.y , 2, event.activator.name .. " kicks off!")
	me.terrain_flag = 255 -- lets us move anywhere
	event.returnvalue = 1 -- tell engine that apply was successful

	function bounce(dir)
		speed = speed - (1 / speed) * deceleration * 1
		-- TODO bounce more naturally off things
		return absDirection(dir + 4 + math.random(-1,1) * math.random(0, 1))
	end

	while game.IsValid(me) and speed > 0 do
		pushdir = getPushDirection()
		if pushdir ~= 0 then 
			direction = pushdir 
			-- TODO: Change visible item direction too
			speed = max_speed
		else
			nx = me.x + dx[direction]
			ny = me.y + dy[direction]

			if me.map:IsWallOnSquare(nx, ny) then direction = bounce(direction) end
		end
		
		if me:Move(direction) == 0 then direction = bounce(direction) end

		coroutine.yield(1/speed)
		speed = speed - (1 / speed) * deceleration		
	end
end

-- TODO: store playing state somehow
playing = false

if not playing then
	run()
end
