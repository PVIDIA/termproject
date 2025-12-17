#include "model.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <filesystem>
#include <unordered_map>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ModelManager modelManager;
int render_mode = 0;

extern GLint texture1Loc;
extern GLint texture2Loc;

Model loadModelBoundary() {
    Model model = Model("Boundary", "", "");

    
    glm::vec3 posints[8] = {glm::vec3(-200.0, -100.0, -300.0), glm::vec3(200.0, -100.0, -300.0), glm::vec3(200.0, -100.0, 300.0), glm::vec3(-200.0, -100.0, 300.0),
                            glm::vec3(-200.0, 100.0, -300.0), glm::vec3(200.0, 100.0, -300.0), glm::vec3(200.0, 100.0, 300.0), glm::vec3(-200.0, 100.0, 300.0)
                        };

    glm::vec3 buffer[24] = {    posints[0], posints[1], posints[1], posints[2], posints[2], posints[3], posints[3], posints[0], 
                                posints[4], posints[5], posints[5], posints[6], posints[6], posints[7], posints[7], posints[4], 
                                posints[0], posints[4], posints[1], posints[5], posints[2], posints[6], posints[3], posints[7]
                            };
                        
    model.count = 24;

    glGenVertexArrays(1, &model.vao);
    glBindVertexArray(model.vao);

    glGenBuffers(1, &model.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, model.vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    return model;
}

void Model::draw() const {
    if(name == "Boundary") {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBindVertexArray(vao);
        glDrawArrays(GL_LINES, 0, count);
        glBindVertexArray(0);
        return;
    }

    if(render_mode > 2) render_mode %= 3;
    
    // Draw all meshes in the model
    for (const auto& mesh : meshes) {
        if(render_mode == 0) {
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // Texture binding
            if (!mesh.material.diffuseTexture.empty()) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, modelManager.get_texture(mesh.material.diffuseTexture));
                glUniform1i(texture1Loc, 0);
            }
            
            if (!mesh.material.normalTexture.empty()) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, modelManager.get_texture(mesh.material.normalTexture));
                glUniform1i(texture2Loc, 1);
            }

            glBindVertexArray(mesh.vao);
            glDrawArrays(GL_TRIANGLES, 0, mesh.count);
            glBindVertexArray(0);

            // Texture free
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else if(render_mode == 1) {
            glDisable(GL_CULL_FACE);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBindVertexArray(mesh.vao);
            glDrawArrays(GL_TRIANGLES, 0, mesh.count);
            glBindVertexArray(0);
        }
        else {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            glBindVertexArray(mesh.vao);

            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_DEPTH_TEST);

            glDrawArrays(GL_TRIANGLES, 0, mesh.count);

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glEnable(GL_POLYGON_OFFSET_LINE);
            glPolygonOffset(-0.2f, -0.2f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(1.2f);

            glDrawArrays(GL_TRIANGLES, 0, mesh.count);
            
            glBindVertexArray(0);
        }
    }
}

Model::Model(const std::string& n, const std::string& m, const std::string& nm) : name(n), texture(m), normal_texture(nm) {}

