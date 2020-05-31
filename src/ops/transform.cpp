#include "hilma/ops/transform.h"
#include "hilma/ops/compute.h"

// #define GLM_ENABLE_EXPERIMENTAL 
// #include <glm/gtx/quaternion.hpp>

namespace hilma {

void transform(std::vector<glm::vec3>& _points, const glm::quat& _quat) {
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        *it = _quat * *it;
}

void transform(std::vector<glm::vec3>& _points, const glm::mat3& _mat) {
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        *it = _mat * (*it);
}

void transform(std::vector<glm::vec3>& _points, const glm::mat4& _mat) {
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        *it = glm::vec3(_mat * glm::vec4(*it, 0.0));
}

void scale(std::vector<glm::vec3>& _points, float _v){
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        *it *= _v;
}

void scaleX(std::vector<glm::vec3>& _points, float _x){
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        it->x *= _x;
}

void scaleY(std::vector<glm::vec3>& _points, float _y){
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        it->y *= _y;
}

void scaleZ(std::vector<glm::vec3>& _points, float _z){
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        it->z *= _z;
}

void scale(std::vector<glm::vec3>& _points, float _x, float _y, float _z){
    scale(_points, glm::vec3(_x, _y, _z));
}

void scale(std::vector<glm::vec3>& _points, const glm::vec3& _v ){
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        *it *= _v;
}


void translateX(std::vector<glm::vec3>& _points, float _x){
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        it->x += _x;
}

void translateY(std::vector<glm::vec3>& _points, float _y){
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        it->y += _y;
}

void translateY(std::vector<glm::vec3>& _points, const Image& _grayscale) {
    if (_grayscale.getChannels() > 1)
        return;

    int width = _grayscale.getWidth();
    int height = _grayscale.getHeight();
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it) {
        int index = _grayscale.getIndex(int(it->x) % width, int(it->z) % height);
        it->y += _grayscale.getValue(index);
    }
}

void translateZ(std::vector<glm::vec3>& _points, const Image& _grayscale) {
    if (_grayscale.getChannels() > 1)
        return;

    int width = _grayscale.getWidth();
    int height = _grayscale.getHeight();
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it) {
        int index = _grayscale.getIndex(int(it->x) % width, int(it->y) % height);
        it->z += _grayscale.getValue(index);
    }
}

void translateZ(std::vector<glm::vec3>& _points, float _z){
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        it->z += _z;
}

void translate(std::vector<glm::vec3>& _points, float _x, float _y, float _z){
    translate(_points, glm::vec3(_x, _y, _z));
}

void translate(std::vector<glm::vec3>& _points, const glm::vec3& _v) {
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        *it += _v;
}

void rotate(std::vector<glm::vec3>& _points, float _rad, const glm::vec3& _axis ) {
    glm::quat q = glm::angleAxis(_rad, _axis);
    glm::mat3 M = glm::mat3_cast(q);
    for (std::vector<glm::vec3>::iterator it = _points.begin(); it != _points.end(); ++it)
        *it = M * *it;
}

void rotate(std::vector<glm::vec3>& _points, float _rad, float _x, float _y, float _z ) {
    rotate( _points, _rad, glm::vec3(_x, _y, _z));
}

void rotateX(std::vector<glm::vec3>& _points, float _rad){
    rotate( _points, _rad, 1.0f, 0.0f, 0.0f);
}

void rotateY(std::vector<glm::vec3>& _points, float _rad){
    rotate( _points, _rad, 0.0f, 1.0f, 0.0f);
}

void rotateZ(std::vector<glm::vec3>& _points, float _rad){
    rotate( _points, _rad, 0.0f, 0.0f, 1.0f);
}

void center(std::vector<glm::vec3>& _points){
    BoundingBox bbox = getBoundingBox(_points);
    glm::vec3 center = bbox.getCenter();
    translate(_points, -center);
}

void rotateX(Mesh& _mesh, float _rad) {
    rotateX(_mesh.vertices, _rad);
    if (_mesh.haveNormals()) rotateX(_mesh.normals, _rad);
}

void rotateY(Mesh& _mesh, float _rad) {
    rotateY(_mesh.vertices, _rad);
    if (_mesh.haveNormals()) rotateY(_mesh.normals, _rad);
}

void rotateZ(Mesh& _mesh, float _rad) {
    rotateZ(_mesh.vertices, _rad);
    if (_mesh.haveNormals()) rotateZ(_mesh.normals, _rad);
}

void rotate(Mesh& _mesh, float _rad, const glm::vec3& _axis ) {
    rotate(_mesh.vertices, _rad, _axis);
    if (_mesh.haveNormals()) rotate(_mesh.normals, _rad, _axis);
}

void rotate(Mesh& _mesh, float _rad, float _x, float _y, float _z ) {
    rotate(_mesh.vertices, _rad, _x, _y, _z);
    if (_mesh.haveNormals()) rotate(_mesh.normals, _rad, _x, _y, _z);
}

void rotateX(Polyline& _polyline, float _rad) {
    rotateX(_polyline.points, _rad);
}

void rotateY(Polyline& _polyline, float _rad) {
    rotateY(_polyline.points, _rad);
}

void rotateZ(Polyline& _polyline, float _rad) {
    rotateZ(_polyline.points, _rad);
}

void rotate(Polyline& _polyline, float _rad, const glm::vec3& _axis ) {
    rotate(_polyline.points, _rad, _axis);
}

void rotate(Polyline& _polyline, float _rad, float _x, float _y, float _z ) {
    rotate(_polyline.points, _rad, _x, _y, _z);
}



}
