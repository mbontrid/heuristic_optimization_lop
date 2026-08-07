#import "template.typ": *
#show: doc => styling(doc)

#title_page
#pagebreak()

#outline(title: text(size: 1.5em, font: "New Computer Modern")[Contents], indent: auto)

#pagebreak()

= Abstract

This report presents the implementation and analysis of local-search, local-search with perturbation and population search methods for the Linear Ordering Problem (LOP). This report analyse the implementation of iterative improvement with first- and best-improvement pivot rules on three neighborhoods (transpose, exchange, insert), each combined with two initialization methods (random and Chenery-Watanabe, CW). There is 12 combination of algorithm variants. Variable Neighborhood Descent (VND) with two neighborhood orders is also studied.

Across the 78 benchmark instances, exchange and insert neighborhoods clearly dominate transpose in solution quality. The best average deviation among the 12 iterative-improvement variants is obtained by random-first-exchange (67.141% average deviation). For VND, the order transpose-exchange-insert performs better than transpose-insert-exchange (79.417% vs 80.617% average deviation). Statistical tests (paired Student t-test and Wilcoxon signed-rank test) show a great differences for 65 out of 66 iterative-improvement comparisons.

= Introduction

The Linear Ordering Problem is a NP-hard combinatorial optimization problem. Given a matrix of weights, the goal is to find a row/column sequence to maximize the sum of the upper-right triangle of the permuted matrix.
Because exact methods are expensive on large instances, local-search heuristics are commonly used.

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

=== Iterative improvement lop

For iterative improvement:
- pivot rules: first-improvement and best-improvement,
- neighborhoods: transpose, exchange, insert,
- initial solutions: random and Chenery-Watanabe (CW).



=== VND

Two VND variants were implemented with first-improvement local search and CW initialization:
- transpose -> exchange -> insert
- transpose -> insert -> exchange

=== Iterated local search

=== Memetic algorithm

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

Two VND variants were implemented with first-improvement local search and CW initialization:
- transpose -> exchange -> insert
- transpose -> insert -> exchange

=== Iterated local search

=== Memetic algorithm

Memetic algorithm is composed of multiple methods and each possible methods has a non negligible spectre of parameters. As such each




==== Initialization

Initialization uses the Chenery-Watanabe initialization only. This mean that CW is applied once to the LOP instance given to the memetic algorithm.


==== VND



==== Crossover

- DPX crossover
- OB crossover

After confirming that OB crossover is the best method as mentioned in @schiavinotto_linear_2004, DPX was not used at all in the analysis.

The memetic algorithm is has theses parameters:

- population size
- Offspring size
- cross and mutation distribution in offspring
- cross rate
- mutation rate
- mean try
- diversity try
- pivot rule
- local search vector (VND)






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

Spedups were implemented to accelerate the computation. For some instances he first version took a day to compute. After multiple speedup (mainly the sum of cost), the total benchmark take five minutes.

=== sum of cost
The order transpose -> exchange -> insert is statistically better in quality, while transpose -> insert -> exchange is faster. This is a clear quality-time trade-off.

#figure(
  image("../../../data/output/figs/vnd_summary.svg", width: 100%),
  caption: [VND summary and per-instance distribution comparison.],
)

= Conclusion

- exchange and insert are superior to transpose,
- neighborhood choice is the dominant for solution quality,
- random initialization provides better quality than CW on this benchmark,
- for VND, transpose -> exchange -> insert improves quality significantly but costs more runtime.

To conclude, exchange/insert-based methods give the best quality and for VND, transpose → exchange → insert has the best quality.
