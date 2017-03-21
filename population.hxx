#ifndef _POPULATION_
#define _POPULATION_

#include <iostream>

class Population
{
   public:
      std::vector<Player> agents;
      
      Population(size_t n) : agents(n) {}
      Population(Population& p) = default;
      Population(Population&& p) = default;
      
      void sort()
      {
         std::sort(std::begin(agents),std::end(agents),
            [](Player const& A, Player const& B)->bool {return B.fitness < A.fitness;});
      }
      
      void get_max(double& max_sco, double& max_div)
      {
         assert(agents.size()!=0);
         
         auto first = std::begin(agents); 
         auto last = std::end(agents);

         max_sco = first->score_ave;
         max_div = first->diversity_ave;
         
         while (++first!=last)
         {
            if (max_sco < first->score_ave)
               max_sco = first->score_ave;
            if (max_div < first->diversity_ave)
               max_div = first->diversity_ave;
         } 
      }
      
      void get_min(double& min_sco, double& min_div)
      {
         assert(agents.size()!=0);
         
         auto first = std::begin(agents); 
         auto last = std::end(agents);

         min_sco = first->score_ave;
         min_div = first->diversity_ave;
         
         while (++first!=last)
         {
            if (min_sco > first->score_ave)
               min_sco = first->score_ave;
            if (min_div > first->diversity_ave)
               min_div = first->diversity_ave;
         } 
      }      
      Player* get_random_agent()
      {
         if (size() < 2) return &*std::begin(agents);
         return &*(std::begin(agents)+Rand::randn(size()));
      }
      
      double fitness_mean() const
      {
         double sum = 0.0;
         for (auto a : agents)
            sum += a.fitness;
         return sum / double(size());
      }
      
      double fitness_sd(double mean) const
      {
         double sum = 0.0;
         for(auto a : agents)
            sum += (a.fitness - mean) * (a.fitness - mean);
         return std::sqrt(sum / double(size()));
      }
      
      double score_mean() const
      {
         double sum = 0.0;
         for (auto a : agents)
            sum += a.score_ave;
         return sum / double(size());
      }
      
      double score_sd(double mean) const
      {
         double sum = 0.0;
         for(auto a : agents)
            sum += (a.score_ave - mean) * (a.score_ave - mean);
         return std::sqrt(sum / double(size()));
      }
      
      double diversity_mean() const
      {
         double sum = 0.0;
         for (auto a : agents)
            sum += a.diversity_ave;
         return sum / double(size());
      }
         
      double diversity_sd(double mean) const
      {
         double sum = 0.0;
         for(auto a : agents)
            sum += (a.diversity_ave - mean) * (a.diversity_ave - mean);
         return std::sqrt(sum / double(size()));
      }
      
      double stat_mean(size_t i) const
      {
         double sum = 0.0;
         for (auto a : agents)
            sum += a.stats[i];
         return sum / double(size());
      }
      
      size_t size() const { return agents.size(); }
      
      friend std::ostream& operator<<(std::ostream& os, Population const& p);
};

#endif
