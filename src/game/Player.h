#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "../node.h"
#include "../portal.h"

#include <vector>

class Player : public ObjectNode {
public:
    Player();
    void update();
    void render();
};

#endif