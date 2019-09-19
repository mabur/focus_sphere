#include <algorithm>
#include <functional>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
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
const size_t NUM_SAMPLES_PER_LINE = 100;
const double BRIGHTNESS = 1000.0;
const auto BLUR_SCALING = {0.01, 0.02, 0.04, 0.08};
const auto BLUR_EXPONENT = {1.0, 2.0, 4.0};

vector<double> random_walk_on_sphere(double blur_scaling, double blur_exponent)
{
    auto g = bind(      normal_distribution<double>(), mt19937(0));
    auto u = bind(uniform_real_distribution<double>(), mt19937(0));

    const auto random_direction = [&]()
    {
        return normalized({ g(), g(), g() });
    };

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
            auto point = vec{(1.0 - d) * p0 + d * p1};

            const auto focus_depth = 1.0;
            const auto dz = focus_depth - point[2];
            point += blur_scaling * pow(dz, blur_exponent) * random_direction();

            const auto x = point[0] * RADIUS + IMAGE_WIDTH  * 0.5;
            const auto y = point[1] * RADIUS + IMAGE_HEIGHT * 0.5;

            if (0.0 <= x && x < IMAGE_WIDTH && 0.0 <= y and y < IMAGE_HEIGHT)
            {
                const auto xi = static_cast<size_t>(x);
                const auto yi = static_cast<size_t>(y);
                ++image[yi * IMAGE_WIDTH + xi];
            }
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

void write_image(const vector<double>& image, const char* filename)
{
    ofstream file(filename);
    file << "P3" << endl << IMAGE_WIDTH << " " << IMAGE_HEIGHT << endl << "255" << endl;
    for (const auto pixel : image)
    {
        auto NUM_COLOR_CHANNELS = 3;
        for (auto c = 0; c < NUM_COLOR_CHANNELS; ++c)
            file << min(int(pixel * BRIGHTNESS), 255) << " ";
    }
    file.close();
}

int main()
{
    for (const auto blur_exponent : BLUR_EXPONENT)
    {
        for (const auto blur_scaling : BLUR_SCALING)
        {
            cout << "Exponent: " << blur_exponent << ". Scaling: " << blur_scaling << endl;
            const auto image = random_walk_on_sphere(blur_scaling, blur_exponent);
            const auto file_name = "image_" + to_string(blur_scaling) + "_" + to_string(blur_exponent) + ".ppm";
            write_image(image, file_name.c_str());
        }
    }
    return 0;
}
