# VulkanPractice0726

A from-scratch Vulkan renderer (SDL3 window/input, VMA for memory, dynamic
rendering — no `VkRenderPass`/`VkFramebuffer`). The design goal throughout has
been: derive as much Vulkan boilerplate as possible from plain C++ types,
rather than hand-writing `VkVertexInputAttributeDescription`s etc. per app.

## Layout

```
Source_support/
  Context/       Vulkan instance/device/window setup (one-time, app lifetime)
  Renderer/      Per-frame lifecycle: acquire, record, submit, present
  RenderContent/ Pipeline, vertex/uniform buffers, draw-list aggregation
  Maths/         Header-only vec/mat/transform library
  App/           Base class tying Context + Renderer + your app together
  InputHandler/  Stub — not wired up yet
Source_apps/     One .cpp per executable (each defines a class deriving App)
Source_shaders/  GLSL sources, compiled to Bin_shaders/*.spv by the Shaders
                 CMake target (glslc)
```

Each subdirectory of `Source_support` (except `App`) becomes its own CMake
library, auto-discovered by `file(GLOB ...)` — see the note on that in
"Gotchas" below. `App` is the one library every other one publicly links, and
every executable under `Source_apps` links all of them.

## The layering, bottom to top

**`Context`** — owns the `VkInstance`, physical/logical device, surface,
swapchain, and the VMA allocator. Created once via `App::Start`, lives for the
whole app. You generally don't touch this directly; `App` hands its pieces
(`mContext.mDevice.mLogicalDevice`, `mContext.mAllocator`, etc.) to whatever
needs them.

**`Renderer`** — owns everything that's *frame-indexed*: command pool/buffers,
per-frame-in-flight semaphores/fences, and now the descriptor-set bind. It
doesn't know about vertex formats, uniform types, or maths — it just walks a
`DrawList` and issues the Vulkan calls. `mCurrentFrameInd` cycles between `0`
and `1` (2 frames in flight, hardcoded — see Gotchas). `GetCurrentFrameInd()`
is how `App` knows which frame's uniform data to write before recording.

**`RenderContent`** — the "shape of the draw" layer, and the most
template-heavy part of the codebase:

- [`pipeline.h`](Source_support/RenderContent/pipeline.h) — `VulkanPipeline<Attributes...>`.
  Template parameter pack = your vertex attributes, e.g.
  `VulkanPipeline<maths::vec3f, maths::vec3f>` for position+color. It derives
  `VkVertexInputBindingDescription`/`VkVertexInputAttributeDescription` from
  each type's `sizeof` and a `Stride2VkFormat<T>` trait (specialized per type —
  add a new specialization here if you introduce a new attribute type).
  `Setup()` optionally takes a `VkDescriptorSetLayout` to fold into the
  pipeline layout.
- [`renderObject.h`](Source_support/RenderContent/renderObject.h) /
  [`dataHandler.h`](Source_support/RenderContent/dataHandler.h) —
  `RenderObject<Attributes...>` + `VulkanDataHandler<Attributes...>`. Same
  attribute pack as the pipeline (must match!). `DataHandler::AddRenderObject`
  takes one `std::vector<Attributes>` per attribute plus an index vector, and
  allocates/uploads a VMA buffer for each (write-once, not per-frame).
- [`uniformData.h`](Source_support/RenderContent/uniformData.h) /
  [`uniformHandler.h`](Source_support/RenderContent/uniformHandler.h) — the
  uniform equivalent, but *not* attribute-pack-generic (just one struct type,
  e.g. `MVPUniform`), and *not* write-once: `VulkanUniformHandler<T>` owns one
  persistently-mapped buffer + descriptor set per frame-in-flight, and
  `Update(frameInd, data)` is meant to be called every frame.
- [`drawList.h`](Source_support/RenderContent/drawList.h) — the thing actually
  handed to `Renderer::RenderFrame` each frame. Aggregates pipelines +
  render objects + (optionally) per-frame descriptor sets. `AddPipeline`'s
  descriptor-set argument is a `std::vector<VkDescriptorSet>` sized to match
  frame-in-flight count; empty = no descriptor set to bind for that pipeline.

**`Maths`** — header-only (`vec3f`/`vec4f` in [`vec.h`](Source_support/Maths/vec.h),
`mat4f` in [`mat.h`](Source_support/Maths/mat.h), transform builders in
[`Transform.h`](Source_support/Maths/Transform.h)). `mat4f` is column-major to
match GLSL/std140 directly — no transpose needed when uploading to a UBO.
Rotation builders are named `RotateRoll`/`RotateYaw`/`RotateTilt` (roll = X
axis, yaw = Y axis, tilt = Z axis). `Perspective` bakes in a Y-flip to account
for Vulkan's Y-down NDC, so nothing downstream needs to know about that.

