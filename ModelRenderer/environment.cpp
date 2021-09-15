#include "environment.h"

Cubemap::Cubemap()
{
    
}

Cubemap::Cubemap(const char* vert, const char* frag)
{
    glGenFramebuffers(1, &fbo);
    glGenRenderbuffers(1, &rbo);
    glGenTextures(1, &hdr);
    glGenTextures(1, &id);

    pShader = new Shader(vert, frag);
    setupMatrices();
}

void Cubemap::loadHDR(const char* fname)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float* data = stbi_loadf(fname, &width, &height, &nrComponents, 0);

    if (data)
    {
        glBindTexture(GL_TEXTURE_2D, hdr);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << fname << " loaded" << std::endl;
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
    }
}

void Cubemap::loadEnvfromDirectory(std::string path, std::vector<std::string> &files, std::vector<std::string>& name, int& count)
{
    std::filesystem::path p(path);
    if (!std::filesystem::exists(p))
        std::cout << "Invalid path to environmental maps" << std::endl;
    std::filesystem::directory_iterator itr(p);

    count = 0;
    while (itr != std::filesystem::end(itr))
    {
        const std::filesystem::directory_entry& entry = *itr;
        //std::cout << entry.path() << std::endl;
        files.push_back(entry.path().string());
        name.push_back(entry.path().filename().string());
        itr++;
        count++;
    }
}

// set up projection and view matrices for capturing data onto the 6 cubemap face directions
void Cubemap::setupMatrices()
{
    projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    views[0] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f));
    views[1] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f));
    views[2] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    views[3] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,-1.0f, 0.0f), glm::vec3(0.0f, 0.0f,-1.0f));
    views[4] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, 0.0f, 1.0f), glm::vec3(0.0f,-1.0f, 0.0f));
    views[5] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, 0.0f,-1.0f), glm::vec3(0.0f,-1.0f, 0.0f));
}

// convert HDR equirectangular environment map to cubemap equivalent
void Cubemap::create()
{
    GLsizei CS = 512;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, CS, CS);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    
    //loadHDR("../../img/envs/Newport_Loft/Newport_Loft_Ref.hdr");

    // setup cubemap to render to and attach to framebuffer
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (unsigned int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, CS, CS, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    pShader->use();
    pShader->setInt("equirectangularMap", 0);
    pShader->setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr);
    
    glViewport(0, 0, CS, CS);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    for (unsigned int i = 0; i < 6; i++)
    {
        pShader->setMat4("view", views[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, id, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cube.render();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


Irradiancemap::Irradiancemap(const char* vert, const char* frag, Cubemap* p)
{
    // init
    pShader = new Shader(vert, frag);
    glGenTextures(1, &id);
    // set-ups
    pCubemap = p;
    //create();
}

// create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
void Irradiancemap::create()
{
    GLsizei IS = 32;
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, IS, IS, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, pCubemap->getFBO());
    glBindRenderbuffer(GL_RENDERBUFFER, pCubemap->getRBO());
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, IS, IS);

    pShader->use();
    pShader->setInt("environmentMap", 0);
    pShader->setMat4("projection", pCubemap->getProjection());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, pCubemap->getID());

    glViewport(0, 0, IS, IS); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, pCubemap->getFBO());
    for (unsigned int i = 0; i < 6; ++i)
    {
        pShader->setMat4("view", pCubemap->getViews(i));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, id, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cube.render();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Prefilteredmap::Prefilteredmap(const char* vert, const char* frag, Cubemap* p)
{
    pShader = new Shader(vert, frag);
    glGenTextures(1, &id);

    pCubemap = p;
    //create();
}

// create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
void Prefilteredmap::create()
{
    GLsizei PS = 128;
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, PS, PS, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter to mip_linear 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
    pShader->use();
    pShader->setInt("environmentMap", 0);
    pShader->setMat4("projection", pCubemap->getProjection());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, pCubemap->getID());

    glBindFramebuffer(GL_FRAMEBUFFER, pCubemap->getFBO());
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = PS * std::pow(0.5, mip);
        unsigned int mipHeight = PS * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, pCubemap->getRBO());
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        pShader->setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            pShader->setMat4("view", pCubemap->getViews(i));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, id, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            cube.render();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

BRDFmap::BRDFmap(const char* vert, const char* frag, Cubemap* p)
{
    pShader = new Shader(vert, frag);
    glGenTextures(1, &id);

    pCubemap = p;
    //create();
}

// create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
void BRDFmap::create()
{
    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
    glBindFramebuffer(GL_FRAMEBUFFER, pCubemap->getFBO());
    glBindRenderbuffer(GL_RENDERBUFFER, pCubemap->getRBO());
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0);

    glViewport(0, 0, 512, 512);
    pShader->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    quad.render();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}