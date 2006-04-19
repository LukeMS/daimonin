-- Prototype of flocking or pack behaviour
-- Will make mobs move towards a virtual "center of friendly mobs"
local me = event.me
local ai = me:GetAI()

-- Here's our parameters
local max_confidence = 1000 -- TODO actually use for something
local min_friendship = 100  -- Who do we consider to be friends

-- Get a list of all friends and their positions
local friends = {}
local friendship_max = 0
local distance_max = 0
for i,obj in pairs(ai:GetKnownMobs()) do
    local friendship = ai:GetFriendship(obj)
    local dist, dir, dx, dy = me:GetVector(obj)

    if dist ~= nil and friendship >= min_friendship then
        friends[obj] = {
            friendship = friendship,
            dist = dist,
            dx = dx,
            dy = dy
        }

        if friendship > friendship_max then
            friendship_max = friendship
        end

        if dist > distance_max then
            distance_max = dist
        end

--        print (obj.name .. " friendship: " .. friendship .. ", dist=".. tostring(dist))
    end
end

-- if table.getn(friends) == 0 then return end

-- Calculate average friendly mob position weighted by friendship
local dx, dy, weights = 0,0,0

for obj, data in pairs(friends) do
    -- Test on how to make a "spring-like" force from the center
    --    local weight = (data['friendship'] / friendship_max) * (distance_max / data['dist'])
    local weight = data['friendship'] / friendship_max
    dx = dx + data['dx'] * weight
    dy = dy + data['dy'] * weight
    weights = weights + weight
    --    print (obj.name .. " weight=".. weight)
end
dx = math.floor(dx / weights + 0.5)
dy = math.floor(dy / weights + 0.5)

ai:MoveRespondCoordinate(me.map, me.x + dx, me.y + dy)