**`App`** — the base class every app derives from. You override:
- `SetupPipelines()` — build your `VulkanPipeline`/`VulkanUniformHandler`,
  register with `DrawList::AddPipeline`.
- `SetupRenderObjects()` — build your `VulkanDataHandler` objects, register
  with `DrawList::AddRenderObject`.
- `UpdateUniforms(uint32_t aFrameInd)` — called once per frame, before
  `DrawFrame()`. Compute your MVP (or whatever) here and call
  `handler.Update(aFrameInd, data)`.
- `DrawFrame()` — pure virtual; almost always just `return
  mRenderer.RenderFrame(mDrawList);`.

`App::Start(w, h)` wires Context → SetupPipelines → SetupRenderObjects →
Renderer::Setup. `App::RunLoop()` handles the SDL event pump, resize, and
calls `UpdateUniforms` then `DrawFrame` each iteration, and waits for the GPU
to idle before returning (so member destructors — pipelines, buffers — are
safe to run after).

## Existing apps

- [`RainbowTriangle.cpp`](Source_apps/RainbowTriangle.cpp) — 2D triangle,
  `vec2` position + `vec3` color, no uniforms. Simplest reference example.
- [`HelloCube.cpp`](Source_apps/HelloCube.cpp) — 3D cube, `vec3` position +
  `vec3` color, one `MVPUniform` (rotating model matrix × fixed view × fixed
  perspective projection) via `VulkanUniformHandler`.

## Adding a new app

1. New `.cpp` under `Source_apps/`, deriving `App`.
2. Pick your vertex attribute types → `VulkanPipeline<...>` +
   `VulkanDataHandler<...>` with matching packs.
3. If new attribute types, add a `Stride2VkFormat<T>` specialization in
   `pipeline.h`.
4. If you need per-frame data, define a struct in (or alongside)
   `uniformData.h`, add a `VulkanUniformHandler<YourStruct>` member, `Setup()`
   it before the pipeline (pipeline needs its `mLayout`), and override
   `UpdateUniforms`.
5. Write/reuse GLSL shaders in `Source_shaders/`, matching your vertex
   attribute locations and any `uniform` block layout.
6. `int main()` — don't forget it (bitten us once already).
7. Reconfigure with `cmake -B Build` (new files under globbed directories
   need a fresh configure, not just a build) before building.

## Gotchas worth knowing

- **Build environment**: this project must be built from a shell with the
  MSVC dev environment loaded (`INCLUDE`/`LIB`/`PATH` set) — e.g. "x64 Native
  Tools Command Prompt for VS". A plain PowerShell/cmd window will fail with
  missing-header errors (`stdarg.h`, `sys/types.h`, etc.) that look like
  source problems but aren't. `Launch-VsDevShell.ps1` has been unreliable for
  this VS install in practice; the Start-menu shortcut is the reliable path.
- **Ninja isn't on PATH** even in the dev shell for this setup — pass
  `-DCMAKE_MAKE_PROGRAM="...\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"`
  explicitly when configuring.
- **CMake globs aren't live**: new files/directories under `Source_support`,
  `Source_apps`, `Source_shaders` need a `cmake -B Build` reconfigure, not
  just `cmake --build Build`, before they're picked up.
- **Header-only library directories** (like `Maths`) are declared
  `INTERFACE`, not `STATIC` — a `STATIC` library with zero `.cpp` files
  produces no archive at all, which fails at *link* time with a confusing
  "cannot open input file" error rather than anything obviously related to
  headers.
- **Frame-in-flight count is hardcoded to `2`** in a few places (`Renderer`'s
  `mCmdBuff`/`mImageAvailable`/`mFrameInFlight`, and wherever an app calls
  `VulkanUniformHandler::Setup(..., 2)`). These aren't derived from a shared
  constant yet — keep them in sync manually if you ever change it.
- **Culling is on** (`VK_CULL_MODE_BACK_BIT` + `VK_FRONT_FACE_COUNTER_CLOCKWISE`
  in `pipeline.h`). Get a face's index winding backwards and it silently
  vanishes rather than rendering inside-out — that's the first thing to check
  if geometry looks like it's missing a face.
- **No real delta-time yet** — `HelloCube::UpdateUniforms` advances rotation
  by a hardcoded `1.0f/60.0f` per frame, so rotation speed will drift with
  actual framerate rather than being wall-clock accurate.
- **`InputHandler` is an empty stub** — present as a placeholder, not wired
  into `App` yet.
