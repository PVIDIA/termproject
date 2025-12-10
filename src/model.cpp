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

struct VertaxData {
    int v;
    int vn;
    int vt;
    VertaxData(int a, int b, int c) : v(a), vn(b), vt(c) {} 
};

std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<glm::vec3> coordinates;
std::vector<std::vector<VertaxData>> faces;

std::vector<float> vertax_array;
std::vector<glm::vec3*> allocated_vertices;
std::vector<VertaxData> tess_array;
GLenum tess_type;

void beginCallback(GLenum type) {
    // type = GL_TRIANGLES, GL_TRIANGLE_FAN 등
    tess_type = type;
    tess_array.clear();

    //if(type == GL_TRIANGLES) std::cout << "GL_TRIANGLES" << std::endl;
    //else if(type == GL_TRIANGLE_FAN) std::cout << "GL_TRIANGLE_FAN" << std::endl;
    //else if(type == GL_TRIANGLE_STRIP) std::cout << "GL_TRIANGLE_STRIP" << std::endl;
}

void vertexCallback(void* vertexData) {
    VertaxData* data = (VertaxData*)vertexData;
    
    tess_array.push_back(*data);
}

void endCallback() {
    //std::cout << "- End primitive -" << std::endl;
    if(tess_type == GL_TRIANGLES) {
        for(auto& data : tess_array) {
            vertax_array.push_back(vertices[data.v].x);
            vertax_array.push_back(vertices[data.v].y);
            vertax_array.push_back(vertices[data.v].z);
            
            vertax_array.push_back(normals[data.vn].x);
            vertax_array.push_back(normals[data.vn].y);
            vertax_array.push_back(normals[data.vn].z);
            
            vertax_array.push_back(coordinates[data.vt].x);
            vertax_array.push_back(coordinates[data.vt].y);
        }
    }
    else if(tess_type == GL_TRIANGLE_FAN) {
        //std::cout << "tess_array size : " << tess_array.size() << std::endl;
        int count = tess_array.size();
        for(int i=1;i<count-1;i++) {
            // v0
            auto data = tess_array[0];
            vertax_array.push_back(vertices[data.v].x);
            vertax_array.push_back(vertices[data.v].y);
            vertax_array.push_back(vertices[data.v].z);
            
            vertax_array.push_back(normals[data.vn].x);
            vertax_array.push_back(normals[data.vn].y);
            vertax_array.push_back(normals[data.vn].z);
            
            vertax_array.push_back(coordinates[data.vt].x);
            vertax_array.push_back(coordinates[data.vt].y);

            // vi
            data = tess_array[i];
            vertax_array.push_back(vertices[data.v].x);
            vertax_array.push_back(vertices[data.v].y);
            vertax_array.push_back(vertices[data.v].z);
            
            vertax_array.push_back(normals[data.vn].x);
            vertax_array.push_back(normals[data.vn].y);
            vertax_array.push_back(normals[data.vn].z);
            
            vertax_array.push_back(coordinates[data.vt].x);
            vertax_array.push_back(coordinates[data.vt].y);
            
            // vi+1
            data = tess_array[i+1];
            vertax_array.push_back(vertices[data.v].x);
            vertax_array.push_back(vertices[data.v].y);
            vertax_array.push_back(vertices[data.v].z);
            
            vertax_array.push_back(normals[data.vn].x);
            vertax_array.push_back(normals[data.vn].y);
            vertax_array.push_back(normals[data.vn].z);
            
            vertax_array.push_back(coordinates[data.vt].x);
            vertax_array.push_back(coordinates[data.vt].y);
        }
    }
    else if(tess_type == GL_TRIANGLE_STRIP) {
        std::cout << "tess_array size : " << tess_array.size() << std::endl;
        int count = tess_array.size();
        for(int i=0;i<count-2;i++) {
            if (i % 2 == 0) {
                // vi
                auto data = tess_array[i];
                vertax_array.push_back(vertices[data.v].x);
                vertax_array.push_back(vertices[data.v].y);
                vertax_array.push_back(vertices[data.v].z);
                
                vertax_array.push_back(normals[data.vn].x);
                vertax_array.push_back(normals[data.vn].y);
                vertax_array.push_back(normals[data.vn].z);
                
                vertax_array.push_back(coordinates[data.vt].x);
                vertax_array.push_back(coordinates[data.vt].y);

                // vi+1
                data = tess_array[i+1];
                vertax_array.push_back(vertices[data.v].x);
                vertax_array.push_back(vertices[data.v].y);
                vertax_array.push_back(vertices[data.v].z);
                
                vertax_array.push_back(normals[data.vn].x);
                vertax_array.push_back(normals[data.vn].y);
                vertax_array.push_back(normals[data.vn].z);
                
                vertax_array.push_back(coordinates[data.vt].x);
                vertax_array.push_back(coordinates[data.vt].y);
                
                // vi+2
                data = tess_array[i+2];
                vertax_array.push_back(vertices[data.v].x);
                vertax_array.push_back(vertices[data.v].y);
                vertax_array.push_back(vertices[data.v].z);
                
                vertax_array.push_back(normals[data.vn].x);
                vertax_array.push_back(normals[data.vn].y);
                vertax_array.push_back(normals[data.vn].z);
                
                vertax_array.push_back(coordinates[data.vt].x);
                vertax_array.push_back(coordinates[data.vt].y);
            } else {
                // vi+1
                auto data = tess_array[i+1];
                vertax_array.push_back(vertices[data.v].x);
                vertax_array.push_back(vertices[data.v].y);
                vertax_array.push_back(vertices[data.v].z);
                
                vertax_array.push_back(normals[data.vn].x);
                vertax_array.push_back(normals[data.vn].y);
                vertax_array.push_back(normals[data.vn].z);
                
                vertax_array.push_back(coordinates[data.vt].x);
                vertax_array.push_back(coordinates[data.vt].y);

                // vi
                data = tess_array[i];
                vertax_array.push_back(vertices[data.v].x);
                vertax_array.push_back(vertices[data.v].y);
                vertax_array.push_back(vertices[data.v].z);
                
                vertax_array.push_back(normals[data.vn].x);
                vertax_array.push_back(normals[data.vn].y);
                vertax_array.push_back(normals[data.vn].z);
                
                vertax_array.push_back(coordinates[data.vt].x);
                vertax_array.push_back(coordinates[data.vt].y);
                
                // vi+2
                data = tess_array[i+2];
                vertax_array.push_back(vertices[data.v].x);
                vertax_array.push_back(vertices[data.v].y);
                vertax_array.push_back(vertices[data.v].z);
                
                vertax_array.push_back(normals[data.vn].x);
                vertax_array.push_back(normals[data.vn].y);
                vertax_array.push_back(normals[data.vn].z);
                
                vertax_array.push_back(coordinates[data.vt].x);
                vertax_array.push_back(coordinates[data.vt].y);
            }
        }
    }
    else {
        std::cout << "Undefiend tess type!!!!" << std::endl;
        exit(1);
    }
    tess_array.clear();
    //std::cout << "----------------" << std::endl;
}

