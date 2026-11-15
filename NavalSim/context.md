# NavalSim — Project Context and Development Handoff

**Last updated:** 15 November 2026  
**Latest completed milestone:** 5.13 — Assemble Runtime Ammunition Inventories  
**Latest commit:** `7c60a25fb9c3091bc4fbc4f8aee37f3bf64fbc1d`  
**Latest test result:** 312 tests passed, 0 failed

---

## 1. Project Overview

NavalSim is a real-time naval warfare simulation inspired by games such as Harpoon.

### Technology

- C++20
- Flecs 4.1.6 (ECS)
- SDL3
- JSON-based data definitions
- CMake
- Doctest

### Long-term goals

- Real-time simulation with adjustable time acceleration.
- Data-driven content, including units, components, ammunition and fuel.
- Mission/scenario construction.
- Modding and content extensibility.
- Generic simulation architecture rather than code tied specifically to naval vessels.
- Terminal-first testing.
- Simulation engine independent of rendering technology.
- Potential future rendering integration with SDL3, Raylib, Godot or another frontend.

The simulation core should remain usable without initializing a graphical application.

---

## 2. Architectural Principles

These principles are important. Do not compromise them without discussing the architectural consequences first.

### Separation of responsibilities

**Data layer**
- Loads and validates JSON definitions.
- Stores and retrieves typed definitions through a generic definition database.
- Does not depend on Flecs.
- Does not depend on SDL3.
- Does not represent live simulation state.

**Simulation layer**
- Contains simulation behaviour and domain logic.
- Coordinates simulation systems, commands and runtime entity assembly.
- Should remain testable without rendering.

**ECS/world layer**
- Uses Flecs to represent live simulation entities and their runtime state.
- Uses strongly typed components.
- Does not understand JSON.
- Should not contain arbitrary JSON maps or untyped property bags.

**Presentation layer**
- Should eventually consume simulation state.
- Must not become a dependency of core simulation logic.

### Definitions versus runtime entities

Static definitions describe what something is.

Runtime ECS components describe the current state of an instance in the simulation.

For example:

- A radar definition specifies its technical range.
- A radar installation has its own condition and enabled/disabled state.
- Two installations of the same radar definition must have independent runtime states.

A future scenario or mission loader should eventually provide the configuration used to assemble runtime entities. The current project does not yet have a scenario/mission configuration implementation.

### Flecs

Use Flecs phases such as `flecs::OnStart` and `flecs::OnUpdate` where appropriate.

Installed components should be represented as individual Flecs child entities using `ChildOf`, rather than being collapsed into arbitrary collections of JSON-like properties.

---

## 3. Important Domain Decisions

These decisions were explicitly discussed and agreed. Do not silently reverse them.

### Units and movement

- Use generic **Unit** terminology rather than Vessel terminology in generic simulation APIs.
- A unit has a direct `max_speed_knots` definition property.
- Do not introduce a physics model based on engine thrust, mass, drag or resistance for the current scope.
- Mass, length and width are not currently required in the initial unit definition.
- Maximum speed is a base definition value.
- Effective speed may be derived at runtime from damage and game rules.
- For example, damage to one of two engines might reduce the unit's effective maximum speed. The exact gameplay rule is separate from the static definition.

Engines are damageable components, not technical propulsion simulation components. Do not add thrust or power properties to engine definitions.

### Component categories

Components are divided into two categories:

1. **Damage-only components**
   - Examples: Engine, Propeller, Hull, Electric Generator and Communications.
   - Can be damaged.
   - Have no technical simulation properties in their definitions.
   - Their damage may influence unit performance through simulation rules.

2. **Simulation components**
   - Examples: Radar, AAA, SAM launcher, Gun, Torpedo launcher and Depth-charge launcher.
   - Can be damaged.
   - Have type-specific technical properties and simulation capabilities.

Example damage-only component definition:

```json
{
  "id": "marine_engine",
  "name": "Marine Engine",
  "category": "damage_only",
  "type": "engine"
}
```

Example simulation component definition:

```json
{
  "id": "radar_search",
  "name": "Search Radar",
  "category": "simulation",
  "type": "radar",
  "properties": {
    "range_nm": 120
  }
}
```

Do not add an empty or arbitrary `properties` object to damage-only definitions.

Do not invent technical properties for weapon systems or launchers before their requirements have been discussed.

### Component condition and operational state

Three discrete condition states have been agreed:

```cpp
enum class ComponentCondition
{
    Operational,
    Damaged,
    Destroyed
};
```

Condition is independent of whether a component is enabled.

For example:
- Operational and enabled.
- Operational and disabled.
- Damaged and enabled.
- Damaged and disabled.
- Destroyed.

