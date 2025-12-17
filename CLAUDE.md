# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a 3D graphics project implementing a Portal-style game mechanic using OpenGL. The project demonstrates advanced rendering techniques including portal rendering with stencil buffers, scene graphs, and collision detection. This was created for POSTECH Computer Graphics course (2025 Autumn).

## Build & Run Commands

### macOS/Linux (CMake)
```bash
# Configure and build
cmake -S . -B buildcmake -G Ninja
cmake --build buildcmake

# Run the executable
./buildcmake/main
```

### Windows
```bash
# Build and run (PowerShell required)
build.bat

# Run only
run.bat
```

The CMake build:
- Requires: OpenGL, GLEW, GLM, GLFW3
- Uses Assimp for 3D model loading (included as subdirectory)
- Copies assets directory to build directory automatically
- Generates compile_commands.json for IDE support

## High-Level Architecture

### Scene Graph System

The project uses a hierarchical scene graph with the following node types:

- **Node**: Base class with position, rotation (quaternion), children, visibility/activity flags
- **SceneNode**: Root of a scene, manages main camera, collision detection, and object lifecycle
- **ObjectNode**: Interactive game objects with update/render cycles
- **ModelNode**: Renders 3D models loaded via Assimp
- **CameraNode**: Base camera (PersCameraNode for perspective, OrthoCameraNode for orthographic)
- **CollisionNode**: Spherical collision volumes attached to ObjectNodes

All nodes support hierarchical transformations through the parent-child relationship.

### Portal Rendering System

The portal system (`portal.h/cpp`) implements view-dependent rendering:

1. **Portal Geometry**: Each portal has position, direction, and up vector defining a local coordinate frame
2. **Stencil-Based Rendering**: 
   - Portals write to stencil buffer (portal1 = 1, portal2 = 2)
   - Scene is rendered multiple times with different stencil tests
   - Depth buffer is cleared in portal areas to show "through" view
3. **Camera Transformation**: `PortalManager::update_camera()` calculates virtual camera positions by transforming the viewer through one portal and out the other (180° rotation applied)
4. **Clip Planes**: `GL_CLIP_DISTANCE0` prevents rendering geometry behind the destination portal

The render order: base scene → stencil mask setup → portal1 view → portal2 view → portal frames

### Model Loading & Rendering

**ModelManager** (singleton-like global `modelManager`):
- Loads models via Assimp supporting FBX, GLTF, OBJ, etc.
- Caches loaded models and textures in maps
- Each Model contains multiple Meshes with individual Materials
- Textures resolved relative to model file path

**Rendering Modes** (toggled with 'W' key):
- Mode 0: Textured with normal mapping
- Mode 1: Flat shaded without textures  
- Mode 2: Wireframe with hidden line removal (polygon offset)

### Game Loop Structure

`main.cpp`:
- Initializes GLFW window and OpenGL context
- Sets up global shader program with uniforms (projection, view, model matrices, textures, lighting, clip plane)
- Input handled via GLFW callbacks stored in global key state maps
- Game loop: `idle()` → `display()` → swap buffers
- `MainScene` created on 'Z' press, then updated/rendered each frame

### Shader Pipeline

Single shader program defined in `shader.h`:
- Vertex shader: transforms vertices, calculates TBN matrix, applies clip plane
- Fragment shader: basic lighting with normal mapping support
- Uniforms set globally, accessed throughout the codebase via extern declarations

### Key Global State

Many variables are declared extern and shared across translation units:
- `modelManager`: Global model/texture cache
- `shaderProgram`: OpenGL shader program ID
- `projLoc, viewLoc, modelLoc, colorLoc, etc.`: Uniform locations
- `keyState, SpecialkeyState`: Input state maps
- `model_matrix, stack_model_matrix`: Transform matrices for hierarchical rendering
- `render_mode`: Current rendering style

## Controls

- Arrow keys: Movement
- Z: Start game / Fire
- Q: Switch camera
- W: Toggle rendering style
- E: Show collision bounds
- I/K/J/L/U/O: Camera movement (up/down/left/right/forward/back)
- ESC: Exit

## Project Structure

```
src/
  main.cpp              - Entry point, OpenGL setup, game loop
  model.h/cpp           - Assimp model loading, ModelManager, rendering modes
  node.h/node2.cpp      - Scene graph node hierarchy
  portal.h/cpp          - Portal rendering with stencil buffers
  shader.h              - GLSL shader source strings
  game/
    MainScene.h/cpp     - Main game scene implementation
assets/                 - 3D models and textures
assimp/                 - Assimp library (git submodule)
buildcmake/             - CMake build output directory
```

## Important Implementation Details

- **Quaternions for Rotation**: All rotations use `glm::quat` to avoid gimbal lock
- **Assimp UV Flipping**: Most models flip UVs on load (`flipUVs=true`), but portal models don't since they use FBO textures
- **Matrix Stack**: `stack_model_matrix` used for hierarchical transforms during scene graph traversal
- **Collision System**: `SceneNode::collectCollisions()` gathers all CollisionNodes, then checks sphere-sphere intersections
- **Object Lifecycle**: Objects mark themselves with `toDelete` flag, removed by `SceneNode::kill()`
