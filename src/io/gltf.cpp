#include "hilma/io/gltf.h"

#include "hilma/io/auto.h"
#include "hilma/io/jpg.h"
#include "hilma/io/png.h"
#include "hilma/io/hdr.h"

#include "hilma/math.h"
#include "hilma/fs.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION
// #define JSON_NOEXCEPTION
#include "../deps//tiny_gltf.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#include "hilma/fs.h"
#include "hilma/text.h"
#include "hilma/ops/convert_image.h"
#include "hilma/ops/compute.h"

namespace hilma {


// IMAGE LOAD
//
bool load(const std::string& _filename, Image& _image, int _channels) {
    int width, height, channels;
    std::string ext = getExt(_filename);

    if (ext == "hdr" || ext == "HDR") {
        float *pixels = stbi_loadf(_filename.c_str(), &width, &height, &channels, _channels);

        if (!pixels)
            return false;

        _image.allocate(width, height, channels);
        int total = width * height * channels;
        if (total != _image.data.size())
            _image.data.resize(total);
        std::memcpy(&_image.data[0], pixels, total * sizeof(float));
        _image.name = getFilename( _filename );

        return true;
    }
    else {
        uint16_t* pixels = stbi_load_16(_filename.c_str(), &width, &height, &channels, _channels);
        
        if (!pixels)
            return false;

        _image.allocate(width, height, channels);
        int total = width * height * channels;
        const float m = 1.f / 65535.f;
        for (int i = 0; i < total; i++)
            _image.setValue(i, float(pixels[i]) * m);

        delete pixels;

    }

    _image.name = getFilename( _filename );

    return true;
}

unsigned char*  load(const std::string& _filename, int* _width, int* _height, int* _channels) {
    return stbi_load(_filename.c_str(), _width, _height, _channels, 0);
}

float* loadHdr(const std::string& _filename, int* _width, int* _height, int* _channels) {
    return stbi_loadf(_filename.c_str(), _width, _height, _channels, 0);
}


// IMAGE SAVE
//

bool saveHdr(const std::string& _filename, const float* _pixels, int _width, int _height, int _channels) {
    return stbi_write_hdr(_filename.c_str(), _width, _height, _channels, _pixels);
}

bool saveHdr(const std::string& _filename, Image& _image) {
    return stbi_write_hdr(_filename.c_str(), _image.getWidth(), _image.getHeight(), _image.getChannels(), &_image[0]);
}

bool savePng(const std::string& _filename, const unsigned char* _pixels, int _width, int _height, int _channels) {
    return stbi_write_png(_filename.c_str(), _width, _height, _channels, _pixels, 0);
}

bool savePng(const std::string& _filename, Image& _image) {
    int total = _image.getWidth() * _image.getHeight() * _image.getChannels();
    unsigned char* pixels = new unsigned char[total];
    for (int i = 0; i < total; i++)
        pixels[i] = static_cast<char>(256 * clamp(_image[i], 0.0f, 0.999f));
    savePng(_filename, pixels, _image.getWidth(), _image.getHeight(), _image.getChannels());
    delete [] pixels;

    return true;
}

bool saveJpg(const std::string& _filename, const unsigned char* _pixels, int _width, int _height, int _channels) {
    return stbi_write_jpg(_filename.c_str(), _width, _height, _channels, _pixels, 90);
}

bool saveJpg(const std::string& _filename, Image& _image) {
    int total = _image.getWidth() * _image.getHeight() * _image.getChannels();
    unsigned char* pixels = new unsigned char[total];
    for (int i = 0; i < total; i++)
        pixels[i] = static_cast<char>(256 * clamp(_image[i], 0.0, 0.999));
    saveJpg(_filename, pixels, _image.getWidth(), _image.getHeight(), _image.getChannels());
    delete [] pixels;

    return true;
}

bool save(const std::string& _filename, Image& _image) {
    std::string ext = getExt(_filename);

    if (ext == "hdr" || ext == "HDR") {
        return saveHdr(_filename, _image);
    }
    else if (ext == "png" || ext == "PNG") {
        return savePng(_filename, _image);
    }
    else if (ext == "jpg" || ext == "JPG" || 
             ext == "jpg" || ext == "JPG" ) {
        return saveJpg(_filename, _image);
    }

    return false;
}


// bool savePng16(const std::string& _filename, uint16_t* _pixels, int _width, int _height, int _channels) {

//     /* create file */
//     FILE *fp = fopen(_filename.c_str(), "wb");
//     if (!fp){
//         std::runtime_error("[write_png_file] File could not be opened for writing");
//         return false;
//     }


//     /* initialize stuff */
//     png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

//     if (!png_ptr){
//         std::runtime_error("[write_png_file] png_create_write_struct failed");
//         return false;
//     }

//     png_infop info_ptr = png_create_info_struct(png_ptr);
//     if (!info_ptr){
//         std::runtime_error("[write_png_file] png_create_info_struct failed");
//         return false;
//     }

//     if (setjmp(png_jmpbuf(png_ptr))){
//         std::runtime_error("[write_png_file] Error during init_io");
//         return false;
//     }

//     png_init_io(png_ptr, fp);


//     /* write header */
//     if (setjmp(png_jmpbuf(png_ptr))){
//         std::runtime_error("[write_png_file] Error during writing header");
//         return false;
//     }

//     png_set_IHDR(png_ptr, info_ptr,
//             (png_uint_32) _width, (png_uint_32) _height,
//             (png_byte) 16, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
//             PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

//     png_write_info(png_ptr, info_ptr);


//     /* write bytes */
//     if (setjmp(png_jmpbuf(png_ptr))){
//         std::runtime_error("[write_png_file] Error during writing bytes");
//         return false;
//     }

//     /* Initialize rows of PNG. */
//     png_bytepp row_pointers = (png_bytepp) malloc (sizeof(png_bytep)*_height);
//     png_bytep row = (png_bytep) malloc(6 * _width * sizeof(png_byte));
//     for (size_t y = 0; y < _height; ++y) {
//         row_pointers[y] = (png_bytep) malloc (png_get_rowbytes(png_ptr,info_ptr));
//         for (size_t x = 0; x < _width; ++x) {
//             size_t i = y * _width + x;
//             png_save_uint_16(&row[x*6+0], (unsigned int)_pixels[i * _channels + 0] );
//             png_save_uint_16(&row[x*6+2], (unsigned int)_pixels[i * _channels + 1] );
//             png_save_uint_16(&row[x*6+4], (unsigned int)_pixels[i * _channels + 2] );
//         }
//         png_write_row(png_ptr, row);
//     }

//     /* end write */
//     if (setjmp(png_jmpbuf(png_ptr))){
//         std::runtime_error("[write_png_file] Error during end of write");
//         return false;
//     }

//     png_write_end(png_ptr, NULL);
//     fclose(fp);
//     free(row);

//     return true;
// }


// GLTF LOAD 
//
bool loadModel(tinygltf::Model& _model, const std::string& _filename) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    std::string ext = getExt(_filename);

