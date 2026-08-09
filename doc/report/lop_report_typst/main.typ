#import "template.typ": *
#show: doc => styling(doc)

#title_page
#pagebreak()

#outline(title: text(size: 1.5em, font: "New Computer Modern")[Contents], indent: auto)

#pagebreak()

= Abstract

This report presents the implementation and analysis of four methods of resolution for the linear ordering problem (LOP). Two In the local-search family: iterative improvement and variable neighborhood descend, and two stochastic local search : Iterated local search (ILS) and a population based memetic algorithm.

Across the 78 benchmark instances, exchange and insert neighborhoods clearly dominate transpose in solution quality. The best average deviation among the 12 iterative-improvement variants is obtained by random-first-exchange (67.141% average deviation). For VND, the order transpose-exchange-insert performs better than transpose-insert-exchange (79.417% vs 80.617% average deviation). Statistical tests (paired Student t-test and Wilcoxon signed-rank test) show a great differences for 65 out of 66 iterative-improvement comparisons.

= Introduction

The Linear Ordering Problem is a NP-hard combinatorial optimization problem. Given a matrix of weights, the goal is to find a row/column sequence to maximize the sum of the upper-right triangle of the permuted matrix.
Because exact methods are expensive on large instances, local-search heuristics are commonly used.
implementation of iterative improvement with first- and best-improvement pivot rules on three neighborhoods (transpose, exchange, insert), each combined with two initialization methods (random and Chenery-Watanabe, CW). There is 12 combination of algorithm variants. Variable Neighborhood Descent (VND) with two neighborhood orders is also studied.
= Problem description

To find a maximum linear ordering, we seek a permutation $\pi$ of $n$ items maximized with the following algorithm category:
- iterative improvement with two pivoting rules (first, best),
- three neighborhoods (transpose, exchange, insert),
- two initialization methods (random, CW),

This yields 12 iterative-improvement algorithmic combinations:
$2$ initializations $times$ $2$ pivots $times$ $3$ neighborhoods.

VND implement first-improvement only and with two neighborhood orders:
- transpose -> exchange -> insert
- transpose -> insert -> exchange

The LOP objective can be written as:
$ max sum_(1 <= i < j <= n) w_(pi_i, pi_j) $

= Material and methods

== Benchmark instances and protocol

The benchmark set contains 78 LOP instances (sizes 150 and 250), each with a best-known objective value. For each algorithm-instance pair, we record solution quality (objective value) and runtime (seconds).

The deviation metric reported in this work is:
$"relative percentage deviation"(%) = ("best_known" - "found") / "best_known" * 100.$

Lower values indicate better performance (0% would match the best-known solution).

== Implemented algorithms

Iterative improvement and variable neighborhood descent (VND) are local search algorithms. Iterated local search and the population-based memetic algorithm are stochastic local search algorithms. Both uses VND.

=== local search

Local search are deterministic and prone to reaching local optima.

=== Iterative improvement lop

For iterative improvement, the following methods were implemented:
- pivot rules: first-improvement and best-improvement,
- neighborhoods: transpose, exchange, insert,
- initial solutions: random and Chenery-Watanabe (CW).

==== VND

Two VND variants were implemented with first-improvement local search and CW initialization:
- transpose -> exchange -> insert
- transpose -> insert -> exchange

VND is exactly the same implementation as iterative improvement. The neighborhoods search is not just one method but a vector of methods to apply in order until there is an improvement. In which case the first method of the function vector is applied again.

=== Stochastic local search

As the search space of lop is far too large to be explored exhaustively, stochastic local search proved to be quite successful in finding good local optima @schiavinotto_linear_2004.

Each heuristic has the following parameters corresponding to the VND used at each search of local optima.

parameters:

- pivot rule
- vector of neighborhods search method : ["transpose", "exchange", "insert"] was used for all benchmark.

==== Iterated local search

Simple stochastic algorithm. Search for the local optima with VND then apply a perturbation on the found solution. If the VND applied on the new solution is not accepted, a new perturbation on the previous solution is applied until `improvement try` is reached.

parameters:

- perturbation rate: Swap are applied on random index of a solutions. If the rate is 1, the new solution is random.
- improvement try: Number of new solution with non improving cost before returning the best result found.
- worse acceptance: Worsening cost to accept as a new solution.

==== Memetic algorithm

Memetic algorithm is composed of multiple methods and each possible methods has a non negligible spectre of parameters.

The memetic algorithm parameters are:

