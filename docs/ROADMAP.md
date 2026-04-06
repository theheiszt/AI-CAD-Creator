# Roadmap

## Phase 1 - single part, deterministic core

- Qt desktop shell
- AIS viewer window
- OCAF document create/open/save
- base planes
- one sketch plane assumption
- hard-coded rectangle + extrude
- STEP export

## Phase 2 - feature graph

- feature base class
- typed parameters
- dependency edges
- recompute manager
- result shape registry
- failure diagnostics

## Phase 3 - CAD IR

- JSON schema validation
- sketch / extrude / cut / fillet / hole subset
- stable feature IDs
- edit diff format
- prompt history attached to document

## Phase 4 - AI loop

- prompt -> CAD IR generation
- edit prompt -> IR diff generation
- preview before commit
- geometry failure retry strategies

## Phase 5 - stronger references

- face/edge query system ✅ starter named-region selectors added for demo prism
- semantic selection aliases ✅ top_face / bottom_face / side_faces / largest_planar_face
- topological naming strategy
- prompt-linked feature explanations

## Phase 6 - practical modeling tools

- revolve
- shell
- pattern
- mirror
- chamfer
- import STEP -> heal -> display