void errorCallback(GLenum errorCode) {
    const GLubyte* errorStr = gluErrorString(errorCode);
    std::cerr << "Tessellation Error: " << errorStr << std::endl;
}

void combineCallback(GLdouble newCoord[3], GLdouble* neighborVertexData[4], GLfloat neighborWeight[4], GLdouble **outData) {
    // 새로운 glm::vec3 동적 생성
    glm::vec3* v = new glm::vec3(newCoord[0], newCoord[1], newCoord[2]);

    std::cout << "Combine callback: new vertex = ("
              << v->x << ", " << v->y << ", " << v->z << ")\n";

    allocated_vertices.push_back(v);
    *outData = (GLdouble*)v; // GLU Tessellator에게 새 정점 전달
}

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

Model loadModelData(const std::string& filePath, const std::string& name, const std::string& texture, const std::string& nm) {
    std::cout << "Start to load : " << filePath << std::endl;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Fail to open the file" << std::endl;
        exit(1);
    }

    /* load obj data */
    vertices.clear();
    normals.clear();
    coordinates.clear();
    faces.clear();

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string lineHeader;
        ss >> lineHeader;

        if (lineHeader == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            vertices.push_back(glm::vec3(x,y,z));
        }
        else if (lineHeader == "vn") {
            float x, y, z;
            ss >> x >> y >> z;
            normals.push_back(glm::vec3(x,y,z));
        }
        else if (lineHeader == "vt") {
            float x, y, z;
            ss >> x >> y;
            coordinates.push_back(glm::vec3(x,y,0.0));
        }
        else if (lineHeader == "f") {
            std::string word; // "int/int/int"
            faces.push_back(std::vector<VertaxData>{});
            auto& current_face = faces.back();

            while(ss >> word) {
                std::replace(word.begin(), word.end(), '/', ' ');
                std::istringstream word_ss(word); // "int int int"

                int v, vn, vt;
                word_ss >> v >> vt >> vn;
                current_face.push_back(VertaxData(v-1, vn-1, vt-1));
            }
        }
    }
    /*
    for(auto& face : faces) {
        std::cout << "=== face ===" << std::endl;
        for(auto& data : face) {
            std::cout << data.v << ", " << data.vn << ", " << data.vt << std::endl;
        }
        std::cout << "============" << std::endl;
    }*/
    /* tesselate init */
    GLUtesselator *tess = gluNewTess();

    gluTessCallback(tess, GLU_TESS_BEGIN, (void(*)())beginCallback);
    gluTessCallback(tess, GLU_TESS_VERTEX, (void(*)())vertexCallback);
    gluTessCallback(tess, GLU_TESS_END, (void(*)())endCallback);
    gluTessCallback(tess, GLU_TESS_ERROR, (void(*)())errorCallback);
    gluTessCallback(tess, GLU_TESS_COMBINE,(void(*)())combineCallback);

    vertax_array.clear();
    /* vertax array, tesselate */
    for(auto& face : faces) {
        switch(face.size()) {
            case 0:
            case 1:
            case 2:
                std::cout << "face vertices count are less than 3" << std::endl;
                break;
            case 3:
                for(auto& data : face) {
                    vertax_array.push_back(vertices[data.v].x);
                    vertax_array.push_back(vertices[data.v].y);
                    vertax_array.push_back(vertices[data.v].z);
                    
                    vertax_array.push_back(normals[data.vn].x);
                    vertax_array.push_back(normals[data.vn].y);
                    vertax_array.push_back(normals[data.vn].z);
                    
                    vertax_array.push_back(coordinates[data.vt].x);
                    vertax_array.push_back(coordinates[data.vt].y);
                }
                break;
            default:
                gluTessBeginPolygon(tess, nullptr);
                gluTessBeginContour(tess);
                
                //std::cout << "=================" <<std::endl;
                for(auto& data : face) {
                    GLdouble coords[3] = {vertices[data.v].x, vertices[data.v].y, vertices[data.v].z};
                    //std::cout << "(" << coords[0] << ", " << coords[1] << ", " << coords[2] << ")" <<std::endl;
                    gluTessVertex(tess, coords, (void*)&data); //data는 VertaxData&*로
                }
                //std::cout << "=================" <<std::endl;

                gluTessEndContour(tess);
                gluTessEndPolygon(tess);
        }
    }

    for(auto& v : allocated_vertices) {
        delete v; 
        v = nullptr;
    }
    allocated_vertices.clear();
    allocated_vertices.shrink_to_fit();
    gluDeleteTess(tess);

    //vao, vbo
    std::cout << "vertax_array : " << vertax_array.size() << std::endl;
    std::cout << "vertax count : " << vertax_array.size()/8 << std::endl;
    std::cout << "tri count : " << vertax_array.size()/24 << std::endl;

    Model model = Model(name, texture, nm);
    model.count = vertax_array.size()/8;

    glGenVertexArrays(1, &model.vao);
    glBindVertexArray(model.vao);

    glGenBuffers(1, &model.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, model.vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertax_array.size(), vertax_array.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    vertax_array.clear();
    vertax_array.shrink_to_fit();

    std::cout << "load end" << std::endl;
    return model;
}

Model::Model(const std::string& n, const std::string& m, const std::string& nm) : name(n), texture(m), normal_texture(nm) {}

std::string ModelManager::loadModel(const std::string& n, const std::string& t, const std::string& nt) {
    std::string dir = "assets/";
    std::string extend = ".obj";
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
        // Portal models should not flip UVs since they use FBO textures
        bool flipUVs = (n != "portal1" && n != "portal2");
        models[n] = loadModelAssimp(dir+n+extend, n, flipUVs);
        
        // Load textures from Assimp material or use provided texture names
        for (auto& mesh : models[n].meshes) {
            // Load diffuse texture
            if (!mesh.material.diffuseTexture.empty()) {
                std::string texPath = dir + mesh.material.diffuseTexture;
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
                std::string texPath = dir + mesh.material.normalTexture;
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