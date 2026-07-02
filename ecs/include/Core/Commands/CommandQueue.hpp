#pragma once

#include <queue>
#include "GameCommand.hpp"

class CommandQueue {
private:
    std::queue<GameCommand> _queue;

public:
    /**
     * @brief Adds a new command to the queue.
     */
    void Enqueue(const GameCommand& cmd) { 
        _queue.push(cmd); 
    }

    /**
     * @brief Returns true if there are commands pending in the queue.
     */
    bool HasCommands() const { 
        return !_queue.empty(); 
    }

    /**
     * @brief Removes and returns the command at the front of the queue.
     */
    GameCommand Dequeue() {
        GameCommand cmd = _queue.front();
        _queue.pop();
        return cmd;
    }

    /**
     * @brief Clears all pending commands from the queue.
     */
    void Clear() { 
        while(!_queue.empty()) _queue.pop(); 
    }
};