A destroyed component cannot be repaired during battle. Repair would require a shipyard, which is outside the current simulation scope.

Damage state belongs to each individual installation, not to the shared component definition.

Radar has a separate operational state:

```cpp
enum class RadarOperationalState
{
    Enabled,
    Disabled
};
```

Do not introduce a generic enabled flag for damage-only installations.

The condition enum and runtime component data are already established. **Condition transitions and their gameplay consequences have not yet been implemented as a general simulation behaviour.**

### Ammunition

Ammunition is a separate definition category, not a component.

Initial ammunition definition properties:

- `id`
- `name`
- `range`
- `speed`
- `is_guided`
- `damage`

Example:

```json
{
  "id": "aim9_sidewinder",
  "name": "AIM-9 Sidewinder",
  "range": 18,
  "speed": 850,
  "is_guided": true,
  "damage": 80
}
```

The values are illustrative.

Ammunition definitions are reusable and referenced by runtime weapon inventories.

**Ammunition inventory belongs to an individual weapon installation.**

For example, a unit with two identical naval guns must be able to have:
- Gun 0 loaded with 200 HE shells.
- Gun 1 empty.

Or:
- Both guns loaded.
- Each gun carrying different ammunition types.
- Different quantities in each installation.

Do not put a single ammunition type or quantity on a shared weapon definition.

**Important rejected proposal:** Do not add an `accepted_ammunition` list, ammunition compatibility metadata or compatibility validation to weapon definitions. The scenario designer or modder is responsible for assigning suitable ammunition.

### Ammunition loadout configuration

Ammunition is assigned to individual weapon installations during unit assembly.

The agreed typed configuration model is:

```cpp
UnitAssemblyConfiguration configuration{
    .componentInstallations = {
        {
            DefinitionId("naval_gun"),
            0,
            {{DefinitionId("127mm_he_shell"), 200}}
        },
        {
            DefinitionId("naval_gun"),
            1,
            {}
        }
    }
};
```

An installation is identified by:
- Component definition ID.
- Zero-based installation index.

This avoids depending on Flecs child iteration order.

Rules:
- Omitted installation configuration means empty ammunition.
- An explicit empty ammunition list also means empty ammunition.
- Ammunition definitions must exist.
- Quantities must be positive.
- Duplicate configuration entries must be rejected.
- Unknown installation indices must be rejected.
- Do not perform ammunition compatibility validation.

The configuration is currently a typed simulation-side assembly input. Do not invent a JSON scenario schema yet.

**Current ECS representation detail:**
- An explicitly configured loadout, including an empty one, creates an `AmmunitionInventory` component.
- An omitted loadout does not create that component.
- Both represent an installation with no ammunition. Do not change this representation without considering the existing tests and the intended meaning of component absence.

---

## 4. Stage 5 — Data Definitions and Runtime Assembly

Stage 5 progressively generalized the definition and assembly architecture.

### Completed milestones

| Milestone | Description | Commit |
|---|---|---|
| 5.1 | Generic definition models | `fb6c83a` |
| 5.2 | Unit collection loading | `5f6c3f8` |
| 5.3 | Refine Unit and Component Definitions | `02a8308` |
| 5.4 | Fuel Definitions and Loading | `4cfde27` |
| 5.5 | Generalise Definition Collection Loading | `c8950a1` |
| 5.6 | Ammunition Collection Loading | `96893c9` |
| 5.7 | Component Collection Loading | `c2dedec` |
| 5.8 | Remove Legacy Vessel Definitions | `4c0fc6f` |
| 5.9 | Generalise Unit Creation and Movement | `d753065` |
| 5.10 | Generalise Simulation Test Terminology | `8b6c8b8` |
| 5.11 | Introduce UnitAssembler | `af5f35e` |
| 5.12 | Assemble Runtime Fuel and Component Installations | `d979c6a` |
| 5.13 | Assemble Runtime Ammunition Inventories | `7c60a25` |

Full commit hashes are available in the user's Git history.

### Latest completed milestone: 5.13

Commit:

`7c60a25fb9c3091bc4fbc4f8aee37f3bf64fbc1d`

Title:

`5.13 — Assemble Runtime Ammunition Inventories`

The `UnitAssembler` now supports:
- Resolving unit definitions.
- Resolving component definitions.
- Expanding component quantities into individual installations.
- Assigning sequential installation indices per component definition.
- Assembling runtime fuel and component installations.
- Assigning independent ammunition inventories to individual installations.
- Validating ammunition references and quantities before runtime entity creation.
- Rejecting duplicate loadout entries and invalid installation indices.
- Cleaning up the parent unit if a later ECS assembly operation throws. Successfully parented child installations are cleaned up through the Flecs parent-child relationship.

