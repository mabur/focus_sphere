#include <algorithm>
#include <functional>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <valarray>

using namespace std;

const size_t IMAGE_WIDTH  = 360;
const size_t IMAGE_HEIGHT = 360;
const double RADIUS = min(IMAGE_HEIGHT, IMAGE_WIDTH) * 0.4;

auto g = bind(      normal_distribution<double>(), mt19937(0));
auto u = bind(uniform_real_distribution<double>(), mt19937(0));
// Definition of vector type:
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
vec random_direction()
{
    return normalized({ g(), g(), g() });
}
int screen_color(double c)
{
    return min(int(c * 255.0), 255);
    //return min(int(pow(c * 255.0, 1.0 / 2.2)), 255);
}

vector<vec> sample_sphere(size_t num_points)
{
    auto points = vector<vec>{};
    for (size_t i = 0; i < num_points; ++i)
    {
        const auto point = random_direction();
        points.push_back(point);
    }
    return points;
}

vector<vec> random_walk_on_sphere(size_t num_steps, double step_size)
{
    auto points = vector<vec>{};
    auto point = random_direction();
    for (size_t i = 0; i < num_steps; ++i)
    {
        auto dir = random_direction();
        point = normalized(point + step_size * dir);
        points.push_back(point);
    }
    return points;
}

vector<double> render_points(const vector<vec>& points)
{
    auto image = vector<double>(IMAGE_WIDTH * IMAGE_HEIGHT, 0.0);
    for (const auto& point : points)
    {
        const auto x = static_cast<size_t>(point[0] * RADIUS) + IMAGE_WIDTH  / 2;
        const auto y = static_cast<size_t>(point[1] * RADIUS) + IMAGE_HEIGHT / 2;

        ++image[y * IMAGE_WIDTH + x];
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

vector<double> render_line_segments(const vector<vec>& line_segments)
{
    auto image = vector<double>(IMAGE_WIDTH * IMAGE_HEIGHT, 0.0);
    for (size_t i = 0; i < line_segments.size() - 1; ++i)
    {
        const auto& p0 = line_segments[i + 0];
        const auto& p1 = line_segments[i + 1];
        const auto NUM_SAMPLES = size_t{30};
        for (size_t s = 0; s < NUM_SAMPLES; ++s)
        {
            const auto d = u();
            const auto point = (1.0 - d) * p0 + d * p1;
            // point = unfocus(point);
            const auto x = static_cast<size_t>(point[0] * RADIUS) + IMAGE_WIDTH  / 2;
            const auto y = static_cast<size_t>(point[1] * RADIUS) + IMAGE_HEIGHT / 2;
            image[y * IMAGE_WIDTH + x] = 1;
        }

//        const auto x = static_cast<size_t>(p0[0] * RADIUS) + IMAGE_WIDTH  / 2;
//        const auto y = static_cast<size_t>(p0[1] * RADIUS) + IMAGE_HEIGHT / 2;
//        ++image[y * IMAGE_WIDTH + x];
//        image[y * IMAGE_WIDTH + x] = 1.0;
    }
    const auto maximum = *max_element(begin(image), end(image));
    const auto normalization_factor = 1.0 / maximum;
    for (auto& pixel : image)
    {
        pixel *= normalization_factor;
    }
    return image;
}

// Render the image to a ppm-file:
vector<double> write_image(const vector<double>& image, const char* filename)
{
    ofstream file(filename);
    file << "P3" << endl << IMAGE_WIDTH << " " << IMAGE_HEIGHT << endl << "255" << endl;
    for (const auto pixel : image)
    {
        auto NUM_COLOR_CHANNELS = 3;
        for (auto c = 0; c < NUM_COLOR_CHANNELS; ++c)
            file << screen_color(pixel) << " ";
    }
    file.close();
}

int main()
{
    auto num_steps = 8000;
    auto step_size = 0.09;
    const auto points = random_walk_on_sphere(num_steps, step_size);
    const auto image = render_line_segments(points);
    write_image(image, "image.ppm");
    return 0;
}