- population size
- Offspring size
- cross and mutation distribution in offspring
- cross rate : rate of the elements of the first parent to keep.
- mutation rate : rate of swap to apply on a solution.
- mean try : Number of try without improvement in the mean cost across the population before doing a diversification.
- diversity try : Number of diversification before returning the best result found.
- pivot rule
- local search vector (VND)

The methods applied in order are:
1. populate
2. offspring
4. select n best
5. diversification

===== populate

Generate a new set of offspring solution. Each solution is unique.

===== offspring

Generate new solutions based on the current population. Distribution between crossover offspring and mutation offspring corresponds to the cross and mutation distribution in offspring.

Crossover:

This crossover implementation also some mutation implemented. Generating crossover offspring was non-efficient as the same offspring or close enough offspring give the same local optima with the VND descend. As each offspring has to be unique, the new offspring was discarded if already existing and a new one had to be generated. Sometime, this could lead to long crossover runtime if the same solution was generated multiple time. To palliate this, a rate of mutation increase at each same solution generation to deviate from the similar solution.

The following crossover methods were implemented:
- DPX crossover
- OB crossover

After confirming with some experimentation that OB crossover is the best method as mentioned in @schiavinotto_linear_2004, only OB was used for the rest of the analysis to reduce the parameter space.


New mutated offspring are then generated based of the current population.

===== select n best

The n best individuals are selected from the population and offspring. A new mean is calculated for the diversification step.

===== Diversification

If the mean cost don't improve for a fixed number of try, the best individual is kept and all the population is replaced by unique randomly generated solution.
This step proved to be sufficient in perturbation. This mean that the mutation step in the crossover is not mandatory.


Here is an example of the stdout output showing a diversification try.

```bash
verbose: memetic: gen=162 | mean_pop_cost=3466069.000000 | best_mean_pop_cost=3466069.000000 | best_cost=3466069 | mean_try=9/10 | diversity_try=0/3
verbose: memetic: Generating offspring
verbose: memetic: Selecting 20 best
verbose: memetic: gen=163 | mean_pop_cost=3466069.000000 | best_mean_pop_cost=3466069.000000 | best_cost=3466069 | mean_try=10/10 | diversity_try=0/3
verbose: memetic: Generating offspring
verbose: memetic: Selecting 20 best
verbose: memetic: Diversifying population
verbose: memetic: gen=164 | mean_pop_cost=3466069.000000 | best_mean_pop_cost=3466069.000000 | best_cost=3466069 | mean_try=0/10 | diversity_try=1/3
verbose: memetic: Generating offspring
verbose: memetic: Selecting 20 best
verbose: memetic: gen=165 | mean_pop_cost=3406517.750000 | best_mean_pop_cost=3466069.000000 | best_cost=3466069 | mean_try=1/10 | diversity_try=1/3
```


== Speedup implementation

Speedups were implemented to accelerate the computation. For some instances the first version took a day to compute. After multiple speedup (mainly the sum of cost), the same instances take five minutes. A non exhaustive list of speedups is described below.

=== sum of cost

All algorithm implemented use the principle of delta-cost. When a candidate is modified, it is returned with de differences in cost. As such, a sum of all the matrix is computed only once and each operationn (optimization, randomization, ...) return the difference in cost (delta-cost) to add to the previously calculated cost. This allows to consider each modification as a swap and reduce complexity of the cost computation. This is the most important speedup implemented.

=== Chenery-Watanabe initialization

By computing the prefix sums of the matrix the cost calculation can avoid redundant computation. The perfix sum being each row computed like so :
$c_j = sum_(j=0)^n$
This speedup is not significant as the Chenery-Watanabe is a initialization method and is called only once in the runtime.

=== insert

The first implementation calculated every possible insert on an index and kept them in memory. This was not efficient as the memory access was slower than calculating few opperation with the cost-delta method.


= results

All results are located in the jupyter notebook.

The order transpose -> exchange -> insert is statistically better in quality, while transpose -> insert -> exchange is faster. This is a clear quality-time trade-off.


= Conclusion

- exchange and insert are superior to transpose,
- neighborhood choice is the dominant for solution quality,
- random initialization provides better quality than CW on this benchmark,
- for VND, transpose -> exchange -> insert improves quality significantly but costs more runtime.

To conclude, exchange/insert-based methods give the best quality and for VND, transpose → exchange → insert has the best quality.

= Annexe

== Tools
Generative AI was used in the pythons scripts as syntax help for generating graphs with matplotlib. The source code and report are fully exempt of such tools.

#bibliography("heuristique.bib")
