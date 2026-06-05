#pragma once

#include <array>
#include <numeric>
#include <random>
#include <cmath>
#include <algorithm>


class PerlinNoise {
private:
    std::array<int, 512> p{};

    double Fade(double t) const noexcept{
        return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
    }

    double Lerp(double t, double a, double b) const noexcept{
        return a + t * (b - a);
    }

    struct Vector2D {
        double x{};
        double y{};
    };

    double Grad(int hash, double x, double y) const noexcept {
    static constexpr Vector2D gradients[8] = {
        { 1.0,  1.0}, {-1.0,  1.0}, { 1.0, -1.0}, {-1.0, -1.0}, // Diagonals
        { 1.0,  0.0}, {-1.0,  0.0}, { 0.0,  1.0}, { 0.0, -1.0}  // Alternates (Axes)
    };

    const Vector2D& g = gradients[hash & 7];
    return { (g.x * x) + (g.y * y) };
    }

public:
    explicit PerlinNoise(unsigned int seed = std::random_device{}()) {
        Reseed(seed);
    }

    void Reseed(unsigned int seed) {
        std::iota(p.begin(), p.begin() + 256, 0);
        std::default_random_engine engine(seed);
        std::shuffle(p.begin(), p.begin() + 256, engine);

        for(int i = 0; i < 256; ++i) {
            p[256 + i] = p [i];
        }
    }

    double Noise2D(double x, double y) const noexcept {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;

        x -= std::floor(x);
        y -= std::floor(y);

        double u = Fade(x);
        double v = Fade(y);

        int aa = p[p[X] + Y];
        int ab = p[p[X] + Y + 1];

        int ba = p[p[X + 1] + Y];
        int bb = p[p[X + 1] + Y + 1];

        double x1 = Lerp(u, Grad(aa, x, y), Grad(ba, x - 1.0, y));
        double x2 = Lerp(u, Grad(ab, x, y -1.0), Grad(bb, x - 1.0, y - 1.0));

        return Lerp(v, x1, x2);    
    }

    double FractalNoise2D(double x, double y, int octaves, double lacunarity, double gain) const noexcept {
        double total = 0.0;
        double amplitude = 1.0;
        double frequency = 1.0;
        double maxValue = 0.0;

        for(int i = 0; i < octaves; ++i) {
            total += Noise2D(x * frequency, y * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= gain;
            frequency *= lacunarity;
        }

        return (total / maxValue + 1.0) / 2;
    }
};