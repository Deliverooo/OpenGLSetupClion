#ifndef MODEL_H
#define MODEL_H
#include "assimp/mesh.h"
#include <vector>
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include <string>
#include "mesh.h"
#include "stb_image.h"


struct aiMaterial;
using namespace std;

std::vector<Texture> loadedTextures;

class Model {
    public:
        Model(string filepath) {
            loadModel(filepath);
        }
        void draw(Shader &shader) {
            for (GLuint i = 0; i < meshes.size(); i++) {
                meshes[i].draw(shader);
            }
        }
    private:

        std::string directory;
        std::vector<Mesh> meshes;

        GLuint loadTexture(const char* path, const std::string directory) {

            string filename = string(path);
            filename = directory + '/' + filename;

            GLuint textureID;
            glGenTextures(1, &textureID);

            int width, height, nrComponents;
            unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
            if (data)
            {
                GLenum format = 0;
                if (nrComponents == 1)
                    format = GL_RED;
                else if (nrComponents == 3)
                    format = GL_RGB;
                else if (nrComponents == 4)
                    format = GL_RGBA;

                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                stbi_image_free(data);
                std::cout << "Loaded texture with path: " << filename << std::endl;
            }
            else
            {
                std::cout << "Texture failed to load at path: " << path << std::endl;

                stbi_image_free(data);
            }

            return textureID;
        }
        std::vector<Texture> loadMaterialTex(aiMaterial *material, aiTextureType type, std::string typeName) {

            std::vector<Texture> textures;
            for (GLuint i = 0; i < material->GetTextureCount(type); i++) {
                aiString str;
                material->GetTexture(type, i, &str);
                Texture texture;
                texture.id = loadTexture(str.C_Str(), directory);
                texture.type = typeName;
                texture.path = str;
                textures.push_back(texture);
                std::cout << "Loaded texture with path: " << texture.path.C_Str() << std::endl;
            }
            return textures;
        }

        Mesh processMesh(aiMesh *mesh, const aiScene *scene) {

            //for the input mesh, it will create a list of vertices, indies and textures for it
            std::vector<Vertex> vertices;
            std::vector<GLuint> indices;
            std::vector<Texture> textures;

            for (GLuint i = 0; i < mesh->mNumVertices; i++) {
                Vertex vertex;

                glm::vec3 vector;
                vector.x = mesh->mVertices[i].x;
                vector.y = mesh->mVertices[i].y;
                vector.z = mesh->mVertices[i].z;
                vertex.Position = vector;

                glm::vec3 normal;
                normal.x = mesh->mNormals[i].x;
                normal.y = mesh->mNormals[i].y;
                normal.z = mesh->mNormals[i].z;
                vertex.Normal = normal;

                if (mesh->mTextureCoords[0]) {
                    glm::vec2 tex;
                    tex.x = mesh->mTextureCoords[0][i].x;
                    tex.y = mesh->mTextureCoords[0][i].y;
                    vertex.TexCoords = tex;
                } else {
                    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                }
                vertices.push_back(vertex);
            }

            for (GLuint i = 0; i < mesh->mNumFaces; i++) {

                aiFace face = mesh->mFaces[i];
                for (int j = 0; j < face.mNumIndices; j++) {
                    indices.push_back(face.mIndices[j]);
                }
            }

            if (mesh->mMaterialIndex >= 0) {
                aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
                std::vector<Texture> diffuseTextures = loadMaterialTex(material, aiTextureType_DIFFUSE, "diffuseTex");

                textures.insert(textures.end(), diffuseTextures.begin(), diffuseTextures.end());
                std::vector<Texture> specularTextures = loadMaterialTex(material, aiTextureType_SPECULAR, "specularTex");

                textures.insert(textures.end(), specularTextures.begin(), specularTextures.end());
            }
            return Mesh(vertices, indices, textures);
        }

        //this function takes in a node and recursively creates a mesh for each of its children, then adds it to the mesh
        void processNode(aiNode *node, const aiScene *scene) {

            for (GLuint i = 0; i < node->mNumMeshes; i++) {
                aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                meshes.push_back(processMesh(mesh, scene));
            }
            for (GLuint i = 0; i < node->mNumChildren; i++) {
                processNode(node->mChildren[i], scene);
            }

        }

        void loadModel(std::string filepath) {
            Assimp::Importer importer;
            const aiScene *scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
                return;
            }
            //calls the process node for the root node, meaning all subsequent meshes will be added to the
            //mesh vertex list
            directory = filepath.substr(0, filepath.find_last_of('/'));

            processNode(scene->mRootNode, scene);

        }
};
#endif //MODEL_H
