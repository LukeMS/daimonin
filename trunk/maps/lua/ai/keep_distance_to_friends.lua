local me = event.me
local ai = me:GetAI()

-- Those should really be parameters
local min_distance = 2
local min_friendship = 100
local max_friendship = 100000

function absdir(dir)
    while dir < 1 do dir=dir+8 end
    while dir > 8 do dir=dir-8 end
    return dir
end

-- Get a list of all friends and their positions
local friends = {}
local friendship_max = 0
for i,obj in pairs(ai:GetKnownMobs()) do
    local friendship = ai:GetFriendship(obj)
    local dist, dir, dx, dy = me:GetVector(obj)

    if dist ~= nil and friendship >= min_friendship and friendship <= max_friendship then
        if dist <= min_distance then
            ai:ForbidMoveDirection(dir)
            if dist < min_distance then
                ai:ForbidMoveDirection(absdir(dir-1))
                ai:ForbidMoveDirection(absdir(dir+1))
                ai:ForbidMoveDirection(0)
            end
        end
    end
end
