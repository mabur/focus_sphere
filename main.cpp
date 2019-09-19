#include <algorithm>
#include <array>
#include <chrono>
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

const size_t NUM_COLOR_CHANNELS = 3;
const size_t IMAGE_WIDTH  = 750;
const size_t IMAGE_HEIGHT = 750;
const double IMAGE_WIDTH_D  = IMAGE_WIDTH;
const double IMAGE_HEIGHT_D = IMAGE_HEIGHT;
const vec CAMERA_CENTER = {IMAGE_WIDTH  * 0.5, IMAGE_HEIGHT * 0.5, 0.0};
const double RADIUS = min(IMAGE_HEIGHT, IMAGE_WIDTH) * 0.4;
const size_t NUM_STEPS = 8000;
const double STEP_SIZE = 0.09;
const size_t NUM_SAMPLES_PER_LINE = 100;
const double BRIGHTNESS = 1000.0;
const auto BLUR_SCALING = 0.04;
const auto NUM_FRAMES = 256;
const auto ABERRATION = array<double,3>{1-.020, 1-.010, 1};

void rotate(vec& v, double a)
{
    const auto c = cos(a);
    const auto s = sin(a);
    const auto x = +c * v[0] + s * v[2];
    const auto z = -s * v[0] + c * v[2];
    v[0] = x;
    v[2] = z;
}

auto sample_normal_focus = bind(normal_distribution<double>(), mt19937(0));

vector<vec> random_walk_on_sphere(double blur_scaling, double angle, double focus_depth)
{
    auto sample_normal_path = bind(normal_distribution<double>(), mt19937(0));
    auto sample_uniform = bind(uniform_real_distribution<double>(), mt19937(0));

    const auto random_direction_path = [&]()
    {
        return normalized({ sample_normal_path(), sample_normal_path(), sample_normal_path() });
    };
    const auto random_direction_focus = [&]()
    {
        return normalized({ sample_normal_focus(), sample_normal_focus(), sample_normal_focus() });
    };

    auto line_segments = vector<vec>{};
    vec point = random_direction_path();
    for (size_t i = 0; i < NUM_STEPS; ++i)
    {
        auto dir = random_direction_path();
        point = normalized(point + STEP_SIZE * dir);
        line_segments.push_back(point);
    }

    auto image = vector<vec>(IMAGE_WIDTH * IMAGE_HEIGHT, vec{0, 0, 0});
    for (size_t i = 0; i < line_segments.size() - 1; ++i)
    {
        const vec& p0 = line_segments[i + 0];
        const vec& p1 = line_segments[i + 1];
        for (size_t s = 0; s < NUM_SAMPLES_PER_LINE; ++s)
        {
            const auto d = sample_uniform();
            vec point_world = vec{(1.0 - d) * p0 + d * p1};
            rotate(point_world, angle);
            const double dz = focus_depth - point_world[2];
            point_world += blur_scaling * dz * random_direction_focus();

            for (size_t c = 0; c < NUM_COLOR_CHANNELS; ++c)
            {
                const vec point_aberrated = ABERRATION[c] * point_world;
                const vec point_camera = RADIUS * point_aberrated + CAMERA_CENTER;
                const double x = point_camera[0];
                const double y = point_camera[1];

                if (0.0 <= x && x < IMAGE_WIDTH_D && 0.0 <= y && y < IMAGE_HEIGHT_D)
                {
                    const size_t xi = static_cast<size_t>(x);
                    const size_t yi = static_cast<size_t>(y);
                    ++image[yi * IMAGE_WIDTH + xi][c];
                }
            }
        }
    }
    const auto compare_pixels = [](vec a, vec b) {return a[0] < b[0];};
    const vec maximum = *max_element(begin(image), end(image), compare_pixels);
    const double normalization_factor = 1.0 / maximum[0];
    for (vec& pixel : image)
    {
        pixel *= normalization_factor;
    }
    return image;
}

void write_image(const vector<vec>& image, const char* filename)
{
    ofstream file(filename);
    file << "P3" << endl << IMAGE_WIDTH << " " << IMAGE_HEIGHT << endl << "255" << endl;
    for (const vec& pixel : image)
    {
        for (size_t c = 0; c < NUM_COLOR_CHANNELS; ++c)
        {
            file << min(int(pixel[c] * BRIGHTNESS), 255) << " ";
        }
    }
    file.close();
}

int main()
{
    for (size_t t = 0; t < NUM_FRAMES; ++t)
    {
        const auto angle = double(t) / NUM_FRAMES * 2.0 * 3.14159265;
        const auto focus_depth = 1.0;
        const auto start = chrono::system_clock::now();
        cout << "Frame: " << t;
        const auto image = random_walk_on_sphere(BLUR_SCALING, angle, focus_depth);
        const auto file_name = "image_" + to_string(t) + ".ppm";
        write_image(image, file_name.c_str());
        const auto end = chrono::system_clock::now();
        const auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        cout << ". Took: "  << duration << endl;
    }
    return 0;
}
