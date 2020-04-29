#include "hilma/Mesh.h"

#include <iostream>

Mesh::Mesh():primitive(TRIANGLES) {

}

Mesh::Mesh(const Mesh &_mother): primitive(_mother.getPrimitive()) {
    add(_mother);
}

Mesh::~Mesh() {

}

void Mesh::setPrimitive(Primitive _primitive) {
    primitive = _primitive;
}

void Mesh::setColor(const glm::vec4 &_color) {
    colors.clear();
    for (uint32_t i = 0; i < vertices.size(); i++) {
        colors.push_back(_color);
    }
}

void Mesh::addColor(const glm::vec4 &_color) {
    colors.push_back(_color);
}

void Mesh::addColors(const std::vector<glm::vec4> &_colors) {
    colors.insert(colors.end(), _colors.begin(), _colors.end());
}

void Mesh::addVertex(const glm::vec3 &_point) {
   vertices.push_back(_point);
}

void Mesh::addVertices(const std::vector<glm::vec3>& _verts) {
   vertices.insert(vertices.end(),_verts.begin(),_verts.end());
}

void Mesh::addVertices(const glm::vec3* verts, int amt) {
   vertices.insert(vertices.end(),verts,verts+amt);
}

void Mesh::addNormal(const glm::vec3 &_normal) {
    normals.push_back(_normal);
}

void Mesh::addNormals(const std::vector<glm::vec3> &_normals ) {
    normals.insert(normals.end(), _normals.begin(), _normals.end());
}

void  Mesh::addTangent(const glm::vec4 &_tangent) {
    tangents.push_back(_tangent);
}

void Mesh::addTexCoord(const glm::vec2 &_uv) {
    texCoords.push_back(_uv);
}

void Mesh::addTexCoords(const std::vector<glm::vec2> &_uvs) {
    texCoords.insert(texCoords.end(), _uvs.begin(), _uvs.end());
}

void Mesh::addIndex(INDEX_TYPE _i) {
    indices.push_back(_i);
}

void Mesh::addIndices(const std::vector<INDEX_TYPE>& inds) {
    indices.insert(indices.end(),inds.begin(),inds.end());
}

void Mesh::addIndices(const INDEX_TYPE* inds, int amt) {
    indices.insert(indices.end(),inds,inds+amt);
}

void Mesh::addTriangle(INDEX_TYPE index1, INDEX_TYPE index2, INDEX_TYPE index3) {
    addIndex(index1);
    addIndex(index2);
    addIndex(index3);
}

void Mesh::add(const Mesh &_mesh) {

    if (_mesh.getPrimitive() != primitive) {
        std::cout << "INCOMPATIBLE DRAW MODES" << std::endl;
        return;
    }

    INDEX_TYPE indexOffset = (INDEX_TYPE)getVertices().size();

    addColors(_mesh.getColors());
    addVertices(_mesh.getVertices());
    addNormals(_mesh.getNormals());
    addTexCoords(_mesh.getTexCoords());

    for (uint32_t i = 0; i < _mesh.getIndices().size(); i++) {
        addIndex(indexOffset+_mesh.getIndices()[i]);
    }
}

Primitive Mesh::getPrimitive() const{
    return primitive;
}

const std::vector<glm::vec4> & Mesh::getColors() const{
    return colors;
}

const std::vector<glm::vec4> & Mesh::getTangents() const {
    return tangents;
}

const std::vector<glm::vec3> & Mesh::getVertices() const{
    return vertices;
}

const std::vector<glm::vec3> & Mesh::getNormals() const{
    return normals;
}

const std::vector<glm::vec2> & Mesh::getTexCoords() const{
    return texCoords;
}

const std::vector<INDEX_TYPE> & Mesh::getIndices() const{
    return indices;
}

std::vector<glm::ivec3> Mesh::getTriangles() const {
    std::vector<glm::ivec3> faces;

    if (getPrimitive() == TRIANGLES) {
        if (hasIndices()) {
            for(unsigned int j = 0; j < indices.size(); j += 3) {
                glm::ivec3 tri;
                for(int k = 0; k < 3; k++) {
                    tri[k] = indices[j+k];
                }
                faces.push_back(tri);
            }
        }
        else {
            for( unsigned int j = 0; j < vertices.size(); j += 3) {
                glm::ivec3 tri;
                for(int k = 0; k < 3; k++) {
                    tri[k] = j+k;
                }
                faces.push_back(tri);
            }
        }
    }
    else {
        //  TODO
        //
        std::cout << "ERROR: getTriangles(): Mesh only add TRIANGLES for NOW !!" << std::endl;
    }

    return faces;
}

