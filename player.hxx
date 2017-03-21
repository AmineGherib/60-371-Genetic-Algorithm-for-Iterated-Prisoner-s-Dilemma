#ifndef _PLAYER_
#define _PLAYER_

#include <bitset>
#include <iostream>
#include <algorithm>

static constexpr size_t combation_pairs(size_t n) { return n * (n-1) / 2; }

using Statistics = std::array<double, 6>;

struct Player
{
    enum Decision { Cooperate=0, Defect=1 };

    struct PastGenes
    {
        PastGenes() : memory(Rand::random_6_bits()) {};
        PastGenes(std::string s) : memory(s) {};
        PastGenes(unsigned long l) : memory(l) {};

        void save(Decision A, Decision B) {
            memory <<= 1;
            memory |= A;
            memory <<= 1;
            memory |= B;
        }
        unsigned long get() const {
            return memory.to_ulong();
        }

        static PastGenes invert(PastGenes const& H)
        {
            static std::bitset<6> even("101010");
            static std::bitset<6> odd("010101");

            auto memoryOfEven = (H.memory & even) >>1;
            auto memoryOfOdd = (H.memory & odd) << 1 ;
            return PastGenes((memoryOfEven | memoryOfOdd).to_ulong());
        }

        static bool paranoid(PastGenes const& H)
        {
            static std::bitset<6> opp("010101");
            return (H.memory & opp) == opp;
        }

        static bool uneasy(PastGenes const& H)
        {
            static std::bitset<6> opp("010101"), zero(0);
            return (H.memory & opp) != zero;
        }

        static bool burned(PastGenes const& H)
        {
            return (   ((H.memory[0]==1) && (H.memory[1]==0))
                    ||((H.memory[2]==1) && (H.memory[3]==0))
                    ||((H.memory[4]==1) && (H.memory[5]==0)) );
        }

        static bool bothBurned(PastGenes const &H)
        {
            int i = 0, j = 0;
            if ((H.memory[0]==1) && (H.memory[1]==0)) ++i;
            if ((H.memory[2]==1) && (H.memory[3]==0)) ++i;
            if ((H.memory[4]==1) && (H.memory[5]==0)) ++i;
            if ((H.memory[0]==0) && (H.memory[1]==1)) ++j;
            if ((H.memory[2]==0) && (H.memory[3]==1)) ++j;
            if ((H.memory[4]==0) && (H.memory[5]==1)) ++j;
            return i == j;
        }

        friend std::ostream& operator<<(std::ostream& os, Player::PastGenes const& h);
        private:
        std::bitset<6> memory;
    };
    struct Genome
    {
        Genome() : genome(Rand::random_64_bits()) {}
        Genome(std::string s) : genome(s) {};

        static Genome crossover(Genome A, Genome B)
        {
            static const Genome mask("1111111111111111111111111111111111111111111111111111111111111111");
            unsigned long cpoint = Rand::rand(1,63);
            Genome next;
            next.genome = (A.genome & (mask.genome << cpoint)) | (B.genome & (mask.genome >> (64 - cpoint)));
            auto retVal = std::move(next);
            return retVal;
        } 

        static unsigned long countUniqueBits(Genome const& a, Genome const& b)
        {
            std::bitset<64> s(a.genome ^ b.genome);
            unsigned long sum = 0;
            for (int i = 0; i < 64; ++i)
                sum += s[i];
            return sum;
        }

        Decision decide(PastGenes const& pg) const
        {
            return (genome[pg.get()] ? Decision::Defect : Decision::Cooperate);
        }

        void mutate()
        {
            unsigned long mpoint = Rand::rand(0,63);
            genome[mpoint].flip();
        }

        int forgiveness(PastGenes const & h) const
        {
            if ( Player::PastGenes::uneasy(h) 
                    && !Player::PastGenes::burned(h) 
                    && (decide(h) == Decision::Cooperate) )
                return 1;
            if ( Player::PastGenes::uneasy(h) 
                    && !Player::PastGenes::burned(h) 
                    && (decide(h) == Decision::Defect) )  
                return -1;
            return 0;
        }

        double calc_forgiveness()
        {
            double sum = 0.0;
            for (unsigned long i = 0; i < 64; ++i)
                sum += forgiveness(PastGenes(i));  
            assert((sum >= -19) && (sum <= 19)); 
            return (sum + 19)/(19*2);
        }

        int niceness(PastGenes const & h) const
        {
            if ( !Player::PastGenes::uneasy(h)  
                    && (decide(h) == Decision::Cooperate) )
                return 1;
            if ( !Player::PastGenes::uneasy(h) 
                    && (decide(h) == Decision::Defect) )  
                return -1;
            return 0;
        }

