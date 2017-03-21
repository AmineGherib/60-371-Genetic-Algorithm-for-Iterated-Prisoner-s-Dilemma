#ifndef _RAND_
#define _RAND_

#include <bitset>
#include <random>
#include <algorithm>

class Rand {
    public:
        static unsigned long rand(float a, float b) { 
            static std::random_device rd;
            static std::mt19937 generator(rd());
            static std::uniform_int_distribution<> distribution(a, b);
            return distribution(generator);
        }

        static unsigned long randBit() { 
            return rand(0,1); 
        }

        static unsigned long randBinN(unsigned int n, double p) {
            static std::random_device rd; 
            static std::mt19937 generator(rd());
            std::binomial_distribution<> distribution(n, p);
            return distribution(generator);
        }

        static unsigned long randn(unsigned int n) {
            return rand(0,n-1);
        }
        
        static std::bitset<64> random_64_bits() {
            std::bitset<64> bits = 0;
            for (size_t i = 0; i < 64; ++i)
                bits[i] = randBit();
            auto retVal = std::move(bits);
            return retVal;
        }

        static std::bitset<6> random_6_bits() {
            std::bitset<6> bits = 0;
            for (size_t i = 0; i < 6; ++i)
                bits[i] = randBit();
            auto retVal = std::move(bits);
            return retVal;
        }
};
#endif
