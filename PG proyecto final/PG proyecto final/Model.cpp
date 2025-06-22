#include "Model.h"

// Constructor - loads model from file with custom transform parameters
Model::Model(const char* file, glm::vec3 customScale, glm::vec3 customTranslation, glm::quat customRotation)
    : modelScale(customScale), modelTranslation(customTranslation), modelRotation(customRotation), file(file)
{
    std::string text = get_file_contents(file);
    JSON = json::parse(text);
    data = getData();
    traverseNode(0);
}

// Sets new scale for the model and rebuilds transformation matrices
void Model::SetScale(glm::vec3 newScale)
{
    modelScale = newScale;
    // Recalculate all matrices with new scale
    matricesMeshes.clear();
    traverseNode(0);
}

// Sets new rotation for the model and rebuilds transformation matrices
void Model::SetRotation(glm::quat newRotation)
{
    modelRotation = newRotation;
    // Recalculate all matrices with new rotation
    matricesMeshes.clear();
    traverseNode(0);
}

// Draws all meshes in the model with given shader and camera
void Model::Draw(Shader& shader, Camera& camera)
{
    // Iterate through all meshes and draw each one
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].Mesh::Draw(shader, camera, matricesMeshes[i]);
    }
}

// Loads mesh data from GLTF/GLB file at specified index
void Model::loadMesh(unsigned int indMesh)
{
    // Get accessor indices for vertex attributes
    unsigned int posAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["POSITION"];
    unsigned int normalAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["NORMAL"];
    unsigned int texAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["TEXCOORD_0"];
    unsigned int indAccInd = JSON["meshes"][indMesh]["primitives"][0]["indices"];

    // Extract vertex data using accessor indices
    std::vector<float> posVec = getFloats(JSON["accessors"][posAccInd]);
    std::vector<glm::vec3> positions = groupFloatsVec3(posVec);
    std::vector<float> normalVec = getFloats(JSON["accessors"][normalAccInd]);
    std::vector<glm::vec3> normals = groupFloatsVec3(normalVec);
    std::vector<float> texVec = getFloats(JSON["accessors"][texAccInd]);
    std::vector<glm::vec2> texUVs = groupFloatsVec2(texVec);

    // Combine vertex components and get indices/textures
    std::vector<Vertex> vertices = assembleVertices(positions, normals, texUVs);
    std::vector<GLuint> indices = getIndices(JSON["accessors"][indAccInd]);
    std::vector<Texture> textures = getTextures();

    // Create mesh from extracted data
    meshes.push_back(Mesh(vertices, indices, textures));
}

// Recursively traverses node hierarchy and builds transformation matrices
void Model::traverseNode(unsigned int nextNode, glm::mat4 matrix)
{
    // Current node data
    json node = JSON["nodes"][nextNode];

    // Get translation (combine with model's custom translation)
    glm::vec3 translation = modelTranslation; // Default to custom translation
    if (node.find("translation") != node.end())
    {
        float transValues[3];
        for (unsigned int i = 0; i < node["translation"].size(); i++)
            transValues[i] = (node["translation"][i]);
        translation = glm::make_vec3(transValues) + modelTranslation;
    }

    // Get rotation (combine with model's custom rotation)
    glm::quat rotation = modelRotation; // Default to custom rotation
    if (node.find("rotation") != node.end())
    {
        float rotValues[4] = { node["rotation"][3], node["rotation"][0], node["rotation"][1], node["rotation"][2] };
        glm::quat nodeRotation = glm::normalize(glm::make_quat(rotValues));
        rotation = modelRotation * nodeRotation; // Combine rotations
    }

    // Get scale (combine with model's custom scale)
    glm::vec3 scale = modelScale; // Start with model scale
    if (node.find("scale") != node.end())
    {
        float scaleValues[3];
        for (unsigned int i = 0; i < node["scale"].size(); i++)
            scaleValues[i] = (node["scale"][i]);
        scale = glm::make_vec3(scaleValues) * modelScale;
    }

    // Get matrix if explicitly defined
    glm::mat4 matNode = glm::mat4(1.0f);
    if (node.find("matrix") != node.end())
    {
        float matValues[16];
        for (unsigned int i = 0; i < node["matrix"].size(); i++)
            matValues[i] = (node["matrix"][i]);
        matNode = glm::make_mat4(matValues);
    }

    // Initialize transformation matrices
    glm::mat4 trans = glm::mat4(1.0f);
    glm::mat4 rot = glm::mat4(1.0f);
    glm::mat4 sca = glm::mat4(1.0f);

    // Build transformation matrices
    trans = glm::translate(trans, translation);
    rot = glm::mat4_cast(rotation);
    sca = glm::scale(sca, scale);

    // Combine all transformations
    glm::mat4 matNextNode = matrix * matNode * trans * rot * sca;

    // Load mesh if node contains one
    if (node.find("mesh") != node.end())
    {
        translationsMeshes.push_back(translation);
        rotationsMeshes.push_back(rotation);
        scalesMeshes.push_back(scale);
        matricesMeshes.push_back(matNextNode);

        loadMesh(node["mesh"]);
    }

    // Recursively process child nodes
    if (node.find("children") != node.end())
    {
        for (unsigned int i = 0; i < node["children"].size(); i++)
            traverseNode(node["children"][i], matNextNode);
    }
}