The implementation validates definition and ammunition references before creating runtime ECS entities.

An explicitly configured empty ammunition list creates an empty inventory component. An omitted loadout also represents an empty inventory, but currently does not create that component.

Latest verification:

**312 tests passed, 0 failed.**

### Exception-safety discussion after 5.13

A possible theoretical edge case was identified:

```cpp
auto installation = ecs.entity();
installation.child_of(unitId);
```

If an exception occurred after child creation but before successful parenting, the child might not be removed when the parent is destroyed.

The proposed 5.14 milestone was to add explicit entity tracking and rollback.

**Decision: Do not implement that milestone at present.**

The gap has not been shown to produce a realistic, reproducible failure. Adding tracking infrastructure and tests without a meaningful failure path would introduce complexity without demonstrating a practical benefit.

The existing implementation already validates inputs before entity creation and destroys the parent on assembly failure. Revisit explicit child tracking if future implementation introduces a genuine failure scenario that requires it.

Do not treat 5.14 exception-safety work as the next planned milestone.

---

## 5. Current Relevant Runtime Architecture

### UnitAssembler

Current known header:

`src/simulation/units/unit_assembler.hpp`

The public API includes:

```cpp
struct AmmunitionLoadoutEntry
{
    navalsim::data::DefinitionId ammunitionId;
    std::size_t quantity;
};

struct ComponentInstallationLoadout
{
    navalsim::data::DefinitionId componentDefinitionId;
    std::size_t installationIndex;
    std::vector<AmmunitionLoadoutEntry> ammunition;
};

struct UnitAssemblyConfiguration
{
    std::vector<ComponentInstallationLoadout> componentInstallations;
};

class UnitAssembler
{
public:
    flecs::entity_t assemble(
        const navalsim::data::DefinitionDatabase& definitions,
        World& world,
        const navalsim::data::DefinitionId& definitionId,
        navalsim::geography::GeographicPosition position,
        double initialHeadingDegrees = 0.0,
        const UnitAssemblyConfiguration& configuration = {}
    ) const;
};
```

`DefinitionDatabase` is forward-declared in the header.

Important namespace detail:
- `World` is a global class, not `navalsim::simulation::World`.
- Use the actual existing declaration and namespace conventions when modifying code.

### Runtime component definitions

Current known file:

`src/simulation/units/component_installation_components.hpp`

Contains:

```cpp
enum class ComponentCondition
{
    Operational,
    Damaged,
    Destroyed
};

struct ComponentInstallation
{
    std::string definitionId;
    std::size_t installationIndex = 0;
};

struct AmmunitionInventoryEntry
{
    std::string ammunitionDefinitionId;
    std::size_t quantity;
};

struct AmmunitionInventory
{
    std::vector<AmmunitionInventoryEntry> entries;
};

struct RadarConfiguration
{
    double rangeNm;
};

enum class RadarOperationalState
{
    Enabled,
    Disabled
};
```

Use this as contextual information, not as a substitute for inspecting the actual current files before editing.

---

## 6. Git History and Branch Structure

The user maintains clean, incremental Git history.

Stage 5 was merged from the Stage 4 data-definition branch.

Important merge commits:

- `d69fa99` — Merge branch `4-data-definition-and-loading`
- `03a8182` — Merge branch `3-simulation-command-architecture`
- `909b5a5` — Merge branch `2-command-movement`

The latest Stage 5 commits are:

```text
7c60a25  5.13 — Assemble Runtime Ammunition Inventories
d979c6a  5.12 — Assemble Runtime Fuel and Component Installations
af5f35e  5.11 — Introduce UnitAssembler
8b6c8b8  5.10 — Generalise Simulation Test Terminology
d753065  5.9 Generalise Unit Creation and Movement
4c0fc6f  5.8 — Remove Legacy Vessel Definitions
c2dedec  5.7 — Component Collection Loading
96893c9  5.6 — Ammunition Collection Loading
c8950a1  5.5 — Generalise Definition Collection Loading
4cfde27  5.4 — Fuel Definitions and Loading
02a8308  5.3 — Refine Unit and Component Definitions
5f6c3f8  5.2 — Unit Collection Loading
fb6c83a  5.1 — Generic definition models
```

The user commits each completed step as a development breadcrumb.

Do not combine several milestones into one commit.

Use sequential milestone numbering, e.g. `5.14`, `5.15`, `5.16`.

Do not append sub-number suffixes to commit titles.

---

## 7. Development Workflow and User Preferences

These instructions are especially important when acting as a coding agent.

### Inspect before changing

