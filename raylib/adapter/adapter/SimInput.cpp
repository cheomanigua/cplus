#include "SimInput.hpp"

// The implementation file decides which backend to use
#if defined(BACKEND_RAYLIB)
    #include "RaylibInput.hpp"
#elif defined(BACKEND_GODOT)
    #include "GodotInput.hpp"
#endif

// Now you implement the function declared in the header
namespace SimEngine::InputAdapter {
    InputIntent PollInputs() noexcept {
        // This calls the actual backend function
        return BackendPollInputs(); 
    }
}
