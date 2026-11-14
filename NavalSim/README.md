cmake --preset default
cmake --build --preset debug
ctest --test-dir build --output-on-failure


./build/Debug/navalsim
./build/Debug/navalsim_clock_tests
./build/Debug/navalsim_scheduler_tests
ctest --test-dir build --output-on-failure
ctest --test-dir build -N


git merge --no-ff branchname


zip -r navalsim-5.2-review.zip . \
  -x ".git/*" "build/*" "cmake-build-*/*"

git apply --check navalsim-5.1.1.patch
git apply navalsim-5.1.1.patch

git diff --check
git status --short
git diff --stat

---------------

for file in \
    src/data/definitions/unit_definition.hpp \
    src/data/definitions/unit_definition.cpp \
    src/data/definitions/component_definition.hpp \
    src/data/definitions/component_definition.cpp \
    src/data/definitions/vessel_definition.hpp \
    src/data/definitions/vessel_definition.cpp
do
    echo
    echo "============================================================"
    echo "$file"
    echo "============================================================"
    cat "$file"
done

----------------

grep -RInE     "VesselDefinition|VesselPropulsion|VesselDimensions|VesselDefinitionLoader|UnitDefinition|ComponentDefinition|ComponentProperties|EngineProperties"     src tests

----------------

files=(
    "src/data/definitions/definition_batch_loader.hpp"
    "src/data/definitions/definition_loader_registry.hpp"
    "src/data/definitions/definition_loader.hpp"
    "src/data/definitions/unit_collection_loader.hpp"
    "tests/CMakeLists.txt"
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

---------------
