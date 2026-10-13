#pragma once

struct Input
{
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
};

Input GetInput();
