#include "DataStructure/Vector3.h"
#include <math.h>
#include "Utils/Globals.h"

Vector3 Vector3::nabla = Vector3(1.f, 1.f, 1.f).normalize();

Vector3::Vector3(float x, float y, float z, bool valid) : x(x), y(y), z(z), valid(valid) {

}
Vector3::Vector3() : Vector3(0.f, 0.f, 0.f) {

}
Vector3::Vector3(const Vector3& copy) : Vector3(copy.x, copy.y, copy.z, copy.valid) {

}
Vector3::Vector3(Vector3* copy) : Vector3(copy->x, copy->y, copy->z, copy->valid) {

}

Vector3::Vector3(qglviewer::Vec other)
    : Vector3(other.x, other.y, other.z)
{

}

Vector3::Vector3(bool valid) : Vector3()
{
    this->valid = valid;
}

Vector3::Vector3(const float *coords, bool valid)
    : Vector3(coords[0], coords[1], coords[2], valid)
{

}

float Vector3::norm() {
    if(this->x == 0 && this->y == 0 && this->z == 0) return 0;
    return sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
}
float Vector3::norm2() {
    if(this->x == 0 && this->y == 0 && this->z == 0) return 0;
    return this->x * this->x + this->y * this->y + this->z * this->z;
}

Vector3& Vector3::normalize() {
    if(this->norm() < 1e-5)
        return *this;
    float norm = this->norm();
    *this /= norm;
    return *this;
}
Vector3 Vector3::normalized() const {
    Vector3 a = *this;
    return a.normalize();
}

Vector3 Vector3::abs()
{
    Vector3 a = *this;
    a.x = std::abs(x);
    a.y = std::abs(y);
    a.z = std::abs(z);
    return a;
}

Vector3& Vector3::setMag(float newMag)
{
    this->normalize();
    this->x *= newMag;
    this->y *= newMag;
    this->z *= newMag;
    return *this;
}

bool Vector3::isAlmostVertical()
{
    return std::abs(this->dot(Vector3(0, 0, 1))) > 0.999;
}

Matrix Vector3::toMatrix()
{
    return Matrix(3, 1, (float*)(*this));
}

float Vector3::dot(Vector3 o) {
    return (this->x * o.x) + (this->y * o.y) + (this->z * o.z);
}
Vector3 Vector3::cross(Vector3 o) {
    Vector3 v(this->y * o.z - this->z * o.y,
              this->z * o.x - this->x * o.z,
              this->x * o.y - this->y * o.x);
    return v;
}
Vector3 Vector3::rounded(int precision) const
{
    Vector3 v = *this;
    v.x = (int)(v.x * pow(10, precision)) / (float)(pow(10, precision));
    v.y = (int)(v.y * pow(10, precision)) / (float)(pow(10, precision));
    v.z = (int)(v.z * pow(10, precision)) / (float)(pow(10, precision));
    return v;
}
Vector3 Vector3::floor() const
{
    Vector3 v = *this;
    v.x = std::floor(v.x);
    v.y = std::floor(v.y);
    v.z = std::floor(v.z);
    return v;
}
Vector3 Vector3::ceil() const
{
    Vector3 v = this->floor();
    if (v.x != x) v.x += 1;
    if (v.y != y) v.y += 1;
    if (v.z != z) v.z += 1;
    return v;
}

Vector3 Vector3::random() {
    Vector3 v(random_gen::generate(-1.0, 1.0), random_gen::generate(-1.0, 1.0), random_gen::generate(-1.0, 1.0));
    v.normalize();
    return v;
}

Vector3 Vector3::random(float norm)
{
    return Vector3::random() * norm;
}

Vector3 Vector3::random(float minNorm, float maxNorm)
{
    return Vector3::random() * random_gen::generate(minNorm, maxNorm);
}

Vector3 Vector3::random(Vector3 maxValues)
{
    return Vector3::random(Vector3(), maxValues);
}

