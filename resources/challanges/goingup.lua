
for x=2,23 do
    setTile(x, 3, tile_fence, 1)
    setTile(x, 2, tile_grassMid)
    for y=0,1 do
        setTile(x, y, tile_grassCenter)
    end
end
height = irandom(30, 60)
local x = irandom(1, 22)
for n=4,height+10,2 do
    setTile(x, n, tile_sandCliffLeft)
    setTile(x+1, n, tile_sandMid)
    setTile(x+2, n, tile_sandCliffRight)
    x = ((x + irandom(5, 10)) % 22) + 1
end
exit(28, height)

function run()
    pos = getCameraPosition()
    while pos.x < 14 do
        pos = getCameraPosition()
        pos.x = pos.x + 0.1
        setCameraPosition(pos)
        yield()
    end
    for delay=1,30 do
        yield()
    end
    while pos.y < height + 10 do
        pos = getCameraPosition()
        pos.y = pos.y + 0.05
        setCameraPosition(pos)
        yield()
    end
    for delay=1,30 do
        yield()
    end
end

