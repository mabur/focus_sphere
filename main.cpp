#include <algorithm>
#include <functional>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <valarray>

using namespace std;
using vec = valarray<double>;
double dot(vec a, vec b)
{
    return (a * b).sum();
}
double norm(vec a)
{
    return sqrt(dot(a, a));
}
vec normalized(vec a)
{
    return a / norm(a);
}

const size_t IMAGE_WIDTH  = 1000;
const size_t IMAGE_HEIGHT = 1000;
const double RADIUS = min(IMAGE_HEIGHT, IMAGE_WIDTH) * 0.4;
const size_t NUM_STEPS = 8000;
const double STEP_SIZE = 0.09;
const size_t NUM_SAMPLES_PER_LINE = 30;

auto g = bind(      normal_distribution<double>(), mt19937(0));
auto u = bind(uniform_real_distribution<double>(), mt19937(0));

vec random_direction()
{
    return normalized({ g(), g(), g() });
}

vector<double> random_walk_on_sphere()
{
    auto line_segments = vector<vec>{};
    auto point = random_direction();
    for (size_t i = 0; i < NUM_STEPS; ++i)
    {
        auto dir = random_direction();
        point = normalized(point + STEP_SIZE * dir);
        line_segments.push_back(point);
    }

    auto image = vector<double>(IMAGE_WIDTH * IMAGE_HEIGHT, 0.0);
    for (size_t i = 0; i < line_segments.size() - 1; ++i)
    {
        const auto& p0 = line_segments[i + 0];
        const auto& p1 = line_segments[i + 1];
        for (size_t s = 0; s < NUM_SAMPLES_PER_LINE; ++s)
        {
            const auto d = u();
            const auto point = (1.0 - d) * p0 + d * p1;
            // point = unfocus(point);
            const auto x = static_cast<size_t>(point[0] * RADIUS) + IMAGE_WIDTH  / 2;
            const auto y = static_cast<size_t>(point[1] * RADIUS) + IMAGE_HEIGHT / 2;
            image[y * IMAGE_WIDTH + x] = 1;
        }
    }
    const auto maximum = *max_element(begin(image), end(image));
    const auto normalization_factor = 1.0 / maximum;
    for (auto& pixel : image)
    {
        pixel *= normalization_factor;
    }
    return image;
}

//vec unfocus(vec v)
//{
//    const auto BLUR = 1.0;
//    const auto z = v[2];
//    const auto dir = random_direction();
//    const auto offset = scale(BLUR * z, dir);
//    return add(v, offset);
//}

vector<double> write_image(const vector<double>& image, const char* filename)
{
    ofstream file(filename);
    file << "P3" << endl << IMAGE_WIDTH << " " << IMAGE_HEIGHT << endl << "255" << endl;
    for (const auto pixel : image)
    {
        auto NUM_COLOR_CHANNELS = 3;
        for (auto c = 0; c < NUM_COLOR_CHANNELS; ++c)
            file << min(int(pixel * 255.0), 255) << " ";
    }
    file.close();
}

int main()
{
    const auto image = random_walk_on_sphere();
    write_image(image, "image.ppm");
    return 0;
}
