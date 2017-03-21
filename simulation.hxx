#ifndef _SIMULATION_
#define _SIMULATION_

#include "rand.hxx"
#include "population.hxx"
#include "player.hxx"

bool fileExists(const std::string& file) {
    struct stat buf;
    return (stat(file.c_str(), &buf) == 0);
}

struct Simulation {
      struct Parameters {
         unsigned int pop_size;
         size_t num_genetic_iter;
         size_t num_compete_iter;
         double selection_rate;
         double mutation_rate;
         Statistics weights;
         
         friend std::ostream& operator<<(std::ostream& os, Simulation::Parameters const& p);
      };

      Simulation(Parameters _params) : params(_params) {
         make_csv("demographics", demographics_csv);
         print_demographics_headings();
         make_csv("final_population", final_population_file);
         print_finalpop_headings();
         make_csv("all_population", all_population_file);
      }
      
      void make_csv(std::string s, std::ofstream& os) {
         static unsigned long i = 0;
         while (fileExists(s + std::to_string(i) + ".csv"))
            ++i;
         os.open(s + std::to_string(i) + ".csv",
            std::ofstream::out | std::ofstream::trunc);
         os << params;
      }
      
      ~Simulation() {
         demographics_csv.close();
         final_population_file.close();
         all_population_file.close();
      }
      
      void print_demographics_headings() {
         demographics_csv
            << std::endl
            << "Generation, Selected, "
            << "Fitness Mean, Fitness SD, "
            << "Score Mean, Score SD, "
            << "Diversity Mean, Diversity SD, "
            << "Relative Score Mean, Relative Diversity Mean, "
            << "Niceness Mean, Forgiveness Mean, "
            << "Retaliation Mean, Non-Envy Mean, "
            << "All-Coop Prop, All-Defect Prop, Mixed Prop"
            << std::endl
            ;
      }
      
      void log_demographics(size_t i, Population const& p) {
         //iterations number
         demographics_csv
            << i << ", " << num_selected << ", ";
            
         double tmp; 
         
         tmp = p.fitness_mean();
         demographics_csv
            << tmp << ", " << p.fitness_sd(tmp) << ", ";
            
         tmp = p.score_mean();
         demographics_csv
            << tmp << ", " << p.score_sd(tmp) << ", ";
            
         tmp = p.diversity_mean();
         demographics_csv
            << tmp << ", " << p.diversity_sd(tmp);
            
         for (size_t a = 0; a < params.weights.size(); ++a)
            demographics_csv << ", " << p.stat_mean(a);
         
         demographics_csv << ", "
            << coop_prop << ", "
            << defect_prop << ", "
            << mixed_prop << std::endl;
      }
      
      void print_finalpop_headings() {
         final_population_file
            << std::endl
            << "Rank, Strategy, Fitness, Average Score, Average Diversity, "
            << "Relative Average Score, Relative Average Diversity, "
            << "Friendliness, Anti-revenge, Revenge, Non-Envy"
            << std::endl;
      }
      
      void log_finalpop(Population const& p) {
         size_t rank = 0;
         for (auto const& a : p.agents)
         {
            final_population_file
               << ++rank << ", "
               << a.genome << ", "
               << a.fitness << ", "
               << a.score_ave << ", "
               << a.diversity_ave;
            for (auto const& b : a.stats)
               final_population_file << ", " << b;
            final_population_file << std::endl;
         }
      }
      
      void evaluate(Population &p)
      {
         for (auto& a : p.agents)
            a.clear_sums();
         
         clear_choice_tally();
         
         for(auto A = std::begin(p.agents); A != std::end(p.agents); ++A) {
            for(auto B = next(A); B != std::end(p.agents); ++B) {
               Player::PastGenes pm;
               for (size_t i = 0; i < params.num_compete_iter; ++i)
                  tally_choice(Player::compete(*A,*B,pm));
               Player::diversity(*A,*B);
            }
         }
         
         calc_choice_prop();
         
         for (auto& a : p.agents)
            a.calc_aves(params.pop_size, params.num_compete_iter);
         
         double max_sco, max_div, min_sco, min_div;
         p.get_max(max_sco, max_div);
         p.get_min(min_sco, min_div);
         
         for (auto& a : p.agents)
            a.calc_fitness(params.weights, max_sco, max_div, min_sco, min_div);
         
         p.sort();
      }
      
      void clear_choice_tally()
      {
         coop_sum = 0;
         defect_sum = 0;
         mixed_sum = 0;
      }
      
      void tally_choice(Player::Result_t r)
      {
         switch (r) 
         {
            case Player::Result_t::allCooperate: ++coop_sum; break;
            case Player::Result_t::allDefect: ++defect_sum; break;
            case Player::Result_t::mixed: ++mixed_sum; break;
         }
      }
      
      void calc_choice_prop()
      {
         double total = coop_sum + defect_sum + mixed_sum;
         coop_prop = coop_sum / total;
         defect_prop = defect_sum / total;
         mixed_prop = mixed_sum / total;
      }
      
      Population select_best(Population const& a)
      {
         Population b(0);
         
         for (size_t i = 0; i < num_selected; ++i)
            b.agents.emplace_back(a.agents[i].genome);
         
         auto retVal = std::move(b);
         return retVal;
      }
      
      Population recombine(Population const& p)
      {
         Population parents(select_best(p));
         Population children(0);
         for(auto A = std::begin(parents.agents); A != std::end(parents.agents); ++A) {
            for(auto B = next(A); B != std::end(parents.agents); ++B) {
               if (children.size() < params.pop_size)
                  children.agents.emplace_back(
                     Player::Genome::crossover(A->genome,B->genome));
               else  {
                   auto retVal = std::move(children);
                   return retVal;
               }
            }
         }
         auto retVal = std::move(children);
         return retVal;
      }
      
      void mutate(Population& p)
      {
         for (size_t i = 0; i < floor(p.size()*params.mutation_rate); ++i)
            p.get_random_agent()->genome.mutate();
      }
      
      void replace_worst(Population &dest, Population& source)
      {
         Population next(select_best(source));
         
         assert(dest.size() >= next.size());
         
         assert(num_selected == next.size());
         for (size_t i=0; i < num_selected; ++i)
            dest.agents.pop_back();
         
         for (auto& best : next.agents)
            dest.agents.emplace_back(best);
      }
      
      void genetic()
      {
         Population initial(params.pop_size);
            
         for (size_t i=0; i < params.num_genetic_iter; ++i)
         {
            std::cout << "                              " << std::endl;
            std::cout << "         ITERATION # " << i << "            " << std::endl;
            
            evaluate(initial);
         
            num_selected = Rand::randBinN(initial.size(),params.selection_rate);
         
            log_demographics(i, initial);
            
            std::cout << "       Initial Population      " << std::endl;
            std::cout << initial;
            all_population_file << initial << std::endl;
            
            Population next = recombine(initial);
            mutate(next);
            evaluate(next);
            
            num_selected = Rand::randBinN(next.size(),params.selection_rate);
            
            std::cout << "********** Next Population **********" << std::endl;
            std::cout << next;
            all_population_file << next << std::endl;
            
            replace_worst(initial, next);
         }
         
         std::cout << std::endl << std::endl;
         
         //log the final population to a file
         log_finalpop(initial);
      }
      
   private:
      std::ofstream demographics_csv, final_population_file, all_population_file;
      unsigned long defect_sum, coop_sum, mixed_sum;
      size_t num_selected;
      double defect_prop, coop_prop, mixed_prop;
      const Parameters params;
};
#endif
