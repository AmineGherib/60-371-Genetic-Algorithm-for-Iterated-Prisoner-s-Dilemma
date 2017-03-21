#include <cassert>
#include <vector>
#include <array>
#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include "rand.hxx"
#include "player.hxx"
#include "population.hxx"
#include "simulation.hxx"

using std::min;
using std::max;

std::ostream& operator<<(std::ostream& os, Player::PastGenes const& h)
{
   os << h.memory;
   return os;
}

std::ostream& operator<<(std::ostream& os, Player::Decision const& c)
{
   os << ((c == Player::Decision::Cooperate) ? "cooperate" : "defect");
   return os;
}

std::ostream& operator<<(std::ostream& os, Player::Genome const& s)
{
   os << s.genome;
   return os;
}

std::ostream& operator<<(std::ostream& os, Player const& p)
{
   os << p.genome << ", " << p.fitness;
   
   return os;
}

std::ostream& operator<<(std::ostream& os, Population const& p)
{
   for (auto const& a : p.agents)
      os << a << std::endl;
   return os;
}

std::ostream& operator<<(std::ostream& os, Simulation::Parameters const& p)
{
   os << "Population Size, " << p.pop_size << std::endl
      << "GA Iterations, " << p.num_genetic_iter << std::endl
      << "Rounds per game, " << p.num_compete_iter << std::endl
      << "Selection Rate, " << p.selection_rate << std::endl
      << "Mutation Rate, " << p.mutation_rate << std::endl
      << "Fitness Weights";
   for (auto const& w : p.weights)
      os << ", " << w;
   os << std::endl;
   return os;
}

std::istream& operator>>(std::istream & is, Simulation::Parameters& p)
{
   Simulation::Parameters tmp;
   is >> tmp.pop_size
      >> tmp.num_genetic_iter
      >> tmp.num_compete_iter
      >> tmp.selection_rate
      >> tmp.mutation_rate;
      
   for (size_t i = 0; i < tmp.weights.size(); ++i)
      is >> tmp.weights[i];
   
   if (is) {
      assert(tmp.pop_size > 2);
      assert(tmp.num_genetic_iter >= 1);
      assert(tmp.num_compete_iter >= 1);
      assert((tmp.selection_rate > 0.0) && (tmp.selection_rate <= 1.0));
      assert((tmp.mutation_rate >= 0.0) && (tmp.mutation_rate <= 1.0));
      
      p = std::move(tmp);
   }
   return is;
}

int main()
{
   Simulation::Parameters params;
   while (std::cin >> params)
   {
      Simulation sim(params);
      sim.genetic();
   }
}
