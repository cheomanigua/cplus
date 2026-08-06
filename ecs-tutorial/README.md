# Introduction

# Architecture

This document is a quick reference. It layouts the architecture and patterns used.

- **Data-Driven Design**: loading configuration from files.
- **Data-Oriented Design**: structuring memory for cache efficiency.
- **ECS**: organizing state as components.

# ECS Pattern

**ECS (Entity Component System)** – Great for the high-performance simulation core (physics, combat, AI).
- **Entity**: Just a unique ID (like a serial number).
- **Component**: Pure data (e.g., a struct with position, a struct with health).
- **System**: Logic that processes many components at once (e.g., a `MovementSystem` that loops through all entities with `PositionComponent` , `DirectionComponent` and `SpeedComponent`).

There is also an alternative and better known pattern, that I put here just for curiosity:

**MVC (Model-View-Controller)** – Great for user interfaces and overall structure.  
- **Model**: The data (health, position, etc.).  
- **View**: What the player sees (sprites, UI).  
- **Controller**: Handles input and orchestrates changes.

## Why ECS is Extremely Fast

Modern CPUs have tiny but blazing-fast memory areas called **L1 and L2 caches** (usually 32–512 KB per core). Accessing data in these caches is ~10–100x faster than fetching from main RAM. A **cache miss** (when the CPU has to wait for RAM) stalls the processor and kills performance.

**ECS achieves high speed by making cache misses rare**:

- **Flat Arrays (Structure of Arrays)**:  
  Instead of objects scattered randomly in memory, ECS stores all components of the same type in one big contiguous block (a flat array).  
  When a System processes 10,000 ships’ health, it walks sequentially through memory. The CPU can **prefetch** the next chunk of data, keeping everything in the fast L1/L2 cache. This is the opposite of traditional OOP, where following pointers jumps all over RAM.

- **Index Map**:  
  A lookup table that translates a flexible identifier (string name or enum) into a memory offset in the flat array.  
  This gives you the best of both worlds: human-friendly names in your JSON data, while the engine still uses fast integer-based access under the hood.

- **Tags / Bitmasks**:  
  Simple flags or bit patterns attached to entities. They let systems quickly filter “only process entities that have a Weapon AND are Alive” without checking every single entity.  
  This enables fast, often branchless (no `if` statements that slow down the CPU) queries.

### Performance vs. Flexibility

| Layer | Function | Performance Impact |
| --- | --- | --- |
| **Flat Array (`Values[]`)** | Contiguous memory for attributes | Maximum (Zero stall cycles) |
| **Index Map** | Identity to Offset translation | High (O(1) lookup) |
| **JSON Blueprints** | Data-driven initialization | Maximum Flexibility (No recompile) |
| **Tag/Masks** | Replaces inheritance | High (Branchless filtering) |

**Bottom line**: ECS organizes data for the *hardware*, not for human intuition. The result is dramatically higher throughput in simulations with thousands of entities.


# Implemented

So far, this is what has been implemented:

* **Step 1. Project Initialization & Shell Setup**: Establishes the base project directory structure, target configuration, and build environment using CMake.
* **Step 2. Core ECS Setup**: Defines the fundamental entity identifiers (`Entity`) and null limits (`NullEntity`) within `Types.h` to drive the ECS core.
* **Step 3. Sparse Set Setup**: Implements a data-oriented `SparseSet` data structure utilizing sparse vectors and dense arrays for efficient component indexing, validation, and cache locality.
* **Step 4. Component Pool Setup**: Creates type-specific `ComponentPool<T>` containers wrapped around sparse sets, backed by the `IComponentPool` virtual interface for generic pool management.
* **Step 5. Registry Component Management Setup**: Builds the central `Registry` class capable of entity creation, type-indexed pool management, component insertion, and retrieval safety checks.
* **Step 6. Position Component & Usage Setup**: Develops the `PositionComponent` struct and integrated initial coordinate data into entities within the application flow.
* **Step 7. Console Render System**: Implements the `ConsoleRenderSystem` to print entity positions and tracking text directly to standard output for debugging.
* **Step 8. Graphical Render System**: Integrates Raylib into `GraphicalRenderSystem` to render entities visually as 2D shapes on a window canvas.
* **Step 9: Movement System & Velocity Component**: Introduces the `DirectionComponent` and implemented the `MovementSystem` to update entity positions based on direction metrics each frame.
* **Step 10: Input System**: Builds the `InputSystem` to capture real-time keyboard states and update entity velocities interactively.
* **Step 11: Tag Components (ECS Tagging System)**: Develops `TagSelectedComponent` to flag and categorize entities dynamically based on runtime states.
* **Step 12: Selection Bounds Component**: Implements `SelectionBoundsComponent` and integrated Raylib collision checks to enable mouse picking and single-entity selection.
* **Step 13: Speed Component**: Added the `SpeedComponent` to decouple movement scaling factors from individual entities and systems.
* **Step 14: Entity Lifecycle Management and ID Recycling**: Implements `destroyEntity(Entity)` to automatically strip components across all generic pools, purge entities from active tracking vectors, and push recycled IDs onto a free-list stack for future reuse.
* **Step 15. Entity Spawning and Despawning**: Implements variadic templates with fold expressions for dynamic component packing during entity creation, alongside unified despawn wrappers to handle runtime lifecycle management.



