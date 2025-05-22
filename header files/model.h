#ifndef MODEL_H
#define MODEL_H
#include "include/assimp/mesh.h"
#include <vector>
#include "include/assimp/postprocess.h"
#include "include/assimp/scene.h"
#include "include/assimp/Importer.hpp"
#include <string>
#include "mesh.h"
#include "stb_image.h"


struct aiMaterial;
using namespace std;


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

        GLuint loadTexture(string path) {

            GLuint id;
            int width, height, nrChannels;

            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_2D, id);
            // set the texture wrapping parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // set texture filtering parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            stbi_set_flip_vertically_on_load(GL_TRUE);

            unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

            if (nrChannels == 3) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            } else if (nrChannels == 4) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }

            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);

            return id;
        }
        std::vector<Texture> loadMaterialTex(aiMaterial *material, aiTextureType type, std::string typeName) {

            std::vector<Texture> textures;
            for (GLuint i = 0; i < material->GetTextureCount(type); i++) {
                aiString str;
                material->GetTexture(type, i, &str);
                Texture texture;
                texture.id = loadTexture(str.C_Str());
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
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
                aiNode *child = node->mChildren[i];
                processNode(child, scene);
            }

        }

        void loadModel(std::string filepath) {
            Assimp::Importer importer;
            const aiScene *scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

            if (!scene) {
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
