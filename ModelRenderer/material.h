#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Material
{
protected:
    glm::vec3 mColor;
    float mRoughness;
    float mMetallic;
    
public:
    Material(glm::vec3 color, float rough, float metal)
    {
        mColor = color;
        mRoughness = rough;
        mMetallic = metal;
    }

    void setColor(glm::vec3 color);
    void setRoughness(float r);
    void setMetallic(float m);
    glm::vec3 getColor() const { return mColor; }
    float getRoughness() const { return mRoughness; }
    float getMetallic() const { return mMetallic; }
};

#endif