// Extracts binary data from GLTF/GLB file
std::vector<unsigned char> Model::getData()
{
    std::string bytesText;
    std::string uri = JSON["buffers"][0]["uri"];

    // Get path to binary file
    std::string fileStr = std::string(file);
    std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);
    bytesText = get_file_contents((fileDirectory + uri).c_str());

    // Convert text data to bytes
    std::vector<unsigned char> data(bytesText.begin(), bytesText.end());
    return data;
}

// Extracts float values from accessor
std::vector<float> Model::getFloats(json accessor)
{
    std::vector<float> floatVec;

    // Get accessor properties
    unsigned int buffViewInd = accessor.value("bufferView", 1);
    unsigned int count = accessor["count"];
    unsigned int accByteOffset = accessor.value("byteOffset", 0);
    std::string type = accessor["type"];

    // Get bufferView properties
    json bufferView = JSON["bufferViews"][buffViewInd];
    unsigned int byteOffset = bufferView["byteOffset"];

    // Determine number of components per vertex
    unsigned int numPerVert;
    if (type == "SCALAR") numPerVert = 1;
    else if (type == "VEC2") numPerVert = 2;
    else if (type == "VEC3") numPerVert = 3;
    else if (type == "VEC4") numPerVert = 4;
    else throw std::invalid_argument("Type is invalid (not SCALAR, VEC2, VEC3, or VEC4)");

    // Extract float values from binary data
    unsigned int beginningOfData = byteOffset + accByteOffset;
    unsigned int lengthOfData = count * 4 * numPerVert;
    for (unsigned int i = beginningOfData; i < beginningOfData + lengthOfData; i += 4)
    {
        unsigned char bytes[] = { data[i], data[i + 1], data[i + 2], data[i + 3] };
        float value;
        std::memcpy(&value, bytes, sizeof(float));
        floatVec.push_back(value);
    }

    return floatVec;
}

// Extracts indices from accessor
std::vector<GLuint> Model::getIndices(json accessor)
{
    std::vector<GLuint> indices;

    // Get accessor properties
    unsigned int buffViewInd = accessor.value("bufferView", 0);
    unsigned int count = accessor["count"];
    unsigned int accByteOffset = accessor.value("byteOffset", 0);
    unsigned int componentType = accessor["componentType"];

    // Get bufferView properties
    json bufferView = JSON["bufferViews"][buffViewInd];
    unsigned int byteOffset = bufferView.value("byteOffset", 0);

    // Extract indices based on component type
    unsigned int beginningOfData = byteOffset + accByteOffset;
    if (componentType == 5125) // GL_UNSIGNED_INT
    {
        for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 4; i += 4)
        {
            unsigned char bytes[] = { data[i], data[i + 1], data[i + 2], data[i + 3] };
            unsigned int value;
            std::memcpy(&value, bytes, sizeof(unsigned int));
            indices.push_back((GLuint)value);
        }
    }
    else if (componentType == 5123) // GL_UNSIGNED_SHORT
    {
        for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i += 2)
        {
            unsigned char bytes[] = { data[i], data[i + 1] };
            unsigned short value;
            std::memcpy(&value, bytes, sizeof(unsigned short));
            indices.push_back((GLuint)value);
        }
    }
    else if (componentType == 5122) // GL_SHORT
    {
        for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i += 2)
        {
            unsigned char bytes[] = { data[i], data[i + 1] };
            short value;
            std::memcpy(&value, bytes, sizeof(short));
            indices.push_back((GLuint)value);
        }
    }

    return indices;
}

