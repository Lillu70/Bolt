# Bolt
C++ 3D Rendering Engine using Vulkan.

Very early in developement, so the feature list is not final.

# Features:

- Rendering of multiple 3D models.

- Specular per fragment shading from one directional light source.

- Sesource manager for client visible objects, such as meshes, textures and materials.

- Simple application framework. Client applications inherit the virtual Bolt::Application class and override the client interface.

- Window and simplefied input handling. (using GLFW https://www.glfw.org/)

- Partial .obj model and .mtl material loading.

# Sandbox Example App
Comes with an example client application that uses Bolt.
It loads example models and has a camera controller (WASD and mouse, or arrow keys)

[W] moves the camera Up, [S] moves the camera Down,

[A] moves the camera to Left, [D] moves the camera to Right,

[Left Shift + W] moves the camera to Forwards, [Left Shift + D] moves the camera to Backwards,

[Up Arrow] & [Down Arrow] keys rotate the camera on the X axis.

[Left Arrow] & [Right Arrow] keys rotate the camera on the Y axis.

Hold down [Left Mouse Button] to rotate the camera by moving the mouse.

# Screenshots
![BOLT_Sandbox](https://user-images.githubusercontent.com/122602146/212363027-f5bf1f86-0224-4ef2-8e06-26e9434bc2e7.png)


![Bolt_Ruins](https://user-images.githubusercontent.com/122602146/212332768-af444d9b-2f4c-4c84-941a-57381d1eb5a5.png)