        double calc_niceness()
        {
            double sum = 0.0;
            for (unsigned long i = 0; i < 64; ++i)
                sum += niceness(PastGenes(i));   
            assert((sum >= -8) && (sum <= 8));
            return (sum + 8)/(8*2);
        }

        int retaliation(PastGenes const & h) const
        {
            if ( ( Player::PastGenes::burned(h) 
                        || Player::PastGenes::paranoid(h) )
                    && (decide(h) == Decision::Defect)    )
                return 1;
            if ( ( Player::PastGenes::burned(h) 
                        || Player::PastGenes::paranoid(h) )
                    && (decide(h) == Decision::Cooperate) ) 
                return -1;
            return 0;
        }

        double calc_retaliation()
        {
            double sum = 0.0;
            for (unsigned long i = 0; i < 64; ++i)
                sum += retaliation(PastGenes(i)); 
            assert((sum >= -38) && (sum <= 38));
            return (sum + 38)/(38*2);
        }

        int non_envy(PastGenes const & h) const
        {
            if ( Player::PastGenes::bothBurned(h) 
                    && (decide(h) == Decision::Cooperate) )
                return 1;
            if ( Player::PastGenes::bothBurned(h) 
                    && (decide(h) == Decision::Defect) )  
                return -1;
            return 0;
        }

        double calc_nonenvy()
        {
            double sum = 0.0;
            for (unsigned long i = 0; i < 64; ++i)
                sum += non_envy(PastGenes(i)); 
            assert((sum >= -20) && (sum <= 20));
            return (sum + 20)/(20*2);
        }

        friend std::ostream& operator<<(std::ostream& os, Genome const& s);
        private:
        std::bitset<64> genome;
    };
    Player() : genome(), fitness(7.75), score_ave(7.75), diversity_ave(32) {}
    Player(Genome _genome) : genome(_genome), fitness(7.75), score_ave(7.75), diversity_ave(32) {}

    Genome genome;
    double fitness;
    double score_ave;
    double diversity_ave;
    Statistics stats;

    void calc_fitness(Statistics const& weights, double const& max_score, 
            double const& max_diversity, double const& min_score, double const& min_diversity)
    {
        assert(weights.size() == stats.size());

        if ((int(max_score*100000) != 0) && (max_score != min_score))
            stats[0] = 1.0 - ((score_ave - min_score) / (max_score - min_score)); 
        else
            stats[0] = 1.0;
        if ((int(max_diversity*100000) != 0) && (min_diversity != max_diversity))
            stats[1] = (diversity_ave - min_diversity) / (max_diversity - min_diversity);
        else
            stats[1] = 1.0;
        stats[2] = genome.calc_niceness();
        stats[3] = genome.calc_forgiveness();
        stats[4] = genome.calc_retaliation();
        stats[5] = genome.calc_nonenvy();

        fitness = 0.0;

        for (size_t i = 0; i < weights.size(); ++i)
            fitness += stats[i]*weights[i];
    }

    void calc_aves(double const pop_size, double const num_games)
    {
        score_ave = double(score_sum) / (combation_pairs(pop_size) * num_games);
        diversity_ave = double(diversity_sum) / combation_pairs(pop_size);
    }

    void clear_sums()
    {
        score_sum = 0;
        diversity_sum = 0;
    }

    static void diversity(Player & a, Player& b)
    {
        unsigned long sum = Genome::countUniqueBits(a.genome, b.genome);
        a.diversity_sum += sum;
        b.diversity_sum += sum;
    }

    static unsigned long score(Decision A, Decision B)
    {
        if (A == Decision::Cooperate && B == Decision::Cooperate)
            return 1;
        if (A == Decision::Cooperate && B == Decision::Defect)
            return 20;
        if (A == Decision::Defect && B == Decision::Cooperate)
            return 0;
        return 10;
    }

    enum Result_t
    {
        allDefect, allCooperate, mixed
    };         

    static Result_t compete(Player & A, Player & B, PastGenes & H)
    {
        Decision a = A.genome.decide(H);
        Decision b = B.genome.decide(H);
        A.score_sum += score(a,b);
        B.score_sum += score(b,a);
        H.save(a,b);


        return ((a==Cooperate && b==Cooperate) ? allCooperate : 
                ( (a==Defect && b==Defect) ? allDefect : mixed)
               );
    }

    friend std::ostream& operator<<(std::ostream& os, Player const& p);
    private:
    unsigned long score_sum;
    unsigned long diversity_sum;
};

#endif