// Loads textures from model file
std::vector<Texture> Model::getTextures()
{
    std::vector<Texture> textures;

    std::string fileStr = std::string(file);
    std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);

    // Process all images in model
    for (unsigned int i = 0; i < JSON["images"].size(); i++)
    {
        std::string texPath = JSON["images"][i]["uri"];

        // Check if texture is already loaded
        bool skip = false;
        for (unsigned int j = 0; j < loadedTexName.size(); j++)
        {
            if (loadedTexName[j] == texPath)
            {
                textures.push_back(loadedTex[j]);
                skip = true;
                break;
            }
        }

        if (!skip)
        {
            // Load diffuse texture
            if (texPath.find("baseColor") != std::string::npos)
            {
                Texture diffuse = Texture((fileDirectory + texPath).c_str(), "diffuse", loadedTex.size());
                textures.push_back(diffuse);
                loadedTex.push_back(diffuse);
                loadedTexName.push_back(texPath);
            }
            // Load specular texture
            else if (texPath.find("metallicRoughness") != std::string::npos)
            {
                Texture specular = Texture((fileDirectory + texPath).c_str(), "specular", loadedTex.size());
                textures.push_back(specular);
                loadedTex.push_back(specular);
                loadedTexName.push_back(texPath);
            }
        }
    }

    return textures;
}

// Combines vertex components into Vertex structures
std::vector<Vertex> Model::assembleVertices(
    std::vector<glm::vec3> positions,
    std::vector<glm::vec3> normals,
    std::vector<glm::vec2> texUVs)
{
    std::vector<Vertex> vertices;
    for (int i = 0; i < positions.size(); i++)
    {
        vertices.push_back(
            Vertex{
                positions[i],
                normals[i],
                glm::vec3(1.0f, 1.0f, 1.0f), // Default color (white)
                texUVs[i]
            }
        );
    }
    return vertices;
}

// Groups float arrays into vec2 vectors
std::vector<glm::vec2> Model::groupFloatsVec2(std::vector<float> floatVec)
{
    const unsigned int floatsPerVector = 2;

    std::vector<glm::vec2> vectors;
    for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector)
    {
        vectors.push_back(glm::vec2(0, 0));

        for (unsigned int j = 0; j < floatsPerVector; j++)
        {
            vectors.back()[j] = floatVec[i + j];
        }
    }
    return vectors;
}

// Groups float arrays into vec3 vectors
std::vector<glm::vec3> Model::groupFloatsVec3(std::vector<float> floatVec)
{
    const unsigned int floatsPerVector = 3;

    std::vector<glm::vec3> vectors;
    for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector)
    {
        vectors.push_back(glm::vec3(0, 0, 0));

        for (unsigned int j = 0; j < floatsPerVector; j++)
        {
            vectors.back()[j] = floatVec[i + j];
        }
    }
    return vectors;
}

// Groups float arrays into vec4 vectors
std::vector<glm::vec4> Model::groupFloatsVec4(std::vector<float> floatVec)
{
    const unsigned int floatsPerVector = 4;

    std::vector<glm::vec4> vectors;
    for (unsigned int i = 0; i < floatVec.size(); i += floatsPerVector)
    {
        vectors.push_back(glm::vec4(0, 0, 0, 0));

        for (unsigned int j = 0; j < floatsPerVector; j++)
        {
            vectors.back()[j] = floatVec[i + j];
        }
    }
    return vectors;
}