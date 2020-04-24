#include "PixelGrid.hpp"
#include "Utils.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <type_traits>

template <class T>
T &PixelGrid<T>::operator()(int row, int col)
{
    return pixelAt(row, col);
}

template <class T>
T &PixelGrid<T>::pixelAt(int row, int col)
{
    if (row >= height || row < 0 || col >= width || col < 0)
    {
        return pixelData[height * width];
    }
    return pixelData[row * width + col];
}

template <class T>
T &PixelGrid<T>::pixelAt(int idx)
{
    if (idx < 0 || idx >= height * width)
    {
        return pixelData[height * width];
    }
    return pixelData[idx];
}

template <class T>
T const &PixelGrid<T>::read(int row, int col) const {
    if (row >= height || row < 0 || col >= width || col < 0)
    {
        return pixelData[height * width];
    }
    return pixelData[row * width + col];
}

template <class T>
PixelGrid<T>::PixelGrid(int width, int height) : width(width), height(height)
{
    pixelData.resize(width * height + 1);
}

template <class T>
PixelGrid<T>::PixelGrid() : width(0), height(0)
{
    pixelData.resize(1);
}

template <class T>
int PixelGrid<T>::getHeight() const
{
    return height;
}

template <class T>
int PixelGrid<T>::getWidth() const
{
    return width;
}

template <>
void PixelGrid<Color>::display() const
{
    FILE *f;

    f = popen("display", "w");

    fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
    for (int y = height - 1; y >= 0; y--)
    //for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            Color color = read(y, x);
            fprintf(f, "%d %d %d ", color.r, color.g, color.b);
        }
        fprintf(f, "\n");
    }
    pclose(f);
}

template class PixelGrid<Color>;
template class PixelGrid<double>;