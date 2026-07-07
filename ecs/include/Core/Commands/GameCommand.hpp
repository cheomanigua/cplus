#pragma once
#include <cstdint>
#include <raylib.h> // Provides Vector2

enum class CommandType : int32_t { Move, Stop, Attack, UpdateStats, EquipItem };

struct GameCommand {
    CommandType Type;
    int32_t EntityId;
    
    // Use Vector2 for velocity and speed properties
    Vector2 Velocity; 
    float Speed;

    // Use a union to keep the struct size small and efficient
    union {
        struct {
            Vector2 Position;
            Vector2 TargetPosition;
        } MoveParams;

        struct {
            int32_t TargetId;
            float Value;
        } InteractionParams;
    };
};