std::string ModelManager::loadModel(const std::string& n, const std::string& t, const std::string& nt) {
    std::string dir = "assets/";
    std::string extend2 = ".png";
    
    if(count(n) == 1) {
        return n;
    }
    if(n == "Boundary") {
        models[n] = loadModelBoundary();
    }
    else {
        std::cout << "load" << std::endl;
        // Use Assimp to load model (supports OBJ and many other formats)
        // Try multiple file formats in order of preference
        std::vector<std::string> extensions = {".fbx", ".gltf", ".glb", ".obj", ".dae", ".blend"};
        std::string modelPath;
        bool found = false;
        
        for (const auto& ext : extensions) {
            std::string testPath = dir + n + ext;
            std::cout << "  Trying: " << testPath << std::endl;
            if (std::filesystem::exists(testPath)) {
                modelPath = testPath;
                found = true;
                std::cout << "Found model: " << modelPath << std::endl;
                break;
            }
        }
        
        if (!found) {
            std::cerr << "Model file not found for: " << n << std::endl;
            std::cerr << "Tried extensions: ";
            for (const auto& ext : extensions) std::cerr << ext << " ";
            std::cerr << std::endl;
            return n;
        }
        
        // Portal models should not flip UVs since they use FBO textures
        bool flipUVs = (n != "portal1" && n != "portal2");
        models[n] = loadModelAssimp(modelPath, n, flipUVs);
        
        // Extract model directory for texture path resolution
        std::string modelDir = modelPath.substr(0, modelPath.find_last_of('/') + 1);
        
        // Load textures from Assimp material or use provided texture names
        for (auto& mesh : models[n].meshes) {
            // Load diffuse texture
            if (!mesh.material.diffuseTexture.empty()) {
                // Resolve texture path relative to model directory
                std::string texPath = modelDir + mesh.material.diffuseTexture;
                if (textures.find(mesh.material.diffuseTexture) == textures.end()) {
                    textures[mesh.material.diffuseTexture] = loadTexture(texPath);
                }
            } else if (t != "") {
                // Fallback to provided texture name
                mesh.material.diffuseTexture = t;
                if (textures.find(t) == textures.end()) {
                    textures[t] = loadTexture(dir+t+extend2);
                }
            }
            
            // Load normal texture
            if (!mesh.material.normalTexture.empty()) {
                // Resolve texture path relative to model directory
                std::string texPath = modelDir + mesh.material.normalTexture;
                if (textures.find(mesh.material.normalTexture) == textures.end()) {
                    textures[mesh.material.normalTexture] = loadTexture(texPath);
                }
            } else if (nt != "") {
                // Fallback to provided normal texture name
                mesh.material.normalTexture = nt;
                if (textures.find(nt) == textures.end()) {
                    textures[nt] = loadTexture(dir+nt+extend2);
                }
            }
        }
        
        // Update legacy texture fields for compatibility
        if (!models[n].meshes.empty()) {
            models[n].texture = models[n].meshes[0].material.diffuseTexture;
            models[n].normal_texture = models[n].meshes[0].material.normalTexture;
        }
    }
    return n;
}

GLuint loadTexture(const std::string& path) {
    std::cout << "loadTexture : " << path << std::endl;
    int width, height, channels;
    
    stbi_set_flip_vertically_on_load(true);  // PNG 좌표계 뒤집기
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load PNG: " << path << std::endl;
        return 0;
    }

    GLenum format;
    if (channels == 1)      format = GL_RED;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 텍스처 업로드
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);

    
    glGenerateMipmap(GL_TEXTURE_2D);

    // 텍스처 파라미터
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    

    return textureID;
}

