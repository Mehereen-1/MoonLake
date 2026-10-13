#ifndef STARS_H
#define STARS_H

#include <glad/glad.h>
#include <vector>

class Stars
{
public:
    Stars(unsigned int count = 320);
    ~Stars();

    void draw() const;

private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int starCount;
};

#endif // STARS_H