    bool res = false;

    // assume binary glTF.
    if (ext == "glb" || ext == "GLB")
        res = loader.LoadBinaryFromFile(&_model, &err, &warn, _filename.c_str());

    // assume ascii glTF.
    else
        res = loader.LoadASCIIFromFile(&_model, &err, &warn, _filename.c_str());

    if (!warn.empty())
        std::cout << "Warn: " << warn.c_str() << std::endl;

    if (!err.empty())
        std::cout << "ERR: " << err.c_str() << std::endl;

    return res;
}

int toPrimitiveMode(const FaceType& _type) {
    if (_type == TRIANGLES)
        return TINYGLTF_MODE_TRIANGLES;
    else if (_type == TRIANGLE_STRIP)
        return TINYGLTF_MODE_TRIANGLE_STRIP;
    else if (_type == TRIANGLE_FAN)
        return TINYGLTF_MODE_TRIANGLE_FAN;
    else
      return TINYGLTF_MODE_POINTS;
}

FaceType toFaceType(const tinygltf::Primitive& _primitive) {
    if (_primitive.mode == TINYGLTF_MODE_TRIANGLES)
        return TRIANGLES;
    else if (_primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP)
        return TRIANGLE_STRIP;
    else if (_primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN)
        return TRIANGLE_FAN;
    else
        return POINTS;
}

