# Bolt
C++ 3D Rendering Engine using Vulkan.

Very early in developement, so the feature list is not final.

# Features:

-Rendering of multiple 3D models.

-Specular per fragment shading from one directional light source.

-Resource manager for client visible objects, such as meshes, textures and materials.

-Simple application framework. Client applications inherit the virtual Bolt::Application class and override the client interface.

-Window and simplefied input handling. (using GLFW https://www.glfw.org/)

-Partial .obj model loading. (using TinyObjLoader https://github.com/tinyobjloader/tinyobjloader)

# Sandbox Example App
Comes with an example client application that uses Bolt.
It loads example models and has a camera controller (WASD and mouse, or arrow keys)

# Screenshots
![Bolt_Ruins](https://user-images.githubusercontent.com/122602146/212332768-af444d9b-2f4c-4c84-941a-57381d1eb5a5.png)


![Bolt_Scene](https://user-images.githubusercontent.com/122602146/212340009-506e75dc-4a96-4131-bdde-59207062d455.png)
