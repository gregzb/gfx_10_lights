#pragma once

#include <vector>
#include <string>

#include "Vec4.hpp"
#include "Utils.hpp"

class Mat4
{
private:
    std::vector<std::vector<double>> m;
    int cols;
    int rows;

public:
    Mat4(int cols = 0);
    Mat4(std::vector<std::vector<double>> m);
    std::vector<double> &operator[](int idx);
    double operator()(int row, int col) const;
    int getRows() const;
    int getCols() const;
    std::string toString() const;

    Vec4 getPoint(int col) const;
    void setPoint(int col, Vec4 vec);

    void clear();
    Mat4 multiply(const Mat4 &matrix) const;
    void multiplyMutate(const Mat4 &matrix);
    void addPoint(Vec4 const &v);
    void addEdge(Vec4 const &v0, Vec4 v1);
    void addTriangle(Vec4 const &v0, Vec4 const &v1, Vec4 const &v2);
    void addQuad(Vec4 const &v0, Vec4 const &v1, Vec4 const &v2, Vec4 const &v3);

    void addCircle(Vec4 const &v, double r, int steps);
    void addCurve(double x0, double y0, double x1, double y1, double rx0, double ry0, double rx1, double ry1, int steps, CurveType curveType);

    void addBox(Vec4 const &v, Vec4 const &dims);
    void addSphere(Vec4 const &v, double r, int thetaSteps, int phiSteps);
    void addTorus(Vec4 const &v, double r1, double r2, int thetaSteps, int phiSteps);

    static Mat4 identity();

    static Mat4 rotX(double theta);
    static Mat4 rotY(double theta);
    static Mat4 rotZ(double theta);
    static Mat4 translate(Vec4 const &v);
    static Mat4 scale(Vec4 const &v);
};