EdgeType toEdgeType(const tinygltf::Primitive& _primitive) {
    if (_primitive.mode == TINYGLTF_MODE_LINE)
      return LINES;
    // else if (_primitive.mode == TINYGLTF_MODE_LINE_LOOP)
    //   return LINE_STRIP ;
    else 
      return UNDEFINED_EDGE;
}

void extractFaceIndices(const tinygltf::Model& _model, const tinygltf::Accessor& _indexAccessor, Mesh& _mesh) {
    const tinygltf::BufferView &buffer_view = _model.bufferViews[_indexAccessor.bufferView];
    const tinygltf::Buffer &buffer = _model.buffers[buffer_view.buffer];
    const uint8_t* base = &buffer.data.at(buffer_view.byteOffset + _indexAccessor.byteOffset);

    switch (_indexAccessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
            const uint32_t *p = (uint32_t*) base;
            for (size_t i = 0; i < _indexAccessor.count; ++i)
                _mesh.addFaceIndex( p[i] );
        }; break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
            const uint16_t *p = (uint16_t*) base;
            for (size_t i = 0; i < _indexAccessor.count; ++i)
                _mesh.addFaceIndex( p[i] );
        }; break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
            const uint8_t *p = (uint8_t*) base;
            for (size_t i = 0; i < _indexAccessor.count; ++i)
                _mesh.addFaceIndex( p[i] );
        }; break;
    }
}

