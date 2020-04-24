#include <fstream>
#include <cmath>
#include "Screen.hpp"
#include "PixelGrid.hpp"

#include "Light.hpp"

Screen::Screen(int width, int height) : PixelGrid(width, height), zbuffer(width, height)
{
    clearZbuf();
}

void Screen::toFileAscii(std::string fileName)
{
    std::ofstream myfile;
    myfile.open(fileName);
    myfile << "P3\n"
           << width << " " << height << "\n255\n";

    for (int row = getHeight() - 1; row >= 0; row--)
    {
        for (int col = 0; col < getWidth(); col++)
        {
            Color color = pixelAt(row, col);
            myfile << " " << +color.r << " " << +color.g << " " << +color.b << "  ";
        }
        myfile << "\n";
    }
    myfile.close();
}

void Screen::toFile(std::string fileName)
{
    std::ofstream myfile;
    myfile.open(fileName);
    myfile << "P6\n"
           << width << " " << height << "\n255\n";

    for (int row = getHeight() - 1; row >= 0; row--)
    {
        for (int col = 0; col < getWidth(); col++)
        {
            Color color = pixelAt(row, col);
            myfile << color.r << color.g << color.b;
        }
    }
    myfile.close();
}

void Screen::toFileExtension(std::string fileName)
{
    std::string command = "convert - ";
    command.append(fileName);

    FILE *f = popen(command.c_str(), "w");
    fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
    for (int y = height - 1; y >= 0; y--)
    //for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            Color color = pixelAt(y, x);
            fprintf(f, "%d %d %d ", color.r, color.g, color.b);
        }
        fprintf(f, "\n");
    }
    pclose(f);
}

void Screen::display()
{
    FILE *f;

    f = popen("display", "w");

    fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
    for (int y = height - 1; y >= 0; y--)
    {
        for (int x = 0; x < width; x++)
        {
            Color color = pixelAt(y, x);
            fprintf(f, "%d %d %d ", color.r, color.g, color.b);
        }
        fprintf(f, "\n");
    }
    pclose(f);
}

void Screen::plot(Vec4 const &pixel, Color color)
{
    if (zbuf(pixel.getY(), pixel.getX()) < pixel.getZ())
    {
        zbuf(pixel.getY(), pixel.getX()) = pixel.getZ();
        pixelAt(pixel.getY(), pixel.getX()) = color;
    }
}

void Screen::drawLine(Vec4 const &a, Vec4 const &b, Color color)
{
    drawLine(a, b, color, false);
}

void Screen::drawLine(Vec4 const &a, Vec4 const &b, Color color, bool flipped)
{
    if (b.getX() < a.getX())
    {
        return drawLine(b, a, color, flipped);
    }

    long x0r = std::lround(a.getX()), x1r = std::lround(b.getX()), y0r = std::lround(a.getY()), y1r = std::lround(b.getY());

    long dY = y1r - y0r, dX = x1r - x0r;

    if (std::abs(dY) > dX)
    {
        return drawLine({a.getY(), a.getX()}, {b.getY(), b.getX()}, color, !flipped);
    }

    int dir = Utils::sign(dY);

    dY = std::abs(dY) * 2;

    int d = dY - dX;

    dX *= 2;

    int y = y0r;
    for (int x = x0r; x <= x1r; x++)
    {
        if (!flipped)
            pixelAt(y, x) = color;
        else
            pixelAt(x, y) = color;

        if (d > 0)
        {
            y += dir;
            d -= dX;
        }
        d += dY;
    }
}

void Screen::drawEdges(Mat4 const &matrix, Color color)
{
    for (int col = 0; col < matrix.getCols(); col += 2)
    {
        drawLine(matrix.getPoint(col), matrix.getPoint(col + 1), color);
    }
}

