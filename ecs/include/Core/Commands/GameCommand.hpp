#pragma once
#include <cstdint>

enum class CommandType : int32_t { Move, Attack, UpdateStats, EquipItem };

struct GameCommand {
    CommandType Type;
    int32_t EntityId;
    
    float VelocityX;
    float VelocityY;
    float Speed;

    union {
        struct {
            float PosX;
            float PosY;
        } MoveParams;

        struct {
            int32_t TargetId;
            float Value;
        } InteractionParams;
    };
};
