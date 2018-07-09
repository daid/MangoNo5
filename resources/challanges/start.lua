
delay = 120

for x=0,3 do
    for y=0,20 do
        setTile(x, y, tile_grassCenter)
    end
end
for x=4,20 do
    setTile(x, 6, tile_fence, 1)
    setTile(x, 5, tile_grassMid)
    for y=0,4 do
        setTile(x, y, tile_grassCenter)
    end
end
setTile(20, 6, tile_fenceBroken, 1)
setTile(21, 5, tile_grassCliffRight)
exit(28, 0)

function run()
    while delay > 0 do
        delay = delay - 1
        yield()
    end
end