Vector3 Vector3::random(Vector3 minValues, Vector3 maxValues)
{
    return Vector3(random_gen::generate(minValues.x, maxValues.x),
                   random_gen::generate(minValues.y, maxValues.y),
                   random_gen::generate(minValues.z, maxValues.z));
}

std::vector<float> Vector3::toArray(Vector3 v)
{
    std::vector<float> arr;
    arr.insert(arr.end(), {v.x, v.y, v.z});
    return arr;
}
std::vector<float> Vector3::toArray(std::vector<Vector3> vs)
{
    std::vector<float> arr;
    for (Vector3& v : vs)
        arr.insert(arr.end(), {v.x, v.y, v.z});
    return arr;
}

Vector3 Vector3::min()
{
    return Vector3(std::numeric_limits<float>::min(),
                   std::numeric_limits<float>::min(),
                   std::numeric_limits<float>::min());
}

Vector3 Vector3::max()
{
    return Vector3(std::numeric_limits<float>::max(),
                   std::numeric_limits<float>::max(),
                   std::numeric_limits<float>::max());
}

Vector3 Vector3::min(Vector3 a, Vector3 b)
{
    return Vector3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

Vector3 Vector3::max(Vector3 a, Vector3 b)
{
    return Vector3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}


Vector3& Vector3::rotate(float angle_x, float angle_y, float angle_z) {
    return this->rotate(Vector3(angle_x, angle_y, angle_z));
}
Vector3& Vector3::rotate(Vector3 eulerAngles) {
    Matrix Rx (3, 3, std::vector<float>({
                   1, 0, 0,
                   0, cos(eulerAngles.x), -sin(eulerAngles.x),
                   0, sin(eulerAngles.x), cos(eulerAngles.x)
               }).data());
    Matrix Rz (3, 3, std::vector<float>({
                    cos(eulerAngles.y), 0, -sin(eulerAngles.y),
                    0, 1, 0,
                    sin(eulerAngles.y), 0, cos(eulerAngles.y)
                }).data());
    Matrix Ry (3, 3, std::vector<float>({
                    cos(eulerAngles.z), -sin(eulerAngles.z), 0,
                    sin(eulerAngles.z), cos(eulerAngles.z), 0,
                    0, 0, 1
                }).data());
    Matrix R = Rx.product(Ry).product(Rz);
    Matrix newCoords = R.product(this->toMatrix());
    this->x = newCoords[0][0];
    this->y = newCoords[1][0];
    this->z = newCoords[2][0];
    return *this;
}
Vector3 Vector3::rotated(float angle_x, float angle_y, float angle_z) {
    return this->rotated(Vector3(angle_x, angle_y, angle_z));
}
Vector3 Vector3::rotated(Vector3 eulerAngles) {
    Vector3 v = *this;
    return v.rotate(eulerAngles);
}
Vector3& Vector3::rotate(float angle, float dir_x, float dir_y, float dir_z) {
    return this->rotate(angle, Vector3(dir_x, dir_y, dir_z));
}
Vector3& Vector3::rotate(float angle, Vector3 direction) {
    float c = cos(angle), s = sin(angle);
    Vector3 v = direction.normalized(); // alias
    Matrix R (3, 3, std::vector<float>({
                   v.x*v.x*(1-c)+c, v.x*v.y*(1-c)-v.z*s, v.x*v.z*(1-c)+v.y*s,
                   v.x*v.y*(1-c)+v.z*s, v.y*v.y*(1-c)+c, v.y*v.z*(1-c)-v.x*s,
                   v.x*v.z*(1-c)-v.y*s, v.y*v.z*(1-c)+v.x*s, v.z*v.z*(1-c)+c
               }).data());
    Matrix newCoords = R.product(this->toMatrix());
    this->x = newCoords[0][0];
    this->y = newCoords[1][0];
    this->z = newCoords[2][0];
    return *this;
}
Vector3 Vector3::rotated(float angle, float dir_x, float dir_y, float dir_z) {
    return this->rotated(angle, Vector3(dir_x, dir_y, dir_z));
}
Vector3 Vector3::rotated(float angle, Vector3 direction) {
    Vector3 v = *this;
    return v.rotate(angle, direction);
}
Vector3& Vector3::translate(float move_x, float move_y, float move_z) {
    return this->translate(Vector3(move_x, move_y, move_z));
}
Vector3& Vector3::translate(Vector3 move) {
    return (*this += move);
}
Vector3 Vector3::translated(float move_x, float move_y, float move_z) {
    return this->translated(Vector3(move_x, move_y, move_z));
}
Vector3 Vector3::translated(Vector3 move) {
    Vector3 v = *this;
    return v.translate(move);
}

Vector3 &Vector3::changeBasis(Vector3 newX, Vector3 newY, Vector3 newZ)
{
    Vector3 newVec = this->changedBasis(newX, newY, newZ);
    this->x = newVec.x;
    this->y = newVec.y;
    this->z = newVec.z;
    return *this;
}

Vector3 Vector3::changedBasis(Vector3 newX, Vector3 newY, Vector3 newZ)
{
    Vector3 newVec;
    newVec += newX * this->x + newY * this->y + newZ * this->z;
    return newVec;
}

Vector3 Vector3::reflexion(Vector3 normal)
{
    float dot = normal.dot(*this);
    Vector3 n2d = normal * 2.f * dot;
    return *this - n2d;
}

//Vector3 &Vector3::setDirection(Vector3 dir, Vector3 upVector)
//{
//    Vector3 forward = Vector3(0, 0, 1);//upVector.normalized();
//    if (dir == forward) return *this;

//    Vector3 old_right = Vector3(1, 0, 0);//this->normalized();
//    Vector3 old_upVector = forward.cross(old_right);
//    Vector3 right = dir.cross(upVector);
////    dir -= *this->normalized();
//    forward.normalize(); upVector.normalize(); right.normalize(); old_upVector.normalize(); old_right.normalize();
////    float angle_heading = std::acos(dir.x) - std::acos(forward.x);
////    float angle_pitch = std::acos(dir.y) - std::acos(forward.y);
////    float angle_bank = std::acos(dir.z) - std::acos(forward.z);

//    float angle_heading = std::atan2(dir.y, dir.x) - std::atan2(forward.x, forward.y);
//    float angle_pitch = std::asin(dir.z) - std::asin(forward.z);
//    float angle_bank = std::atan2(right.dot(upVector) / right.norm(),
//                                  1.0) - std::atan2(old_right.dot(old_upVector) / old_right.norm(), 1.0);

//    /*std::cout << "Forward   : " << forward <<
//                 "\nold Right : " << old_right <<
//                 "\nold Up    : " << old_upVector <<
//                 "\nnew Forwa : " << dir <<
//                 "\nnew Right : " << right <<
//                 "\nnew Up    : " << upVector <<
//                 "\nrotation  : " << Vector3(angle_heading, angle_pitch, angle_bank) << std::endl;*/
//    return this->rotate(angle_heading, angle_pitch, angle_bank).normalize();

//    /*
//    dir.normalize(); upVector.normalize();
//    Vector3 right = dir.cross(upVector);
//    float angle_heading = std::atan2(dir.y, dir.x);
//    float angle_pitch = std::asin(dir.z);
//    float angle_bank = std::atan2(right.dot(upVector) / right.norm(),
//                                  1.0);
//    return this->rotate(angle_heading, angle_pitch, angle_bank);*/
//}






//Vector3 operator+(Vector3 a, Vector3&b) {
//    a += b;
//    return a;
//}
Vector3 operator+(Vector3 a, Vector3 b) {
    a += b;
    return a;
}
Vector3& Vector3::operator+=(const Vector3& o) {
    this->x += o.x;
    this->y += o.y;
    this->z += o.z;
    return *this;
}
Vector3 operator-(Vector3 a, Vector3 b) {
    a -= b;
    return a;
}
Vector3& Vector3::operator-=(const Vector3& o) {
    this->x -= o.x;
    this->y -= o.y;
    this->z -= o.z;
    return *this;
}
Vector3 operator*(Vector3 a, Vector3 b) {
    a *= b;
    return a;
}
Vector3& Vector3::operator*=(Vector3 o) {
    this->x *= o.x;
    this->y *= o.y;
    this->z *= o.z;
    return *this;
}
Vector3 Vector3::operator/(Vector3 o) {
    Vector3 v = *this;
    v /= o;
    return v;
}
Vector3& Vector3::operator/=(Vector3 o) {
    this->x /= o.x;
    this->y /= o.y;
    this->z /= o.z;
    return *this;
}/*
Vector3 operator/(Vector3 a, Vector3 b) {
    a /= b;
    return a;
}*/
Vector3 Vector3::operator*(float o) {
    Vector3 v = *this;
    v *= o;
    return v;
}
Vector3& Vector3::operator*=(float o) {
    this->x *= o;
    this->y *= o;
    this->z *= o;
    return *this;
}
Vector3 Vector3::operator/(float o) {
    Vector3 v = *this;
    v /= o;
    return v;
}
Vector3& Vector3::operator/=(float o) {
    this->x /= o;
    this->y /= o;
    this->z /= o;
    return *this;
}
Vector3 Vector3::operator+(float o) {
    Vector3 v = *this;
    v += o;
    return v;
}
Vector3& Vector3::operator+=(float o) {
    this->x += o;
    this->y += o;
    this->z += o;
    return *this;
}
Vector3 Vector3::operator-(float o) {
    Vector3 v = *this;
    v -= o;
    return v;
}
Vector3& Vector3::operator-=(float o) {
    this->x -= o;
    this->y -= o;
    this->z -= o;
    return *this;
}
Vector3& Vector3::operator=(const Vector3& o) {
    this->x = o.x;
    this->y = o.y;
    this->z = o.z;
    this->valid = o.valid;
    return *this;
}

Vector3 operator+(float a, Vector3 b) {
    return b + a;
}
Vector3 operator-(float a, Vector3 b) {
    return b * -1.f + a;
}
Vector3 operator*(float a, Vector3 b) {
    return b * a;
}

bool operator==(Vector3 a, Vector3 b)
{
    return int(a.x * 1000) == int(b.x * 1000) && int(a.y * 1000) == int(b.y * 1000) && int(a.z * 1000) == int(b.z * 1000);
}

Vector3 operator-(Vector3 v) {
    return v * -1.f;
}

bool operator!=(Vector3 a, Vector3 b)
{
    return !(a == b);
}

bool operator<(Vector3 a, Vector3 b)
{
    return a.norm2() < b.norm2();
}

bool operator<=(Vector3 a, Vector3 b)
{
    return !(a > b);
}

bool operator>(Vector3 a, Vector3 b)
{
    return a.norm2() > b.norm2();
}

bool operator>=(Vector3 a, Vector3 b)
{
    return !(a < b);
}

std::ostream& operator<<(std::ostream& io, const Vector3& v) {
    io << v.toString();
    return io;
}

std::ostream& operator<<(std::ostream& io, std::shared_ptr<Vector3> v) {
    io << v->toString();
    return io;
}

Vector3 std::abs(Vector3 o) { return o.abs(); }

bool Vector3::isInBox(Vector3 pos, Vector3 minPos, Vector3 maxPos) {
    return (minPos.x <= pos.x && pos.x <= maxPos.x) && (minPos.y <= pos.y && pos.y <= maxPos.y) && (minPos.z <= pos.z && pos.z <= maxPos.z);
//    return (pos - minPos).minComp() >= 0.f && (pos - (minPos + maxPos)).maxComp() <= 0.f;
}
float Vector3::signedDistanceToBoundaries(Vector3 pos, Vector3 minPos, Vector3 maxPos, bool ignoreZdimension)
{
    // TODO : Need to be improved one day
    pos -= minPos;
    maxPos -= minPos;

    if (ignoreZdimension) {
        pos = pos.xy();
        maxPos = maxPos.xy();
    }
    if (Vector3::isInBox(pos, Vector3(), maxPos)) {
        if (ignoreZdimension)
            return -std::min({pos.x, pos.y, maxPos.x - pos.x, maxPos.y - pos.y});
        else
            return -std::min({pos.x, pos.y, pos.z, maxPos.x - pos.x, maxPos.y - pos.y, maxPos.z - pos.z});
    } else {
        if (ignoreZdimension)
            return -std::min({pos.x, pos.y, maxPos.x - pos.x, maxPos.y - pos.y});
        else
            return -std::min({pos.x, pos.y, pos.z, maxPos.x - pos.x, maxPos.y - pos.y, maxPos.z - pos.z});
    }
    /*
    Vector3 halfRectDim = (maxPos - minPos) * .5f;
    pos = (pos - (minPos)).abs() - halfRectDim;

    if (ignoreZdimension) {
        pos = pos.xy();
        halfRectDim = halfRectDim.xy();
    }
    Vector3 posMinHalf = pos;
    float distOut = posMinHalf.norm();
    float distIn = pos.norm();
//    return Vector3::max(pos.abs(), Vector3()).norm();
    return (halfRectDim - pos.abs()).maxComp();



    pos -= minPos;
    maxPos -= minPos;

    Vector3 q = pos.abs() - maxPos; //abs(p) - b;
    float maxQ = Vector3::max(q, Vector3()).maxComp();
    float minQ = std::min(q.maxComp(), 0.f);
    return maxQ + minQ; // length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
    */
    /*
    if (ignoreZdimension) {
        pos = pos.xy();
        maxPos = maxPos.xy();
    }
    Vector3 posMinusMax = pos - maxPos;

    float res = 0.f;
    if (Vector3::isInBox(pos, Vector3(), maxPos)) {
        res = -std::max(-pos.minComp(), -((maxPos - pos).maxComp()));
    } else {
        res = std::min(pos.minComp(), -((pos - maxPos).maxComp()));
    }
    return res;*/

//    Vector3 d = pos.abs() - maxPos;
//    return Vector3::max(d, Vector3(0, 0, 0)).norm() + std::min(d.maxComp(), 0.f);


//    return -(Vector3::max(d, Vector3(0, 0, 0)).norm() + std::min(d.maxComp(), 0.f));
//    return Vector3::distanceToBoundaries(pos, minPos, maxPos, ignoreZdimension) * (Vector3::isInBox(pos, minPos, maxPos) ? 1.f : -1.f);
}

float Vector3::distanceToBoundaries(Vector3 pos, Vector3 minPos, Vector3 maxPos, bool ignoreZdimension)
{
    return std::abs(Vector3::signedDistanceToBoundaries(pos, minPos, maxPos, ignoreZdimension));
//    float x = std::min(std::abs(pos.x), std::abs(maxPos.x - pos.x));
//    float y = std::min(std::abs(pos.y), std::abs(maxPos.y - pos.y));
//    float z = std::min(std::abs(pos.z), std::abs(maxPos.z - pos.z)) * (ignoreZdimension ? 0.f : 1.f);
//    return std::max({x, y, z});
}


nlohmann::json vec3_to_json(const Vector3& vec) {
    return nlohmann::json({{"x", vec.x}, {"y", vec.y}, {"z", vec.z}});
}

Vector3 json_to_vec3(nlohmann::json json)
{
    return Vector3(json.at("x").get<float>(), json.at("y").get<float>(), json.at("z").get<float>());
}
