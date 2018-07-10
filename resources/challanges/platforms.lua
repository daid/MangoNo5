
size = irandom(28, 100)

function platform(x, y, w)
    for n=0,width-1 do
        setTile(x+n, y, tile_grassMid)
    end
    if width == 1 then
        setTile(x, y, tile_grass)
    else
        setTile(x, y, tile_grassCliffLeft)
        setTile(x+width-1, y, tile_grassCliffRight)
    end
    
    if random(0, 100) < 30 then
        top = irandom(y+5, 20)
        if random(0, 100) < 70 and width > 3 then
            for n=y+1,top do
                setTile(x+1, n, tile_ropeVertical, 1)
                setTile(x+width-2, n, tile_ropeVertical, 1)
            end
            setTile(x+1, top, tile_ropeAttached, 1)
            setTile(x+width-2, top, tile_ropeAttached, 1)
        else
            for n=y+1,top do
                setTile(x+math.floor((width-1)/2), n, tile_ropeVertical, 1)
            end
            setTile(x+math.floor((width-1)/2), top, tile_ropeAttached, 1)
        end
    end
end

x = 3
y = 5
while x < size do
    width = irandom(3, 7)
    new_y = irandom(4, 12)
    if new_y > y + 4 then new_y = y + 4 end
    y = new_y
    platform(x, y, width)
    x = x + width + irandom(5, 7)
end
size = size + width
exit(size, 0)

for extra=1,irandom(1,3) do
    width = irandom(1, 5)
    x = irandom(0, size - width)
    y = irandom(4, 12)
    platform(x, y, width)
end

function run()
    while true do
        pos = getCameraPosition()
        pos.x = pos.x + 0.1
        if pos.x >= size - 14 then return end
        setCameraPosition(pos)
        yield()
    end
end
