# AI CAD Creator (OCCT Starter Scaffold)

This is a starter scaffold for an **AI-first parametric CAD application** built on **Open CASCADE Technology (OCCT)**.

It is intentionally focused on the right first milestone:

- prompt to structured CAD plan
- structured CAD plan to feature graph
- feature graph to OCCT execution
- OCCT result to interactive viewer
- export to STEP

This repo is a **starting point**, not a finished CAD system. It gives you a clean project layout, a CAD IR schema, and the core app/document/viewer boundaries so you can build the real engine without painting yourself into a corner.

## First milestone

Support this flow:

> Create a mounting bracket 100 x 50 x 6 mm with two 5 mm holes centered 10 mm from each end.

Then support edits like:

- make it 8 mm thick
- move the holes 5 mm inward
- export STEP

## Architecture

### Layers

1. **AI layer**
   - turns natural language into CAD IR JSON
   - validates edits and feature diffs

2. **Document layer**
   - OCAF-backed model store for parts, parameters, features, references, and metadata

3. **Feature graph**
   - deterministic recompute engine
   - each feature has typed inputs, outputs, and an execution strategy

4. **Geometry layer**
   - OCCT operations for sketch profiles, extrude, cut, fillet, export, etc.

5. **Viewer layer**
   - AIS/V3d display, selection, highlight, and preview

## Current contents

- `schemas/cad_ir.schema.json` - starter JSON schema for the AI CAD IR
- `docs/ROADMAP.md` - staged build plan
- `src/main.cpp` - app entry point
- `src/app/` - app shell
- `src/document/` - OCAF document boundary
- `src/features/` - feature abstraction + first placeholder extrude feature
- `src/ai/` - CAD IR parsing boundary
- `src/viewer/` - native Qt host + OCCT AIS viewport
- `src/geom/` - first real geometry path (rectangle profile -> prism/extrude)
- `src/selectors/` - topology-independent named-region selector definitions and OCCT face queries

## Build notes

This scaffold does **not** assume a specific OCCT package layout.
Instead, it expects you to point CMake at your OCCT and Qt installs using cache variables.

### Example configure command

```bash
cmake -S . -B build \
  -DOCCT_INCLUDE_DIR=/path/to/occt/inc \
  -DOCCT_LIBRARY_DIR=/path/to/occt/lib \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/your-compiler
```

Then:

```bash
cmake --build build
```

## What to build next

1. get a blank Qt + AIS viewer window running
2. create/open an OCAF document
3. execute one hard-coded feature chain
4. load a CAD IR JSON file and map it to feature nodes
5. add prompt-to-IR generation
6. add conversational edit diffs

## Design rules

- never let the LLM write raw geometry code directly into the model
- always go through CAD IR + validation
- keep geometry deterministic
- keep AI explainable and reversible
- persist prompt-to-feature links for later editing


## Current native demo

The center viewport is now wired to a native OCCT AIS/V3d view hosted inside Qt.
On startup it builds one real body through OCCT:

- rectangle wire
- planar face
- prism / extrude along +Z

You can also export that demo body through **File -> Export Demo STEP...**

## Current selector demo

The viewer now includes a **Named Regions** dock driven by topology-independent selectors.
For the demo prism it exposes:

- `top_face` -> `max_z_planar_face`
- `bottom_face` -> `min_z_planar_face`
- `largest_planar_face` -> `largest_planar_face`
- `side_faces` -> `vertical_planar_faces`

This is the first step toward stable AI edits and later CAE boundary mapping without relying on fragile raw face IDs.


## Current interaction demo

The OCCT viewport now supports direct mouse interaction:

- **left drag** -> rotate
- **middle drag** or **Shift + left drag** -> pan
- **right drag** or **mouse wheel** -> zoom
- **left click** -> select a face through AIS
- **double left click** -> fit view

When a clicked face matches one of the demo named-region selectors, the **Named Regions** dock is synchronized automatically.
