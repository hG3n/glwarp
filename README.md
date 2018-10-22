# glwarp

A  software toolkit for real-time warping and configuration of single projector full-dome setups.

# Setup

## Dependencies
In order to build and run the project, several dependencies have to be met. Before installing further dependencies, make sure the Ubuntu `build-essentials` as well as `cmake` are installed. If not install them via

```
sudo apt-get install build-essentials
sudo apt-get install cmake
```

Furthermore, check whether the linux tool `pkg-config` is installed, this is needed to find library paths as well as include flags for some of the following libraries.

```
sudo apt-get install libglfw3-dev
sudo apt-get install libglm-dev 
sudo apt-get install libxmu-dev
sudo apt-get install libglew-dev
```

## Graphics card driver
Since the application employs OpenGl as 3D rendering library there might be some issues regarding graphics card drivers that might lead to the application not starting or bad performance. Therefore, make sure that either current nvidia drivers or the default ubuntu drivers are installed for the used graphics card.


## Software support
The glWarp tool was tested under Ubuntu Studio x as well as Ubuntu 18.04.


## Application
GlWarp was developed to capture screen contents while simultaneously using those as textures for the warping mesh created by [glWarp Configurator](https://github.com/hg3n/glwarp-configurator-qt). Therefor the `main.cpp` file is the entry point for extending or restructuring the application.

Code to load shaders as well as textures was taken from [OpenGl Tutorial](http://www.opengl-tutorial.org/) for simplicity reasons. It is therefor necessary to stick with the Microsoft Bitmap (bmp) format for textures. Both functions can be found as static functions within the `Shader` and `Texture` class. 

## Command line arguments
In order to specify certain options upon application start a series of command line arguments are supported. These are also printed on application start by adding the `-h` flag.

### General video options
#### Framerate `-fps`
This command simply enables printing the current framerate every second.

#### Show Polygons `-poly`
In order to debug unforseen behaviour as well as to analyze the warping mesh geometry, this flag will enable rendering polylines visualizing the to-be-rendered triangles.

#### VSync `-vsync`
This flag enables vertical synchronization. Note that enabling this might lead to a lower framerate.

#### Capture Screen `-capture`
The capture flag enables capturing the current screen output in order to reuse it as a texture for the transformation mesh. This optin is set to false per default.

### File input options
#### Configuration file specification `-config <file>`
This flag specifies the `json` file to be used as model config. Model configs are the ouput of the beforementioned glWarp-Configurator tool. If no file is specified, the application will use default config files from the `default` folder.

#### Mesh file  `-mesh <file>`
The `-mesh` flag specifies what warping mesh to use. Default files are as well situated in the default folder.

#### Texture coordinates `-texcoords <file>`
In order for the application to know how to employ a captured screenshot this file specifies the texture coordinates for an image specified as texture.

#### Texture file `-texture <file>`
If a file is specified using this flag it will be used to texturize the given mesh file instead of live capturing.

### Runtime manipulations
In order to adjust minor errors resulting from a simulation the following commands can be used to manipulate the meshs position and orientation using simple key commands.

#### General
| Key | functionality |
|-----|---------------| 
| esc | exit glwarp|
| r |reload transformation settings|
| i |print mesh position and rotation information|
| x |reset mesh position and rotation|
| f |activate continuous fps output|

#### Mesh
|Key| Funcitionality|
|---|---------------|
| w | increase distance to mesh|
| s | decrease distance to mesh|
| a | move mesh to the left|
| d | move mesh to the right|
| j | move mesh up|
| k | move mesh down|

#### Movement & Rotation settings
|Key| Funcitionality|
|---|---------------|
|1 | decrease movement factor| 
|2 | increase movement factor| 
|3 | decrease rotation factor| 
|4 | increase rotation factor| 
