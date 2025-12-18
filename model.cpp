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
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

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

    if(render_mode == 0) {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        //Texture binding
        std::cout << "redner texture : " << texture << " : " << modelManager.get_texture(texture) << std::endl;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, modelManager.get_texture(texture));
        glUniform1i(texture1Loc, 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, modelManager.get_texture(normal_texture));
        glUniform1i(texture2Loc, 1);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, count);
        glBindVertexArray(0);

        //Texture free
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if(render_mode == 1) {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, count);
        glBindVertexArray(0);
    }
    else {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glBindVertexArray(vao);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_DEPTH_TEST);

        glDrawArrays(GL_TRIANGLES, 0, count);

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-0.2f, -0.2f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.2f);

        glDrawArrays(GL_TRIANGLES, 0, count);
        
        glBindVertexArray(0);
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
        models[n] = loadModelData(dir+n+extend, n, t, nt);
        if(t != "")  textures[t] = loadTexture(dir+t+extend2);
        if(nt != "")  textures[nt] = loadTexture(dir+nt+extend2);
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    

    return textureID;
}