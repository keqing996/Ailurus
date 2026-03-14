# Ailurus Infrastructure Kit 

[![Build](https://github.com/keqing996/Ailurus/actions/workflows/cmake-windows.yml/badge.svg)](https://github.com/keqing996/Ailurus/actions/workflows/cmake-windows.yml)
[![Build](https://github.com/keqing996/Ailurus/actions/workflows/cmake-linux.yml/badge.svg)](https://github.com/keqing996/Ailurus/actions/workflows/cmake-linux.yml)
[![Build](https://github.com/keqing996/Ailurus/actions/workflows/cmake-macos.yml/badge.svg)](https://github.com/keqing996/Ailurus/actions/workflows/cmake-macos.yml)

## CMake Options

- `AILURUS_ENABLE_TEST=ON` enables the C++ test targets.
- `AILURUS_ENABLE_EXAMPLE=ON` enables the native example targets.
- `libwebsockets` is always built and linked into `ailurus`.

Example configure command:

```bash
cmake -S . -B build \
	-DAILURUS_ENABLE_TEST=ON \
	-DAILURUS_ENABLE_EXAMPLE=ON
```

## External Editor Prototype

The current prototype is split into two processes:

1. Native renderer example target: `ailurus_example_render` or `ailurus_example_external_editor`
2. Browser UI project under `editor/`

The runtime model is:

- A dedicated network thread owns the WebSocket server.
- The renderer main thread consumes inbound commands at fixed loop points.
- Initial connection gets a full snapshot.
- Later scene and render changes are sent as incremental deltas.
- Revision mismatch or queue overflow triggers an automatic full resync.

### Build

After configuring with `AILURUS_ENABLE_EXAMPLE=ON`, build the workspace as usual. The prototype example target is generated under the existing CMake build tree.

### Run the renderer example

On macOS, the graphics renderer app is built at:

```bash
build/example/Graphics/ailurus_example_render.app
```

The dedicated static-scene prototype is also built at:

```bash
build/example/ExternalEditor/ailurus_example_external_editor.app
```

If you launch either binary directly from the bundle, use the bundled MoltenVK ICD:

```bash
cd build/example/Graphics/ailurus_example_render.app/Contents/MacOS
export VK_DRIVER_FILES="$PWD/../Resources/vulkan/icd.d/MoltenVK_icd.json"
./ailurus_example_render
```

The prototype listens on:

```text
ws://127.0.0.1:12138
```

### Run the browser editor

Install dependencies once:

```bash
cd editor
npm install
```

Start the development server:

```bash
cd editor
npm run dev
```

Or produce a production bundle:

```bash
cd editor
npm run build
```

The browser UI expects the renderer to already be running, then it connects to `ws://127.0.0.1:12138` automatically.

### Current prototype scope

- Single browser client only.
- Edit existing entity name and transform.
- Edit a small render-settings subset.
- Automatic reconnect and resync.

Not included yet:

- Entity create/delete/reparent from the browser.
- Browser-embedded viewport streaming.
- Multi-client collaboration.