
for x=2,23 do
    setTile(x, 3, tile_fence, 1)
    setTile(x, 2, tile_grassMid)
    setTile(x, 18, tile_grassMid)
    for y=0,1 do
        setTile(x, y, tile_grassCenter)
    end
end
x = irandom(6, 20)
for y=0,3 do
    setTile(x, y, -1, 1)
    setTile(x+1, y, -1, 1)
end
x = irandom(6, 20)
for y=18,18 do
    setTile(x, y, -1, 1)
    setTile(x+1, y, -1, 1)
end

for n=1,10 do
    setTile(irandom(5, 22), irandom(3, 17), tile_grassMid)
end

exit(28, 0)

function run()
    pos = getCameraPosition()
    while pos.x < 14 do
        pos = getCameraPosition()
        pos.x = pos.x + 0.1
        setCameraPosition(pos)
        yield()
    end

    for y=6,14 do
        setTile(4, y, tile_castleCenter)
        setTile(23, y, tile_castleCenter)
    end

    r = 0
    while r < 360 * 2 do
        setTilemapRotation({14, 10}, r)
        r = r + 0.4
        yield()
    end
    setTilemapRotation({14, 10}, 0)

    for y=6,14 do
        setTile(4, y, -1, 1)
        setTile(23, y, -1, 1)
    end
end