void extractVertexData(uint32_t v_pos, const uint8_t *base, int accesor_componentType, int accesor_type, bool accesor_normalized, uint32_t byteStride, float *output, uint8_t max_num_comp) {
    float v[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    uint32_t ncomp = 1;
    switch (accesor_type) {
        case TINYGLTF_TYPE_SCALAR: ncomp = 1; break;
        case TINYGLTF_TYPE_VEC2:   ncomp = 2; break;
        case TINYGLTF_TYPE_VEC3:   ncomp = 3; break;
        case TINYGLTF_TYPE_VEC4:   ncomp = 4; break;
        default:
            assert(!"invalid type");
    }
    switch (accesor_componentType) {
        case TINYGLTF_COMPONENT_TYPE_FLOAT: {
            const float *data = (float*)(base+byteStride*v_pos);
            for (uint32_t i = 0; (i < ncomp); ++i)
                v[i] = data[i];
        }
        // TODO SUPPORT OTHER FORMATS
        break;
        default:
            assert(!"Conversion Type from float to -> ??? not implemented yet");
            break;
    }
    for (uint32_t i = 0; i < max_num_comp; ++i)
        output[i] = v[i];
}


Material extractMaterial(const tinygltf::Model& _model, const tinygltf::Material& _material, bool _verbose) {
    Material mat = Material( toLower( toUnderscore( purifyString( _material.name ) ) ) );
    
    if (_material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
        const tinygltf::Texture &tex = _model.textures[_material.pbrMetallicRoughness.baseColorTexture.index];
        const tinygltf::Image &image = _model.images[tex.source];
        std::string name = image.name + "_" + image.uri;
        if (name == "_")
            name = _material.name;
        name += "_diffuse";

        // Todo:
        //      - image.bits
        //
        Image img = Image(&image.image[0], image.width, image.height, image.component);
        img.name = name;
        
        mat.set("diffuse", img);

        if (_verbose)
            std::cout << "Loading " << name << " as diffuse image" << std::endl;
    }
    else {
        std::vector<double> c = _material.pbrMetallicRoughness.baseColorFactor;
        mat.set("diffuse", glm::vec4(float(c[0]), float(c[1]), float(c[2]), float(c[3])));
    }

    if (_material.emissiveTexture.index >= 0) {
        const tinygltf::Image &image = _model.images[_model.textures[_material.emissiveTexture.index].source];
        std::string name = image.name + "_" + image.uri;
        if (name == "_")
            name = _material.name;
        name += "_emissive";

        Image img = Image(&image.image[0], image.width, image.height, image.component);
        img.name = name;

        mat.set("emissive", img);

        if (_verbose)
            std::cout << "Loading " << name << " as emissive image" << std::endl;
    }
    else {
        std::vector<double> c = _material.emissiveFactor;
        mat.set("emissive", glm::vec3(float(c[0]), float(c[1]), float(c[2])));
    }

    bool isOcclusionRoughnessMetallic = false;
    if (_material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
        tinygltf::Texture tex = _model.textures[_material.pbrMetallicRoughness.metallicRoughnessTexture.index];
        const tinygltf::Image &image = _model.images[tex.source];

        Image img = Image(&image.image[0], image.width, image.height, image.component);
        std::string name = image.name + "_" + image.uri;
        if (name == "_")
            name = _material.name;

        std::vector<Image> imgs = splitChannels(img);
        
        if (_material.occlusionTexture.index >= 0) {
            const tinygltf::Image &occlussionImage = _model.images[_model.textures[_material.occlusionTexture.index].source];
            if (image.uri != "" && image.uri == occlussionImage.uri) {
                imgs[0].name = name + "_occlusion";
                
                mat.set("occlusion", imgs[0]);
                isOcclusionRoughnessMetallic = true;
                if (_verbose)
                    std::cout << "Loading " << name << " as occlusion image" << std::endl;
            }
        }

        imgs[1].name = name + "_roughness";
        mat.set("roughness", imgs[1]);

        imgs[2].name = name + "_metallic";
        mat.set("metallic", imgs[2]);

        if (_verbose)
            std::cout << "Loading " << name << " as roughness/metallic image" << std::endl;
    }
    else {
        mat.set("roughness", (float)_material.pbrMetallicRoughness.roughnessFactor);
        mat.set("metallic", (float)_material.pbrMetallicRoughness.metallicFactor);
    }

     // OCCLUSION
    if (!isOcclusionRoughnessMetallic && _material.occlusionTexture.index >= 0) {
        const tinygltf::Image &image = _model.images[_model.textures[_material.occlusionTexture.index].source];
        std::string name = image.name + "_" + image.uri;
        if (name == "_")
            name = _material.name;
        name += "_occlusion";
        
        Image img = Image(&image.image[0], image.width, image.height, image.component);
        img.name = name;
        
        mat.set("occlusion", img);
        if (_verbose)
            std::cout << "Loading " << name << " as occlusion image" << std::endl;
    }

    // NORMALMAP
    if (_material.normalTexture.index >= 0) {
        const tinygltf::Image &image = _model.images[_model.textures[_material.normalTexture.index].source];
        std::string name = image.name + "_" + image.uri;
        if (name == "_")
            name = _material.name;
        name += "_normalmap";

        Image img = Image(&image.image[0], image.width, image.height, image.component);
        img.name = name;
        
        mat.set("normalmap", img);
        if (_verbose)
            std::cout << "Loading " << name << " as normalmap" << std::endl;
    }

    return mat;
}


void extractMesh(const tinygltf::Model& _model, const tinygltf::Mesh& _mesh, glm::mat4 _matrix, Mesh& _out_mesh, bool _verbose) {
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(_matrix)));

    for (size_t i = 0; i < _mesh.primitives.size(); ++i) {
        if (_verbose)
            std::cout << "   primitive " << i + 1 << "/" << _mesh.primitives.size() << std::endl;

        const tinygltf::Primitive &primitive = _mesh.primitives[i];

        Mesh mesh;
        if (primitive.indices >= 0)
            extractFaceIndices(_model, _model.accessors[primitive.indices], mesh);
        mesh.setFaceType( toFaceType(primitive) );

        // Extract Vertex Data
        for (auto &attrib : primitive.attributes) {
            const tinygltf::Accessor &accessor = _model.accessors[attrib.second];
            const tinygltf::BufferView &bufferView = _model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer &buffer = _model.buffers[bufferView.buffer];
            int byteStride = accessor.ByteStride(bufferView);

            if (attrib.first.compare("POSITION") == 0)  {
                for (size_t v = 0; v < accessor.count; v++) {
                    glm::vec4 pos = glm::vec4(1.0);
                    extractVertexData(v, &buffer.data.at(bufferView.byteOffset + accessor.byteOffset), accessor.componentType, accessor.type, accessor.normalized, byteStride, &pos[0], 3);
                    mesh.addVertex( glm::vec3(_matrix * pos) );
                }
            }

            else if (attrib.first.compare("COLOR_0") == 0)  {
                for (size_t v = 0; v < accessor.count; v++) {
                    glm::vec4 col = glm::vec4(1.0f);
                    extractVertexData(v, &buffer.data.at(bufferView.byteOffset + accessor.byteOffset), accessor.componentType, accessor.type, accessor.normalized, byteStride, &col[0], 4);
                    mesh.addColor(col);
                }
            }

            else if (attrib.first.compare("NORMAL") == 0)  {
                for (size_t v = 0; v < accessor.count; v++) {
                    glm::vec3 nor;
                    extractVertexData(v, &buffer.data.at(bufferView.byteOffset + accessor.byteOffset), accessor.componentType, accessor.type, accessor.normalized, byteStride, &nor[0], 3);
                    mesh.addNormal( normalize(normalMatrix * nor) );
                }
            }

            else if (attrib.first.compare("TEXCOORD_0") == 0)  {
                for (size_t v = 0; v < accessor.count; v++) {
                    glm::vec2 uv;
                    extractVertexData(v, &buffer.data.at(bufferView.byteOffset + accessor.byteOffset), accessor.componentType, accessor.type, accessor.normalized, byteStride, &uv[0], 2);
                    mesh.addTexCoord(uv);
                }
            }

            else if (attrib.first.compare("TANGENT") == 0)  {
                for (size_t v = 0; v < accessor.count; v++) {
                    glm::vec4 tan;
                    extractVertexData(v, &buffer.data.at(bufferView.byteOffset + accessor.byteOffset), accessor.componentType, accessor.type, accessor.normalized, byteStride, &tan[0], 4);
                    mesh.addTangent(tan);
                }
            }

            else {
                std::cout << " " << std::endl;
                std::cout << "Attribute: " << attrib.first << std::endl;
                std::cout << "  type        :" << accessor.type << std::endl;
                std::cout << "  component   :" << accessor.componentType << std::endl;
                std::cout << "  normalize   :" << accessor.normalized << std::endl;
                std::cout << "  bufferView  :" << accessor.bufferView << std::endl;
                std::cout << "  byteOffset  :" << accessor.byteOffset << std::endl;
                std::cout << "  count       :" << accessor.count << std::endl;
                std::cout << "  byteStride  :" << byteStride << std::endl;
                std::cout << " "<< std::endl;
            }
        }

        if (_verbose) {
            std::cout << "    vertices = " << mesh.getVertices().size() << std::endl;
            std::cout << "    colors   = " << mesh.getColors().size() << std::endl;
            std::cout << "    normals  = " << mesh.getNormals().size() << std::endl;
            std::cout << "    uvs      = " << mesh.getTexCoords().size() << std::endl;
            std::cout << "  f indices  = " << mesh.getFaceIndicesTotal() << std::endl;
            if (mesh.getFaceType() == TRIANGLES)
                std::cout << "    triang.  = " << mesh.getFaceIndicesTotal()/3 << std::endl;
            std::cout << "  e indices  = " << mesh.getEdgeIndicesTotal() << std::endl;
            if (mesh.getEdgeType() == LINES )
                std::cout << "    lines    = " << mesh.getEdgeIndicesTotal()/2 << std::endl;
        }

        if ( !mesh.haveNormals() )
            mesh.computeNormals();

        if ( !mesh.haveTangents() )
            mesh.computeTangents();

        Material mat = extractMaterial( _model, _model.materials[primitive.material], _verbose );
        _out_mesh.addMaterial(mat);
        _out_mesh.append(mesh);
    }
}

