# OpenGL Coursework 2 (70%) "The Fish"

CONTROlS
WASD: move fish
QE: Rotate fish
ARROW L&R: Pan camera
ESC: Exit Game

This project is an OpenGL demonstration of various shader techniques. 
The project was made in Visual Studios 2022 (v.17.9.0)
It was created on Windows 10 (10.0.22631, Build 22631)
The version of OpenGL uses: GLFW, GLM, GLAD.

The project contains several shader files of type frag and vert to assist in creating a visual rendering of the scene in OpenGL. 
The types of shaders used in this project are ShadowMap and PBR(physically based rendering)

The shadow shader prograam (ShadowProg) is used to cast dynamic shadows on object, it uses a shadow mapping technique with basic blinn-phong lighting model
to render objects.

The PBR or Physical Based Rendering shader is used to compute the lighting and shading of a surface based on materialistic properties given
to it. The properties that can change this are the material roughness and whether the material is made of metal or not.

With these shaders, the main bulk of the code in SceneBasic_Uniform renders the scene with different lighting and objects, including custom made
models from blender, such as the fish bowl, fish and cake. Also here, the game elements are made with camera controls to simulate a home security camera,
and the ability to control the fishes movement inside the play area.

Win the game by controlling the fish and taking him back to the bowl! you might even get a little surprise!




