#include <iomanip>
#include <random>
#include <map>
#include <iostream>

int main(int argc, char *argv[])
{
    double const exp_dist_mean = 10;
    double const exp_dist_lambda = 1 / exp_dist_mean;
    srand(time(0));

    std::random_device rd;
    std::exponential_distribution<> rng(exp_dist_lambda);
    std::mt19937 rnd_gen(rd());

    // double total = 0;

    for (int i = 0; i < 10; i++)
        std::cout << rng(rnd_gen) << std::endl;

    // for (int i = 0; i < 1000000; ++i)
    //     ++result_set[rng(rnd_gen) * 4];

    // for (auto &v : result_set)
    // {
    //     std::cout << std::setprecision(2) << std::fixed;

    //     std::cout << v.first / 4.f << " - " << (v.first + 1) / 4.f << " -> ";
    //     std::cout << std::string(v.second / 4000, '.') << std::endl;

    //     if (v.second / 400 == 0)
    //         break;
    // }
}