// bind models
void extractNodes(const tinygltf::Model& _model, const tinygltf::Node& _node, glm::mat4 _matrix, Mesh& _mesh, bool _verbose) {
    if (_verbose)
        std::cout << "Entering node " << _node.name << std::endl;

    glm::mat4 R = glm::mat4(1.0f);
    glm::mat4 S = glm::mat4(1.0f);
    glm::mat4 T = glm::mat4(1.0f);

    if (_node.rotation.size() == 4) {
        glm::quat q = glm::make_quat(_node.rotation.data());
        R = glm::mat4( q );
    }

    if (_node.scale.size() == 3)
        S = glm::scale( S, glm::vec3(   float(_node.scale[0]), 
                                        float(_node.scale[1]), 
                                        float(_node.scale[2]) ) );

    if (_node.translation.size() == 3)
        T = glm::translate( T, glm::vec3(   float(_node.translation[0]), 
                                            float(_node.translation[1]), 
                                            float(_node.translation[2]) ) );

    glm::mat4 localMatrix = T * R * S; 

    if (_node.matrix.size() == 16)
        localMatrix = glm::make_mat4x4(_node.matrix.data());

    _matrix = _matrix * localMatrix;

    if (_node.mesh >= 0)
        extractMesh(_model, _model.meshes[ _node.mesh ], _matrix, _mesh, _verbose);

    if (_node.camera >= 0)
        if (_verbose)
            std::cout << "  w camera" << std::endl;
        // TODO extract camera
    
    for (size_t i = 0; i < _node.children.size(); i++) {
        extractNodes(_model, _model.nodes[ _node.children[i] ], _matrix, _mesh, _verbose);
    }
};

