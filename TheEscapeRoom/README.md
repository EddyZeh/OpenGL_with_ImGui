# The Escape Room

A small first-person puzzle game built on the **Graphics Engine** in this repository
(`../src`). This project implements the plan in *The Escape Room - Project Plan*:
one room, one key, one code puzzle, one exit, and an ending.

The room is built entirely from **placeholder primitives** (boxes, spheres, cylinders).
Each object belongs to a named group, and any group can be swapped for an imported model
later without touching gameplay, collision or interaction.

---

## Building (Visual Studio 2022)

1. Open `Graphics Engine.sln` (the project has been added to it) or `TheEscapeRoom/TheEscapeRoom.sln`.
2. Tell the project where **GLFW** and **Assimp** live by editing `TheEscapeRoom/Dependencies.props`
   (or set a `DEPS_DIR` environment variable). Use the same copies as the Graphics Engine project.

   | Macro                 | Default                         |
   |-----------------------|---------------------------------|
   | `DEPS_DIR`            | `C:\OpenGL`                     |
   | `GLFW_INCLUDE_DIR`    | `$(DEPS_DIR)\include`           |
   | `GLFW_LIB_DIR` / `GLFW_LIB` | `$(DEPS_DIR)\lib` / `glfw3.lib` |
   | `ASSIMP_INCLUDE_DIR`  | `$(DEPS_DIR)\include`           |
   | `ASSIMP_LIB_DIR` / `ASSIMP_LIB` | `$(DEPS_DIR)\lib` / `assimp-vc143-mt.lib` |
   | `ASSIMP_BIN_DIR`      | `$(DEPS_DIR)\bin` (DLL copied next to the exe) |

   Dear ImGui (1.91.9, matching the engine's back ends), GLAD (GL 3.3 core), GLM and
   stb_image are bundled in `vendor/`, so nothing else is needed.
   With vcpkg instead: `vcpkg install glfw3 assimp`, run `vcpkg integrate install`, and set
   `GLFW_LIB` to `glfw3dll.lib` and `ASSIMP_LIB` to the name vcpkg installed.
3. Set **TheEscapeRoom** as the startup project, pick **x64**, build and run.

The post-build step copies the engine's shaders + textures and this project's shaders + fonts
next to the executable (`bin\x64\<Config>\`), because the engine resolves all asset paths
relative to the `.exe`.

The game compiles the engine's `.cpp` files directly from `../src` (everything except the
engine's `main.cpp`), so engine fixes are shared by both projects.

---

## How to play

You wake up in a sealed underground room. Everything you need is inside it.

| Action | Keys |
|---|---|
| Move / sprint | WASD or arrows / Shift |
| Look | Mouse |
| Interact | **E** |
| Torch on/off | F |
| Pause / close a window | Esc |
| Keypad | Click, or type 0-9, Enter, Backspace, Delete |
| Debug panel / collision boxes | F1 / F2 |
| Gamepad | Left stick move, right stick look, A interact, Start pause |

Puzzle chain (codes are randomised every game):

```
Note on the desk ──> explains the clue board
Clue board ───────> count the lit lights per row (red, green, blue, yellow) = cabinet code
Cabinet keypad ───> cabinet opens ──> brass key
Key ──────────────> lockbox on the table ──> card with the exit code
Exit keypad ──────> door unlocks ──> press E on the door ──> walk out through the corridor
```

Main menu options: **Timed mode** (3-15 minute lockdown countdown, with a game-over screen),
mouse sensitivity, FOV, invert Y, head bob, shadows.

---

## Code layout

```
TheEscapeRoom/
├── src/
│   ├── main.cpp                    window + ImGui front end (engine Scene), main loop
│   ├── Game/
│   │   ├── Game.h/.cpp             game loop, screens, input routing, win / lose
│   │   ├── GameState.h             progression flags, timer
│   │   ├── GameContext.h           what objects may read / change; UI requests
│   │   └── Level.h/.cpp            the room: props, colliders, lights, interactables
│   ├── Player/Player.h/.cpp        FPS controller with AABB collision (engine Camera for looking)
│   ├── Interaction/
│   │   ├── Interactable.h          common interface (canTarget / prompt / interact / bounds)
│   │   └── InteractionSystem.*     ray cast from the view centre, max range, occlusion
│   ├── Puzzles/PuzzleManager.*     codes, clue generation, every unlock rule
│   ├── Objects/                    Door (hinged, lock state), Cabinet, Key, LockBox,
│   │                               Readable notes, ClueBoard, Keypad (+ exit panel), ExitDoor
│   ├── Rendering/
│   │   ├── Prop.h                  placeholder primitive + surface description
│   │   ├── Primitives.*            cube / sphere / cylinder meshes (engine Mesh)
│   │   └── WorldRenderer.*         draws the level with the engine's lights, shadow maps,
│   │                               Model loader and PostProcessor
│   └── UI/GameUI.*                 all ImGui screens: menu, HUD, keypad, note, pause, victory
├── shaders/                        room.vert/.frag (lighting), post.frag (vignette, grain, fades)
├── assets/fonts, assets/models     fonts (Roboto, Cousine); put your models here
└── vendor/                         imgui core, glad, glm, stb_image
```

Engine pieces reused: `Scene` (window, input callbacks, ImGui front end, view/projection uniforms),
`Camera`, `Keyboard`/`Mouse`/`Joystick`, `Shader`, `Texture`, `Mesh`, `Model` (Assimp),
`PointLight`/`SpotLight` + their shadow maps, `PostProcessor`, `ResourceManager`/`Paths`,
and the engine's depth shaders.

Added to the engine (header-only, does not affect the Graphics Engine project):
`src/physics/collision.hpp` - AABB, ray-vs-AABB slab test, transformed bounds, XZ penetration.

---

## Replacing placeholders with models

Every placeholder group has an **anchor** (its placement in the room). In
`Level::applyModelReplacements()` (bottom of `src/Game/Level.cpp`):

```cpp
// model is drawn at anchor("desk") * local; the desk's primitives are hidden,
// its collider and interaction volume stay exactly as they are
replaceWithModel("desk", "desk/scene.gltf", glm::scale(glm::mat4(1.0f), glm::vec3(0.01f)));
```

Paths are relative to `assets/models` next to the exe: drop models in `TheEscapeRoom/assets/models/`
(copied by the post-build step). Moving parts work too: a door's anchor is its hinge, the key's
anchor spins with it, and a model hides automatically when its group is hidden (the key is taken).

Group names: `desk`, `chair`, `desk_lamp`, `desk_clutter`, `note`, `cabinet`, `cabinet_door`, `key`,
`table`, `table_clutter`, `lockbox_body`, `lockbox` (lid), `exit_card`, `clueboard`, `exit_door`,
`exit_keypad`, `cot`, `backpack`, `barrels`, `crates`, `pipes`, `ceiling_lamp`, `room`, `corridor`.

A working example is included: set `USE_IMPORTED_MODELS = true` near the top of `Level.cpp` and
`COPY_ENGINE_MODELS` to `true` in `Dependencies.props` to replace the backpack placeholder with the
engine's `survival_guitar_backpack` model. Note that the engine's `Model` loader ignores node
transforms, so glTF files exported Z-up need the -90° X rotation used in that example.

---

## Notes

- **Collision**: the player is a 0.5 × 1.75 × 0.5 m box resolved per axis in ≤5 cm sub-steps
  (no tunnelling on frame spikes), plus a push-out step so a swinging door never traps you.
  Press F2 to see every collider (green), interaction volume (yellow) and the exit trigger (blue).
- **Interaction**: ray from the screen centre, 2.3 m range, nearest hit wins; walls and the
  closed exit door block the ray.
- **Doors**: `Locked → Closed → Opening → Open`. The visual transform and the collider both
  come from the same hinge angle, so an open door never leaves an invisible wall behind.
- One small engine fix was made in `src/graphics/model.cpp`: texture de-duplication compared the
  stored full path with the relative name, so it never matched and multi-mesh models re-uploaded
  the same texture once per mesh.
- Quick key taps are read through ImGui's input queue, because the engine's
  `Keyboard::keyWentDown` misses a press and release that happen within the same frame.