// Load model using Assimp library
Model loadModelAssimp(const std::string& filePath, const std::string& name, bool flipUVs) {
    std::cout << "Loading model with Assimp: " << filePath << std::endl;
    
    Assimp::Importer importer;
    
    // Load model with appropriate postprocessing flags
    // aiProcess_Triangulate: convert all primitives to triangles
    // aiProcess_FlipUVs: flip texture coordinates (OpenGL convention) - optional for FBO textures
    // aiProcess_GenNormals: generate normals if missing
    // aiProcess_CalcTangentSpace: calculate tangent/bitangent for normal mapping
    unsigned int flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace;
    if (flipUVs) {
        flags |= aiProcess_FlipUVs;
    }
    
    const aiScene* scene = importer.ReadFile(filePath, flags);
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
        return Model();
    }
    
    Model model;
    model.name = name;
    model.scene = scene;  // Keep pointer for future animation use
    
    std::string directory = filePath.substr(0, filePath.find_last_of('/'));
    
    // Process all meshes in the scene
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* aiMesh = scene->mMeshes[i];
        Mesh mesh;
        
        // Build vertex data with layout: position(3) + normal(3) + tangent(3) + bitangent(3) + texcoord(2) = 14 floats
        std::vector<float> vertices;
        vertices.reserve(aiMesh->mNumVertices * 14);
        
        for (unsigned int v = 0; v < aiMesh->mNumVertices; v++) {
            // Position
            vertices.push_back(aiMesh->mVertices[v].x);
            vertices.push_back(aiMesh->mVertices[v].y);
            vertices.push_back(aiMesh->mVertices[v].z);
            
            // Normal
            if (aiMesh->HasNormals()) {
                vertices.push_back(aiMesh->mNormals[v].x);
                vertices.push_back(aiMesh->mNormals[v].y);
                vertices.push_back(aiMesh->mNormals[v].z);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);
            }
            
            // Tangent
            if (aiMesh->HasTangentsAndBitangents()) {
                vertices.push_back(aiMesh->mTangents[v].x);
                vertices.push_back(aiMesh->mTangents[v].y);
                vertices.push_back(aiMesh->mTangents[v].z);
            } else {
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
            
            // Bitangent
            if (aiMesh->HasTangentsAndBitangents()) {
                vertices.push_back(aiMesh->mBitangents[v].x);
                vertices.push_back(aiMesh->mBitangents[v].y);
                vertices.push_back(aiMesh->mBitangents[v].z);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
            }
            
            // Texture coordinates (use first UV channel)
            if (aiMesh->HasTextureCoords(0)) {
                vertices.push_back(aiMesh->mTextureCoords[0][v].x);
                vertices.push_back(aiMesh->mTextureCoords[0][v].y);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }
        
        // Flatten indices into vertex array (for indexed drawing later, we use simple approach now)
        std::vector<float> vertexData;
        for (unsigned int f = 0; f < aiMesh->mNumFaces; f++) {
            aiFace& face = aiMesh->mFaces[f];
            for (unsigned int idx = 0; idx < face.mNumIndices; idx++) {
                unsigned int vertexIndex = face.mIndices[idx];
                for (int j = 0; j < 14; j++) {
                    vertexData.push_back(vertices[vertexIndex * 14 + j]);
                }
            }
        }
        
        mesh.count = vertexData.size() / 14;
        
        // Create OpenGL buffers
        glGenVertexArrays(1, &mesh.vao);
        glBindVertexArray(mesh.vao);
        
        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
        
        // Vertex attributes: position(0), normal(1), tangent(2), bitangent(3), texcoord(4)
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // Tangent attribute (for future normal mapping improvements)
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        
        // Bitangent attribute
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(9 * sizeof(float)));
        glEnableVertexAttribArray(3);
        
        // Texture coordinate attribute
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(12 * sizeof(float)));
        glEnableVertexAttribArray(4);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        // Load material properties
        if (aiMesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
            
            aiColor3D ambient(0.2f, 0.2f, 0.2f);
            aiColor3D diffuse(0.8f, 0.8f, 0.8f);
            aiColor3D specular(0.5f, 0.5f, 0.5f);
            float shininess = 32.0f;
            
            material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
            material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
            material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
            material->Get(AI_MATKEY_SHININESS, shininess);
            
            mesh.material.ambient = glm::vec3(ambient.r, ambient.g, ambient.b);
            mesh.material.diffuse = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
            mesh.material.specular = glm::vec3(specular.r, specular.g, specular.b);
            mesh.material.shininess = shininess;
            
            // Load diffuse texture
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString texPath;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
                mesh.material.diffuseTexture = texPath.C_Str();
                std::cout << "  Diffuse texture: " << mesh.material.diffuseTexture << std::endl;
            }
            
            // Load normal map texture
            if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
                aiString texPath;
                material->GetTexture(aiTextureType_NORMALS, 0, &texPath);
                mesh.material.normalTexture = texPath.C_Str();
                std::cout << "  Normal texture: " << mesh.material.normalTexture << std::endl;
            } else if (material->GetTextureCount(aiTextureType_HEIGHT) > 0) {
                // Some formats use HEIGHT for normal maps
                aiString texPath;
                material->GetTexture(aiTextureType_HEIGHT, 0, &texPath);
                mesh.material.normalTexture = texPath.C_Str();
                std::cout << "  Normal texture (height): " << mesh.material.normalTexture << std::endl;
            }
        }
        
        model.meshes.push_back(mesh);
        
        std::cout << "  Mesh " << i << ": " << mesh.count << " vertices" << std::endl;
    }
    
    // Legacy compatibility: point to first mesh
    if (!model.meshes.empty()) {
        model.vao = model.meshes[0].vao;
        model.vbo = model.meshes[0].vbo;
        model.count = model.meshes[0].count;
        model.texture = model.meshes[0].material.diffuseTexture;
        model.normal_texture = model.meshes[0].material.normalTexture;
    }
    
    std::cout << "Loaded " << model.meshes.size() << " meshes from " << filePath << std::endl;
    return model;
}