- Do not assume the current repository state.
- Do not pretend to have local repository access.
- The user runs commands locally and shares output.
- Inspect the relevant architecture and existing files before proposing changes.
- Agree on the target architecture before implementing substantial changes.
- Do not invent project structure or APIs.

### File inspection

When requesting file contents, provide **one ready-to-run Bash script** that prints all required files.

Preferred format:

```bash
files=(
    "src/path/file.hpp"
    "src/path/file.cpp"
)

for file in "${files[@]}"; do
    echo
    echo "============================================================"
    echo "FILE: $file"
    echo "============================================================"

    if [[ -f "$file" ]]; then
        cat "$file"
    else
        echo "ERROR: File not found"
    fi
done
```

Do not give several separate inspection commands when one script can do the job.

### Code changes

- For substantial changes, provide complete replacement contents for every file that needs modification.
- Always show full project-relative paths.
- Do not provide scattered snippets and ask the user to integrate them.
- Tiny, isolated changes may be described separately.
- Explain the reason for architectural changes and their consequences.
- Do not silently modify unrelated files.

### Testing

- Build after implementation.
- Run the full test suite, not just the new tests.
- Do not recommend committing until the full suite passes.
- The user reports build and test output.

Known test command:

```bash
ctest --test-dir build --output-on-failure
```

The project has CMake configure presets named `default` and `release`. Build preset availability has not been conclusively established in the current context. If necessary, inspect available build presets rather than assuming a preset name.

Keep using the existing `build/` directory.

### Git

- One milestone per commit.
- The user performs commits.
- Provide a suggested commit title after verification.
- Do not commit on the user's behalf.

### Other repository-specific instructions

- `README.md` is the user's personal notes. Do not modify it or ask the user to revert/commit changes to it.
- Keep `src/world/components/tags/entity_type_tags.hpp` as an unused placeholder/reminder. Do not delete it.
- Do not rename `"test-vessel"` IDs or `"Test Vessel"` labels in `tests/data`. They are valid naval test fixtures.
- Ignore `README.md`, `script.sh` and the tag placeholder during grep cleanup discussions.
- The user uses openSUSE Leap 16.
- Project directory: `~/Videos/harpoon/NavalSim`.

### Communication style

- Explain architectural reasoning clearly.
- Work incrementally.
- Avoid overwhelming the user with many unrelated tasks.
- Ask one direct question at a time if clarification is needed.
- Keep the user oriented with the current milestone, commit title and next step.
- Do not ask the user to repeat context already included in this document.

---

## 8. What Is Not Yet Established

Do not assume these features already exist:

- Scenario or mission definition/loading.
- JSON schema for per-installation ammunition loadouts.
- Weapon firing and ammunition consumption.
- Damage propagation and gameplay consequences.
- General component condition transition behaviour.
- Repair systems.
- Detailed weapon technical properties.
- Radar detection or tracking systems.
- Full simulation component behaviour.

Some of these may be logical future milestones, but they need architectural discussion and repository inspection before implementation.

---

## 9. Instructions for the Next Agent

The latest completed milestone is **5.13**. Treat commit `7c60a25fb9c3091bc4fbc4f8aee37f3bf64fbc1d` as the known baseline, while still checking the user's actual working tree before making changes.

Stage 5 has established a generic data-definition-to-runtime-assembly pipeline. Do not restart earlier work or assume another Stage 5 milestone is required.

### Likely next direction: Stage 6 — Runtime Simulation Behaviour

The next broad area of development is likely to be the behaviour of assembled runtime entities.

One candidate for an initial milestone is:

**6.1 — Introduce Component Condition Transitions**

The aim would be to establish how an individual installed component moves between the agreed condition states:

`Operational → Damaged → Destroyed`

This is only a candidate, not an approved implementation plan.

Before selecting it:

1. Inspect the current project structure.
2. Inspect the existing command architecture and simulation systems.
3. Inspect the current component installation runtime data and relevant tests.
4. Determine where state-changing simulation behaviour belongs.
5. Consider whether condition transitions should be driven by commands, systems, or another existing architectural pattern.
6. Identify the smallest useful, independently testable milestone.
7. Explain its purpose, scope, affected files and tests.
8. Agree on the target architecture and milestone with the user before implementing.

Do not introduce hit points, damage calculations, combat mechanics, repair systems or other gameplay features merely to implement condition transitions.

Do not assume Stage 6.1 must be component condition transitions if repository inspection reveals a more appropriate next step.

### General objective

Continue building a clean, generic, data-driven simulation foundation without prematurely introducing gameplay systems, rendering dependencies or unnecessary abstractions.

Preserve the separation between static definitions, runtime ECS state, simulation behaviour and presentation.
