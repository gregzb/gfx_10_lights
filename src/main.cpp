#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <sstream>
#include <limits>

#include <random>
#include <unordered_map>
#include <chrono>

#include <utility>

#include <cmath>

#include "Utils.hpp"
#include "PixelGrid.hpp"
#include "Screen.hpp"
#include "Mat4.hpp"
#include "Vec4.hpp"

#include "Light.hpp"

std::vector<Light> WORLD_LIGHTS;

void parse(std::string fileName, Screen &screen, Mat4 &edges, Mat4 &triangles, std::vector<Mat4> &coordSystems, Color color)
{
    std::ifstream infile(fileName);

    std::string line;

    Vec4 offset;

    while (std::getline(infile, line))
    {
        std::cout << "command: " << line << std::endl;
        if (line == "line")
        {
            std::getline(infile, line);
            std::istringstream iss(line);

            Vec4 v0, v1;
            iss >> v0 >> v1;
            edges.addEdge(v0, v1);

            edges.multiplyMutate(coordSystems.back());
            screen.drawEdges(edges, {255, 255, 255, 255});
            edges.clear();
        }
        else if (line == "triangle")
        {
            std::getline(infile, line);
            std::istringstream iss(line);

            Vec4 v0, v1, v2;
            iss >> v0 >> v1 >> v2;
            triangles.addTriangle(v0, v1, v2);

            triangles.multiplyMutate(coordSystems.back());
            screen.drawTriangles(triangles, {255, 255, 255, 255});
            triangles.clear();
        }
        else if (line == "scale")
        {
            std::getline(infile, line);
            std::istringstream iss(line);

            Vec4 v;
            iss >> v;
            Mat4 const &temp = Mat4::scale(v);
            coordSystems.back() = coordSystems.back().multiply(temp);
            //transform.multiplyMutate(temp);
        }
        else if (line == "move" || line == "translate")
        {
            std::getline(infile, line);
            std::istringstream iss(line);

            Vec4 v;
            iss >> v;
            Mat4 const &temp = Mat4::translate(v);
            coordSystems.back() = coordSystems.back().multiply(temp);
            //transform.multiplyMutate(temp);
        }
        else if (line == "rotate")
        {
            std::getline(infile, line);
            std::istringstream iss(line);

            char axis;
            double angle;
            iss >> axis >> angle;

            double PI = std::atan(1) * 4;

            angle *= PI / 180;
            Mat4 temp(0);
            temp.identity();
            if (axis == 'x')
            {
                temp = Mat4::rotX(angle);
            }
            else if (axis == 'y')
            {
                temp = Mat4::rotY(angle);
            }
            else if (axis == 'z')
            {
                temp = Mat4::rotZ(angle);
            }
            //std::cout << coordSystems.back().toString() << std::endl;
            coordSystems.back() = coordSystems.back().multiply(temp);
            //transform.multiplyMutate(temp);
        }
        else if (line == "display")
        {
            screen.display();
        }
        else if (line == "save")
        {
            std::getline(infile, line);
            std::cout << "file name " << line << std::endl;
            screen.toFileExtension(line);
        }
        else if (line == "circle")
        {
            std::getline(infile, line);
            std::istringstream iss(line);

            Vec4 v;
            double r;
            iss >> v >> r;
            edges.addCircle(v, r, 100);

            edges.multiplyMutate(coordSystems.back());
            screen.drawEdges(edges, {255, 255, 255, 255});
            edges.clear();
        }
        else if (line == "hermite")
        {
            std::getline(infile, line);
            std::istringstream iss(line);

            double x0, y0, x1, y1, rx0, ry0, rx1, ry1;
            iss >> x0 >> y0 >> x1 >> y1 >> rx0 >> ry0 >> rx1 >> ry1;
            edges.addCurve(x0, y0, x1, y1, rx0, ry0, rx1, ry1, 100, CurveType::Hermite);

            edges.multiplyMutate(coordSystems.back());
            screen.drawEdges(edges, {255, 255, 255, 255});
            edges.clear();
        }
        else if (line == "bezier")
        {
            std::getline(infile, line);
            std::istringstream iss(line);

            double x0, y0, x1, y1, x2, y2, x3, y3;
            iss >> x0 >> y0 >> x1 >> y1 >> x2 >> y2 >> x3 >> y3;
            edges.addCurve(x0, y0, x1, y1, x2, y2, x3, y3, 100, CurveType::Bezier);
            //edges.addCurve(x0, y0, x3, y3, 3*(x1-x0), 3*(y1-y0), 3*(x3-x2), 3*(y3-y2), 50, CurveType::Hermite);
            edges.multiplyMutate(coordSystems.back());
            screen.drawEdges(edges, {255, 255, 255, 255});
            edges.clear();
        }
        else if (line == "box")
        {
            std::getline(infile, line);
            std::istringstream iss(line);

            Vec4 v, dims;
            iss >> v >> dims;
            triangles.addBox(v, dims);

            triangles.multiplyMutate(coordSystems.back());
            screen.drawTriangles(triangles, {255, 255, 255, 255});
            triangles.clear();
        }
        else if (line == "sphere")
        {
            std::getline(infile, line);
            std::istringstream iss(line);

            Vec4 v;
            double r;
            iss >> v >> r;

            triangles.addSphere(v, r, 80, 40);
            
            triangles.multiplyMutate(coordSystems.back());
            screen.drawTriangles(triangles, {255, 255, 255, 255});
            triangles.clear();
        }
        else if (line == "torus")
        {
            std::getline(infile, line);
            std::istringstream iss(line);

            Vec4 v;
            double r1, r2;
            iss >> v >> r1 >> r2;

            offset.set(3, 0);

            triangles.addTorus(v, r1, r2, 120, 48);

            triangles.multiplyMutate(coordSystems.back());

            Mat4 copy = triangles;
            for (int row = 0; row < 3; row++)
            {
                for (int thing = 0; thing < copy[row].size(); thing++)
                {
                    copy[row][thing] += offset[row];
                }
            }
            screen.drawTriangles(copy, {255, 0, 255, 255});

            //screen.drawTriangles(triangles, {255, 255, 255, 255});
            triangles.clear();
        }
        else if (line == "push")
        {
            coordSystems.push_back(coordSystems.back());
        }
        else if (line == "pop")
        {
            coordSystems.pop_back();
        }
        else if (line == "clear")
        {
            screen.clear({0, 0, 0, 255});
            screen.clearZbuf();
        }
        else if (line == "movescreen")
        {
            std::getline(infile, line);
            std::istringstream iss(line);

            iss >> offset;
        }
        else
        {
            std::cout << "Unrecognized command!" << std::endl;
        }
    }
}

int main()
{
    Screen screen(500, 500);

    Mat4 edges(0);
    Mat4 triangles(0);
    std::vector<Mat4> coordSystems;
    coordSystems.push_back(Mat4::identity());

    WORLD_LIGHTS.push_back({LightType::Ambient, {.15, .15, .15}});
    WORLD_LIGHTS.push_back({LightType::Directional, {}, {.6, .6, .6}, {.6, .6, .6}, {-1, -1, -1}});

    parse("script", screen, edges, triangles, coordSystems, {255, 127, 127, 255});

    return 0;
}
