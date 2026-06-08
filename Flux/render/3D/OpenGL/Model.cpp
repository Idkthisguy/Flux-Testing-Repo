#include "Model.h"
#include "logic/Textureloader.h"
#include <filesystem>
#include <iostream>

namespace Flux
{

Model::~Model()
{
    for (auto &mesh : meshes)
    {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.EBO);
    }
}

void Model::Load()
{
    for (auto &mesh : meshes)
    {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
        glDeleteBuffers(1, &mesh.EBO);
    }
    meshes.clear();

    size_t fileSize;
    void *fileBuffer = SDL_LoadFile(path.c_str(), &fileSize);
    if (!fileBuffer)
    {
        std::cerr << "Failed to load model file: " << path << " - " << SDL_GetError() << std::endl;
        Output::addLog("MODEL ERROR: Failed to load model file: " + path);
        return;
    }

    std::string ext = std::filesystem::path(path).extension().string();
    if (!ext.empty() && ext[0] == '.')
        ext = ext.substr(1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    bool isFbx = (ext == "fbx");
    unsigned int importFlags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
                               aiProcess_PreTransformVertices | aiProcess_JoinIdenticalVertices |
                               aiProcess_OptimizeMeshes | aiProcess_RemoveRedundantMaterials;
    if (isFbx)
        importFlags |= aiProcess_FlipUVs;

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFileFromMemory(fileBuffer, fileSize, importFlags, ext.c_str());

    SDL_free(fileBuffer);

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
    {
        std::cerr << "ASSIMP: Could not parse buffer from " << path << "\n";
        Output::addLog("MODEL ERROR: Could not parse buffer from " + path);
        return;
    }

    std::filesystem::path modelDir = std::filesystem::path(path).parent_path();

    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh *aMesh = scene->mMeshes[i];

        std::vector<Vertex> verts;
        std::vector<unsigned int> indices;

        for (unsigned int j = 0; j < aMesh->mNumVertices; j++)
        {
            Vertex v;
            v.Position = {aMesh->mVertices[j].x, aMesh->mVertices[j].y, aMesh->mVertices[j].z};
            v.Normal = aMesh->HasNormals() ? glm::vec3(aMesh->mNormals[j].x, aMesh->mNormals[j].y, aMesh->mNormals[j].z)
                                           : glm::vec3(0.f, 1.f, 0.f);
            v.TexCoords = aMesh->mTextureCoords[0]
                              ? glm::vec2(aMesh->mTextureCoords[0][j].x, aMesh->mTextureCoords[0][j].y)
                              : glm::vec2(0.f);
            v.Tangent = aMesh->mTangents
                            ? glm::vec3(aMesh->mTangents[j].x, aMesh->mTangents[j].y, aMesh->mTangents[j].z)
                            : glm::vec3(1.f, 0.f, 0.f);
            v.Bitangent = aMesh->mBitangents
                              ? glm::vec3(aMesh->mBitangents[j].x, aMesh->mBitangents[j].y, aMesh->mBitangents[j].z)
                              : glm::vec3(0.f, 1.f, 0.f);
            verts.push_back(v);
        }

        for (unsigned int j = 0; j < aMesh->mNumFaces; j++)
        {
            const aiFace &f = aMesh->mFaces[j];
            for (unsigned int k = 0; k < f.mNumIndices; k++)
                indices.push_back(f.mIndices[k]);
        }

        if (verts.empty() || indices.empty())
            continue;

        Mesh myMesh;
        myMesh.indexCount = (unsigned int)indices.size();

        if (aMesh->mMaterialIndex < scene->mNumMaterials)
        {
            aiMaterial *mat = scene->mMaterials[aMesh->mMaterialIndex];

            int twoSidedVal = 0;
            mat->Get(AI_MATKEY_TWOSIDED, twoSidedVal);
            myMesh.twoSided = (twoSidedVal != 0);
            aiString texPath;
            if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
            {
                std::string rawPath = texPath.C_Str();

                const aiTexture *aitex = scene->GetEmbeddedTexture(rawPath.c_str());
                if (aitex)
                {

                    if (aitex->mHeight == 0)
                    {
                        stbi_set_flip_vertically_on_load(false);
                        int w, h, ch;

                        unsigned char *data = stbi_load_from_memory(
                            reinterpret_cast<const unsigned char *>(aitex->pcData), (int)aitex->mWidth, &w, &h, &ch, 4);

                        if (data)
                        {
                            unsigned int id;
                            glGenTextures(1, &id);
                            glBindTexture(GL_TEXTURE_2D, id);

                            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                            glGenerateMipmap(GL_TEXTURE_2D);

                            stbi_image_free(data);

                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                            myMesh.textureID = id;
                            myMesh.hasAlpha = true;
                        }
                    }
                    else
                    {

                        int w = (int)aitex->mWidth;
                        int h = (int)aitex->mHeight;
                        unsigned int id;
                        glGenTextures(1, &id);
                        glBindTexture(GL_TEXTURE_2D, id);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, aitex->pcData);
                        glGenerateMipmap(GL_TEXTURE_2D);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        myMesh.textureID = id;
                    }
                }
                else
                {

                    std::filesystem::path fullTex = modelDir / rawPath;
                    std::string texStr = fullTex.string();
                    std::replace(texStr.begin(), texStr.end(), '\\', '/');

                    if (!std::filesystem::exists(texStr))
                    {

                        std::string justName = std::filesystem::path(rawPath).filename().string();
                        texStr = (modelDir / justName).string();
                        std::replace(texStr.begin(), texStr.end(), '\\', '/');
                    }

                    myMesh.textureID = TextureLoader::Load(texStr);

                    std::string lext = std::filesystem::path(texStr).extension().string();
                    std::transform(lext.begin(), lext.end(), lext.begin(), ::tolower);
                    if (lext == ".png" || lext == ".tga" || lext == ".dds" || lext == ".webp")
                        myMesh.hasAlpha = true;
                }
            }

            float opacity = 1.0f;
            mat->Get(AI_MATKEY_OPACITY, opacity);
            if (opacity < 1.0f)
                myMesh.hasAlpha = true;

            if (myMesh.hasAlpha && !myMesh.twoSided)
                myMesh.twoSided = true;

            if (myMesh.textureID == 0)
            {
                aiColor3D col(0.8f, 0.4f, 0.1f);
                mat->Get(AI_MATKEY_COLOR_DIFFUSE, col);
                myMesh.matColor = glm::vec3(col.r, col.g, col.b);
                myMesh.hasMtlColor = true;
            }
        }

        glGenVertexArrays(1, &myMesh.VAO);
        glGenBuffers(1, &myMesh.VBO);
        glGenBuffers(1, &myMesh.EBO);
        glBindVertexArray(myMesh.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, myMesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myMesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Position));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));

        glBindVertexArray(0);

        myMesh.verticies = verts;
        myMesh.indices = indices;

        meshes.push_back(myMesh);
    }
}

void Model::Draw(float /*alphaOverride*/)
{
    for (auto &mesh : meshes)
    {
        if (mesh.textureID != 0)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.textureID);
        }
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
}

void Model::SetTexture(unsigned int texID)
{
    for (auto &m : meshes)
        m.textureID = texID;
}

} // namespace Flux