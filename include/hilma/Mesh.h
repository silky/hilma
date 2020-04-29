#pragma once

#include <vector>
#include <string>

#include "glm/glm.hpp"

#if defined(PLATFORM_RPI) || defined(PLATFORM_RPI4) 
#define INDEX_TYPE uint16_t
#else
#define INDEX_TYPE uint32_t
#endif

enum Primitive {
    POINTS,
    LINES,
    TRIANGLES,
    LINE_STRIP
};

class Mesh {
public:

    Mesh();
    Mesh(const Mesh& _mother);
    virtual ~Mesh();

    void        add(const Mesh& _mesh);

    void        setPrimitive(Primitive _drawMode = TRIANGLES);
    Primitive   getPrimitive() const;

    void        setColor(const glm::vec4& _color);
    void        setColor(float _r, float _g, float _b, float _a = 1.0f);
    void        addColor(const glm::vec4& _color);
    void        addColor(float _r, float _g, float _b, float _a = 1.0f);
    void        addColors(const std::vector<glm::vec4>& _colors);

    void        addVertex(const glm::vec3& _point);
    void        addVertex(float _x, float _y, float _z = 0.0f);
    void        addVertices(const std::vector<glm::vec3>& _verts);
    void        addVertices(const glm::vec3* _verts, int _amt);

    void        addNormal(const glm::vec3& _normal);
    void        addNormal(float _nX, float _nY, float _nZ);
    void        addNormals(const std::vector<glm::vec3>& _normals );

    void        addTangent(float _x, float _y, float _z, float _w);
    void        addTangent(const glm::vec4& _tangent);

    void        addTexCoord(const glm::vec2& _uv);
    void        addTexCoord(float _tX, float _tY);
    void        addTexcoords(const std::vector<glm::vec2>& _uvs);

    void        addIndex(INDEX_TYPE _i);
    void        addIndices(const std::vector<INDEX_TYPE>& _inds);
    void        addIndices(const INDEX_TYPE* _inds, int _amt);

    void        addTriangle(INDEX_TYPE index1, INDEX_TYPE index2, INDEX_TYPE index3);
    std::vector<glm::ivec3>  getTriangles() const ;

    const bool  hasColors() const { return colors.size() > 0; }
    const bool  hasNormals() const { return normals.size() > 0; }
    const bool  hasTexcoords() const { return texcoords.size() > 0; }
    const bool  hasTangents() const { return tangents.size() > 0; }
    const bool  hasIndices() const { return indices.size() > 0; }
    const bool  hasEdgeIndices() const { return edge_indices.size() > 0; }
    const bool  hasEdgeColors() const { return edge_colors.size() > 0; }

    bool        computeNormals();
    bool        computeTangents();
    void        clear();

    std::vector<glm::vec4>  edge_colors;
    std::vector<glm::vec4>  colors;
    std::vector<glm::vec4>  tangents;
    std::vector<glm::vec3>  vertices;
    std::vector<glm::vec3>  normals;
    std::vector<glm::vec2>  texcoords;
    std::vector<INDEX_TYPE> indices;
    std::vector<INDEX_TYPE> indices_normals;
    std::vector<INDEX_TYPE> indices_texcoords;
    std::vector<INDEX_TYPE> edge_indices;

private:
    Primitive               primitive;
};
