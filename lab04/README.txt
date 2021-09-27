Carter Fowler - Aethon Misty

1. What is a uniform variable for GLSL? Why should the MVP matrix be a uniform?
	A uniform variable is any variable that is used to store data passed into the GLSL from OpenGL code that is consistent for every vertex. The MVP matrix needs to be uniform in this situation because it is constant for every vertex in the object.

2. What is an attribute variable for GLSL? Why should the vertex position be an attribute?
	An attribute variable is any variable that is unique to each vertex, like its position. Which is why the vertex position is an attribute, to specify that it will depend on which vertex we are looking at for the shader.

3. What is a varying variable for GLSL? Why should the color be varying?
	A variable that is passed from the vertex shader to the fragment shader, and is set per vertex and then read per fragment. The color should be varying since we want to be able to change the color based on what vertex we are at.

4. Why do we not need to recompile the C++ code after we modify the GLSL code? Why are weallowed to rerun our C++ program to test new GLSL code?
	No changes were made in our C++ code, so it doesn't need to recompile. The only thing that needs to compile is the GLSL code, which is compiled on the GPU, not on the CPU like the C++ code. It is because the GLSL code compiles on the GPU that allows us to test new GLSL code with having to recompile the C++ code.

5. Why does it look like that?
	The color is based on the vertex's position, so we naturally get a rainbow of colors on the sphere since it has nearly every possible position value to apply to RGB.

6. Did this lab clear up the confusion involving GLSL and shaders? If not, what confusionremains?
	Yeah, I think I just need a little more practice before I feel really comfortable.

7. Was this lab fun? 1-10 (1 least fun, 10 most fun)
	8, fun in the end but stressful to start, like most labs for me.

8. How was the write-up for the lab? Too much hand holding? Too thorough? Too vague? Justright?
	A little vague at times, but mostly pretty good.

9. How long did this lab take you?
	3-4 hours

10. Any other comments?
	Nope