void Screen::drawTriangles(Mat4 const &matrix, Color color)
{
    std::vector<Vec4> verts(3);
    for (int col = 0; col < matrix.getCols(); col += 3)
    {
        Vec4 pos;
        for (int p = 0; p < 3; p++)
        {
            verts[p] = matrix.getPoint(col + p);
            pos = pos + verts[p];
        }
        pos = pos / 3;

        Vec4 normal = verts[0].getNormal(verts[1], verts[2]);

        if (normal.getZ() < std::numeric_limits<double>::epsilon() * 100) //this works in screen space :)
        {
            continue;
        }

        normal = normal.normalize();

        //Color color = {(col * 4 + 40) % 256, 0, (col * 6 + 90) % 256, 255};

        Vec4 originalColor = {200/256.0, 50/256.0, 50/256.0, 1.0};
        double Ns = 100;

        Vec4 illum;
        for (auto const &light : WORLD_LIGHTS)
        {
            if (light.type == LightType::Ambient)
            {
                illum = illum + originalColor * light.Ia;
            }
            else if (light.type == LightType::Directional)
            {
                Vec4 const &L = light.pos.negate().normalize();
                Vec4 const &V = pos.negate().normalize();
                Vec4 const &H = (L + V).normalize();
                double NL = normal.dot(L);
                int B = NL > 0.00000001 ? 1 : 0;
                Vec4 const &diffuse = originalColor * light.Id * B * (NL);
                double inner = ((normal * 2) * NL - L).getZ();
                Vec4 const &specular = originalColor * light.Is * std::pow(inner > 0.000001 ? inner : 0, 10);
                //Vec4 const &diffuse = originalColor * light.Id * B * (NL);
                //Vec4 const &specular = originalColor * light.Is * B * std::pow(normal.dot(H), Ns);
                illum = illum + diffuse + specular;
            }
        }

        //std::cout << illum << std::endl;

        illum.set(0, Utils::clamp(illum[0], 0, 1));
        illum.set(1, Utils::clamp(illum[1], 0, 1));
        illum.set(2, Utils::clamp(illum[2], 0, 1));

        fillTriangle(verts, illum.toColor());
        // for (int i = 0; i < 3; i++) {
        //     drawLine(verts[i], verts[(i+1)%verts.size()], {255, 255, 255, 255});
        // }
    }
}

struct LineInfo
{
    double x, xStepY;
    double z, zStepY;
};

void Screen::fillTriangle(std::vector<Vec4> &verts, Color const &color)
{
    if (verts[0].getY() > verts[1].getY())
        std::swap(verts[0], verts[1]);
    if (verts[1].getY() > verts[2].getY())
        std::swap(verts[1], verts[2]);
    if (verts[0].getY() > verts[1].getY())
        std::swap(verts[0], verts[1]);

    Vec4 const &vert20 = verts[2] - verts[0];
    Vec4 const &vert10 = verts[1] - verts[0];
    Vec4 const &vert21 = verts[2] - verts[1];

    double x0Step = vert20[0] / vert20[1];
    double x1Step = vert10[0] / vert10[1];
    double x2Step = vert21[0] / vert21[1];

    double z0StepY = vert20[2] / vert20[1];
    double z1StepY = vert10[2] / vert10[1];
    double z2StepY = vert21[2] / vert21[1];

    int y = std::ceil(verts[0][1]);

    double yOffset0 = y - verts[0][1];
    double yOffset1 = std::ceil(verts[1][1]) - verts[1][1];

    double x0 = verts[0][0] + yOffset0 * x0Step;
    double x1 = verts[0][0] + yOffset0 * x1Step;
    double x2 = verts[1][0] + yOffset1 * x2Step;

    double z0 = verts[0][2] + yOffset0 * z0StepY;
    double z1 = verts[0][2] + yOffset0 * z1StepY;
    double z2 = verts[1][2] + yOffset1 * z2StepY;

    LineInfo left{x0, x0Step, z0, z0StepY};
    LineInfo right{x1, x1Step, z1, z1StepY};
    LineInfo temp{x2, x2Step, z2, z2StepY};

    bool side = vert10.cross(vert20).getZ() >= 0;

    if (!side)
        std::swap(left, right);

    for (int i = 1; i < 3; i++)
    {
        int goal = std::ceil(verts[i][1]);
        while (y < goal)
        {
            double xOffset = std::ceil(left.x) - left.x;
            double zStep = (right.z - left.z) / (right.x - left.x);
            double z = left.z + zStep * xOffset;

            for (unsigned int x = std::ceil(left.x); x < std::ceil(right.x); x++)
            {
                if (zbuf(y, x) < z)
                {
                    zbuf(y, x) = z;
                    pixelAt(y, x) = color;
                }
                z += zStep;
            }
            left.x += left.xStepY;
            left.z += left.zStepY;
            right.x += right.xStepY;
            right.z += right.zStepY;
            y++;
        }
        if (!side)
            left = temp;
        else
            right = temp;
    }
}

void Screen::clear(Color color)
{
    for (int row = 0; row < getHeight(); row++)
    {
        for (int col = 0; col < getWidth(); col++)
        {
            pixelAt(row, col) = color;
        }
    }
}

PixelGrid<double> &Screen::zbuf()
{
    return zbuffer;
}

double &Screen::zbuf(int row, int col)
{
    return zbuffer(row, col);
}

void Screen::clearZbuf(double value)
{
    for (int row = 0; row < zbuf().getHeight(); row++)
    {
        for (int col = 0; col < zbuf().getWidth(); col++)
        {
            zbuf(row, col) = value;
        }
    }
}