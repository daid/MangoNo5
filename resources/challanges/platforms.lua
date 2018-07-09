
size = irandom(28, 100)

x = 3
y = 5
while x < size do
    width = irandom(3, 7)
    new_y = irandom(4, 12)
    if new_y > y + 4 then new_y = y + 4 end
    y = new_y
    print(x, y)
    for n=0,width-1 do
        setTile(x+n, y, tile_grassMid)
    end
    x = x + width + irandom(5, 8)
end
exit(size + width, 0)

function run()
    while true do
        pos = getCameraPosition()
        pos.x = pos.x + 0.1
        if pos.x >= size + width - 14 then return end
        setCameraPosition(pos)
        yield()
    end
end