bool    loadGltf( const std::string& _filename, Mesh& _mesh ) {
    tinygltf::Model model;

    if (! loadModel(model, _filename)) {
        std::cout << "Failed to load .glTF : " << _filename << std::endl;
        return false;
    }

    const tinygltf::Scene &scene = model.scenes[model.defaultScene];
    for (size_t i = 0; i < scene.nodes.size(); ++i)
        extractNodes(model, model.nodes[scene.nodes[i]], glm::mat4(1.0), _mesh, false);

    return true;
}

int makeBuffer( const std::string& _name, const float* _data, const int _total, const int _floatsPerData,
                tinygltf::Model& _outModel, bool _embebedFiles ) {

    tinygltf::Buffer        buffer = tinygltf::Buffer();
    tinygltf::BufferView    view = tinygltf::BufferView();
    tinygltf::Accessor      accessor = tinygltf::Accessor();

    // give the buffer a unique name and address
    buffer.name = _name;
    if (!_embebedFiles)
        buffer.uri = _name + ".bin";

    // Convert vector of vec3 to binnaty 
    view.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    view.byteOffset = 0;
    view.byteStride = 0;
    // view.byteStride = sizeof(float) * _floatsPerData;
    view.byteLength = sizeof(float) * _floatsPerData * _total;
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(_data);
    buffer.data = std::vector<unsigned char>(   bytes + view.byteOffset, 
                                                bytes + view.byteOffset + view.byteLength);

    // Assigne the position on the vector of buffers where it will store
    view.buffer = _outModel.buffers.size();
    _outModel.buffers.push_back(buffer);

    // Assign the position of the vector of bufferViews wher it will be store
    accessor.bufferView = _outModel.bufferViews.size();
    _outModel.bufferViews.push_back(view);

    accessor.byteOffset = 0;
    if (_floatsPerData == 1)
        accessor.type = TINYGLTF_TYPE_SCALAR;
    else if (_floatsPerData == 2)
        accessor.type = TINYGLTF_TYPE_VEC2;
    else if (_floatsPerData == 3)
        accessor.type = TINYGLTF_TYPE_VEC3;
    else if (_floatsPerData == 4)
        accessor.type = TINYGLTF_TYPE_VEC4;
    else
        accessor.type = TINYGLTF_TYPE_VECTOR;

    accessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accessor.count = _total;
    accessor.normalized = false;

    std::cout << _name << " min: ";
    std::vector<float> range_min = getMin(_data, _total, _floatsPerData);
    for (size_t i = 0; i < range_min.size(); i++) {
        accessor.minValues.push_back(range_min[i]);
        std::cout << range_min[i] << " ";
    }
    std::cout << std::endl;

    std::cout << _name << " max: ";
    std::vector<float> range_max = getMax(_data, _total, _floatsPerData);
    for (size_t i = 0; i < range_max.size(); i++) {
        accessor.maxValues.push_back(range_max[i]);
        std::cout << range_max[i] << " ";
    }
    std::cout << std::endl;

    int access_index = _outModel.accessors.size();
    
    _outModel.accessors.push_back(accessor);

    return access_index;
}

