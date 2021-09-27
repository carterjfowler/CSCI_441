Carter Fowler - Aethon Misty

1. Cause they are always being drawn in eye space, specifically after the view matrix has been applied, so the quads are always drawn in relation to the camera.

2. Because we we drawing the images closest to the camera first, we didn't try to implement the painter's algorithm for this (yet).

3. The shader program is what tells the computer what colors to draw where, so we can very easily draw textures in the shaders as long as we have the right data from main.cpp.

4. Since we know none of these objects are going to become weirdly in front of and behind each other, we can use the painter's algorithm. This works because the textures are transparent, so we don't have to worry about keying out the black, we just need to draw the images furthest away first.

5. 9

6. 10, really good only a couple points were I got stuck but I was able to figure out what I was missing

7. 2 hours

8. Nope