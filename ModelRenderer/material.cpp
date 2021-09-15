#include "material.h"

void Material::setColor(glm::vec3 color)
{
    mColor = color;
}

void Material::setMetallic(float m)
{
    mMetallic = m;
}

void Material::setRoughness(float r)
{
    mRoughness = r;
}