General:

* **Core Types & Identifiers**: Defines `Entity` as a unique integer ID and set up `NullEntity` limits.
* **Sparse Set Memory Layout**: Implements a data-oriented `SparseSet` data structure utilizing sparse vectors and dense arrays for efficient component storage and cache locality.
* **Component Pools & Polymorphism**: Creates `ComponentPool<T>` wrapped around sparse sets and introduced the `IComponentPool` virtual interface to handle component containers generically.
* **Entity Registry**: Builds the central `Registry` class capable of entity creation, component insertion, component retrieval, safety checks, and dynamic component additions.
* **Entity Lifecycle & ID Recycling**: Implements `destroyEntity(Entity)` to automatically strip all components from component pools, purge entities from active tracking vectors, and push recycled IDs onto a free-list stack for future reuse.
* **Component Modules**: Develops multiple dedicated component structs including `PositionComponent`, `DirectionComponent`, `SpeedComponent`, `TagSelectedComponent`, and `SelectionBoundsComponent`.
* **Simulation Systems**: Implements functional systems for user input (`InputSystem`), physics movement (`MovementSystem`), console logging (`ConsoleRenderSystem`), and Raylib-based 2D shape rendering (`GraphicalRenderSystem`).
* **Interactive Application Loop**: Set up a game loop inside `main.cpp` leveraging Raylib to handle real-time mouse clicking, entity selection, and dynamic movement processing.


# Not implemented


### Step 18: Scene / World Management Container

A mature ECS engine requires a central container or scene manager to own the registry, handle state resets, and manage multi-scene transitions cleanly.
* Without scene isolation, global registries accumulate stale states and make resetting or loading new game levels difficult.
* With a dedicated world container, the engine can initialize, update, and tear down entire simulation environments safely.


By introducing this organizational layer, we validate our architecture's ability to handle structured game flow and application state changes.


### Step 19: Component-Based Event / Messaging System

Asynchronous communication between systems is essential for decoupling complex gameplay interactions like collision alerts or damage triggers.
* Without an event queue, systems must invoke each other directly or rely on rigid sequential checks, leading to tightly coupled code.
* With a lightweight messaging pipeline, systems can broadcast and listen for events across simulation loops safely.


By introducing this event queue layer, we validate our engine's ability to support decoupled, reactive gameplay mechanics.


### Step 20: Data-Driven Configuration (JSON Loading)
Hardcoding entity initializations inside application source files restricts flexibility for modding and level design.
* Without external data configuration, every design adjustment requires recompiling source code.
* With an integrated JSON parser, the engine can read entity blueprints, initial positions, and component properties from external files.

By introducing data-driven loading, we validate our architecture's capability to separate code structure from content data.

### Step 21: Multi-Threaded System Scheduling
High-performance engines require parallel task execution to maximize multi-core CPU utilization during large simulations.
* Without system scheduling separation, all tasks run sequentially on a single thread regardless of workload size.
* With categorized read-only and read-write system identification, independent simulations like rendering or audio calculations can execute concurrently.

By introducing multi-threaded scheduling, we validate our engine's readiness for heavy, production-scale performance demands.

### Step 22: View / Query System

An Entity Component System (ECS) architecture benefits from query abstractions to efficiently retrieve and iterate over entities matching specific component combinations without repetitive manual checks.
* Without a dedicated query view, systems must manually verify component presence for every entity, increasing boilerplate and reducing cache traversal efficiency.
* With a view iterator helper in the registry, systems can cleanly and automatically target only the entities possessing the exact required component signatures.


By introducing this querying layer, we validate our engine's ability to scale system logic cleanly as component combinations grow.

### Step 23: Spatial Partitioning
### Step 24: Dirty Flags
### Step 25: Collisions