bool convertMesh( const Mesh& _inMesh, tinygltf::Model& _outModel, bool _embebedFiles) {
    tinygltf::Mesh mesh = tinygltf::Mesh();
    mesh.name = _inMesh.getName();

    int iVertices = -1;
    int iFaces = -1;
    int iColors = -1;
    int iNormals = -1;
    int iTexCoords = -1;
    int iTangents = -1;

    // Vertices
    {
        iVertices = makeBuffer( _inMesh.getName() + "_vertices", &_inMesh.getVertices()[0].x, _inMesh.getVerticesTotal(), 3, _outModel, _embebedFiles);
    }
    
    // Colors
    // if (_inMesh.haveColors())
    //     iColors = makeBuffer( _inMesh.getName() + "_colors", &_inMesh.getColors()[0].x, _inMesh.getColorsTotal(), 4, _outModel, _embebedFiles);

    // // Tangent
    // if (_inMesh.haveTangents())
    //     iTangents = makeBuffer( _inMesh.getName() + "_tangents", &_inMesh.getTangents()[0].x, _inMesh.getTangetsTotal(), 4, _outModel, _embebedFiles);


    // // Normals
    // if (_inMesh.haveNormals())
    //     iNormals = makeBuffer( _inMesh.getName() + "_normals", &_inMesh.getNormals()[0].x, _inMesh.getNormalsTotal(), 3, _outModel, _embebedFiles);

    
    // // TexCoords
    // if (_inMesh.haveColors())
    //     iColors = makeBuffer( _inMesh.getName() + "_texcoords", &_inMesh.getTexCoords()[0].x, _inMesh.getTexCoordsTotal(), 2, nullptr, _outModel, _embebedFiles);

    // Indices
    if ( _inMesh.haveFaceIndices() ) {
        tinygltf::Buffer        facesBuffer = tinygltf::Buffer();
        tinygltf::BufferView    facesView = tinygltf::BufferView();
        tinygltf::Accessor      facesAccessor = tinygltf::Accessor();

        // give the buffer a unique name and address
        facesBuffer.name = _inMesh.getName() + "_facesBuffer";
        if (!_embebedFiles)
            facesBuffer.uri = _inMesh.getName() + "_faces.bin";

        // Convert to a binnary buffer
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&_inMesh.getFaceIndices()[0]);
        facesView.name = _inMesh.getName() + "_facesView";
        facesView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
        facesView.byteOffset = 0;
        // facesView.byteStride = sizeof(INDEX_TYPE);
        facesView.byteLength = sizeof(INDEX_TYPE) * _inMesh.getFaceIndicesTotal();
        facesBuffer.data = std::vector<unsigned char>(  bytes + facesView.byteOffset, 
                                                        bytes + facesView.byteOffset + facesView.byteLength );
        // Reference the buffer on the viewer
        facesView.buffer = _outModel.buffers.size();
        _outModel.buffers.push_back(facesBuffer);

        // Reference the view on the accessor
        facesAccessor.name = _inMesh.getName() + "_facesAccessor";
        facesAccessor.bufferView = _outModel.bufferViews.size();
        _outModel.bufferViews.push_back(facesView);
        facesAccessor.byteOffset = 0;
        facesAccessor.type = TINYGLTF_TYPE_SCALAR;
        facesAccessor.count = _inMesh.getFaceIndicesTotal();
        facesAccessor.normalized = false;
#if defined(PLATFORM_RPI)
#define INDEX_TYPE uint16_t
        facesAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
#else
#define INDEX_TYPE uint32_t
        facesAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
#endif

        // Reference the data on the primitive
        iFaces = _outModel.accessors.size();
        _outModel.accessors.push_back(facesAccessor);
    }
    tinygltf::Primitive faces = tinygltf::Primitive();
    faces.mode = toPrimitiveMode( _inMesh.getFaceType() );

    faces.attributes["POSITION"] = iVertices;
    if (iNormals >= 0) faces.attributes["NORMAL"] = iNormals;
    if (iColors >= 0) faces.attributes["COLOR_0"] = iColors;
    if (iTexCoords >= 0) faces.attributes["TEXCOORD_0"] = iTexCoords;
    if (iTangents >= 0) faces.attributes["TANGENT"] = iTangents;
    
    tinygltf::Material mat = tinygltf::Material();
    mat.name = "default_material";
    mat.emissiveFactor = {0.0, 0.0, 0.0};       // length 3. default [0, 0, 0]
    mat.alphaMode   = "OPAQUE";                 // default "OPAQUE"
    mat.alphaCutoff = 0.5;                      // default 0.5
    mat.doubleSided = false;                    // default false;
    mat.pbrMetallicRoughness = tinygltf::PbrMetallicRoughness();
    int iMat = _outModel.materials.size();
    _outModel.materials.push_back(mat);

    if (iFaces >= 0) faces.indices = iFaces;
    faces.material = iMat;
    mesh.primitives.push_back(faces);

    tinygltf::Node node = tinygltf::Node();
    node.name = "meshNode";
    node.mesh = _outModel.meshes.size(); 
    _outModel.meshes.push_back(mesh);

    int aNode = _outModel.nodes.size();
    _outModel.nodes.push_back(node);

    tinygltf::Scene scene = tinygltf::Scene();
    scene.name = "root";
    scene.nodes.push_back(aNode);
    _outModel.scenes.push_back(scene);

    return true;
}

bool saveGltf( const std::string& _filename, const Mesh& _mesh ) {
    tinygltf::Model model = tinygltf::Model();
    model.asset = tinygltf::Asset();
    model.asset.version = "2.0";
    model.asset.generator = "Genereted using Hilma C++/Python Library";
    model.asset.copyright = "2020 (c) Patricio Gonzalez Vivo";

    std::string ext = getExt(_filename);
    bool bin = (ext == "glb" || ext == "GLB");

    convertMesh(_mesh, model, bin);

    tinygltf::TinyGLTF gltf = tinygltf::TinyGLTF();
    return gltf.WriteGltfSceneToFile(   &model, 
                                        _filename,
                                        bin,       // embedBuffer
                                        bin,       // embedImagess
                                        !bin,       // prettyPrint
                                        bin );    // writeBinary
}

}