void Mesh::clear() {
    if (!vertices.empty()) {
        vertices.clear();
    }
    if (hasColors()) {
        colors.clear();
    }
    if (hasNormals()) {
        normals.clear();
    }
    if (hasTexCoords()) {
        texCoords.clear();
    }
    if (hasTangents()) {
        tangents.clear();
    }
    if (hasIndices()) {
        indices.clear();
    }
}

bool Mesh::computeNormals() {
    if (getPrimitive() != TRIANGLES) 
        return false;

    //The number of the vertices
    int nV = vertices.size();

    //The number of the triangles
    int nT = indices.size() / 3;

    std::vector<glm::vec3> norm( nV ); //Array for the normals

    //Scan all the triangles. For each triangle add its
    //normal to norm's vectors of triangle's vertices
    for (int t=0; t<nT; t++) {

        //Get indices of the triangle t
        int i1 = indices[ 3 * t ];
        int i2 = indices[ 3 * t + 1 ];
        int i3 = indices[ 3 * t + 2 ];

        //Get vertices of the triangle
        const glm::vec3 &v1 = vertices[ i1 ];
        const glm::vec3 &v2 = vertices[ i2 ];
        const glm::vec3 &v3 = vertices[ i3 ];

        //Compute the triangle's normal
        glm::vec3 dir = glm::normalize(glm::cross(v2-v1,v3-v1));

        //Accumulate it to norm array for i1, i2, i3
        norm[ i1 ] += dir;
        norm[ i2 ] += dir;
        norm[ i3 ] += dir;
    }

    //Normalize the normal's length and add it.
    normals.clear();
    for (int i=0; i<nV; i++) {
        addNormal( glm::normalize(norm[i]) );
    }

    return true;
}

// http://www.terathon.com/code/tangent.html
bool Mesh::computeTangents() {
    //The number of the vertices
    size_t nV = vertices.size();

    if (texCoords.size() != nV || 
        normals.size() != nV || 
        getPrimitive() != TRIANGLES)
        return false;

    //The number of the triangles
    size_t nT = indices.size() / 3;

    std::vector<glm::vec3> tan1( nV );
    std::vector<glm::vec3> tan2( nV );

    //Scan all the triangles. For each triangle add its
    //normal to norm's vectors of triangle's vertices
    for (size_t t = 0; t < nT; t++) {

        //Get indices of the triangle t
        int i1 = indices[ 3 * t ];
        int i2 = indices[ 3 * t + 1 ];
        int i3 = indices[ 3 * t + 2 ];

        //Get vertices of the triangle
        const glm::vec3 &v1 = vertices[ i1 ];
        const glm::vec3 &v2 = vertices[ i2 ];
        const glm::vec3 &v3 = vertices[ i3 ];

        const glm::vec2 &w1 = texCoords[i1];
        const glm::vec2 &w2 = texCoords[i2];
        const glm::vec2 &w3 = texCoords[i3];

        float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;
        
        float s1 = w2.x - w1.x;
        float s2 = w3.x - w1.x;
        float t1 = w2.y - w1.y;
        float t2 = w3.y - w1.y;
        
        float r = 1.0f / (s1 * t2 - s2 * t1);
        glm::vec3 sdir( (t2 * x1 - t1 * x2) * r, 
                        (t2 * y1 - t1 * y2) * r, 
                        (t2 * z1 - t1 * z2) * r);
        glm::vec3 tdir( (s1 * x2 - s2 * x1) * r, 
                        (s1 * y2 - s2 * y1) * r, 
                        (s1 * z2 - s2 * z1) * r);
        
        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;
        
        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;
    }

    //Normalize the normal's length and add it.
    tangents.clear();
    for (size_t i = 0; i < nV; i++) {
        const glm::vec3 &n = normals[i];
        const glm::vec3 &t = tan1[i];
        
        // Gram-Schmidt orthogonalize
        glm::vec3 tangent = t - n * glm::dot(n, t);

        // Calculate handedness
        float hardedness = (glm::dot( glm::cross(n, t), tan2[i]) < 0.0f) ? -1.0f : 1.0f;

        addTangent(glm::vec4(tangent, hardedness));
    }

    return true;
}
