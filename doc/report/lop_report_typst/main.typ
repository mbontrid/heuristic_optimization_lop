#import "template.typ": *
#show: doc => styling(doc)

#title_page
#pagebreak()

#outline(title: text(size: 1.5em, font: "New Computer Modern")[Contents], indent: auto)

#pagebreak()

= Foreword

This report is based on the report of the previous implementation task. This allow to presents VND on which the memetic and iterated local search are using.

During the last month and a half, I had 6 concurrent course projects. Morover, I accumulated 110 hours of writing code on this project but didn't manage to finish it due to obvious time constraint. As I have profs of the cumulated times spend on this. I'll will ask by email for a chance to rentabilize this work with a time extension. Best regards.

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

== Speedup implementation

Speddups were implemented to accelerate the computation. The first version took a day to compute. After optimization, the total benchmark take five minutes.

=== inserts

The first implementation was testing all combinations on neighborhoods.
speedup:
The insert neighborhoods is based on multiple swap operations and a previous insert. The cost of a insert depends on the cost of the previous inserts. An efficient implementation of this is a constructive algorithms computing the delta cost of each swap until there all combinations of insert have been tested.

== CW initialization

By calculating the prefix sum, there is no need to sum j = i+1 element but two per row.

== sum of cost

All the algorithms in this implementation use a delta-cost principee where only the optimization in returned to avoid to compute the total cost at each steps.

== Statistical analysis

For each algorithm, was reported:
- average percentage deviation,
- standard deviation of percentage deviation,
- total runtime across all instances.

To compare algorithms,paired Student t-tests and Wilcoxon was used over per-instance relative percentage deviation values.

= Results

== Iterative improvement: descriptive statistics

#table(
  columns: (3.2fr, 1fr, 1fr, 1fr),
  table.header([Algorithm], [Avg. relative percentage deviation (%)], [Std. dev.], [Total time (s)]),
  [random-first-exchange], [67.141], [3.909], [1085.167],
  [random-best-insert], [67.836], [3.869], [322.952],
  [random-best-exchange], [67.942], [3.976], [172.946],
  [random-first-insert], [68.396], [3.958], [331.788],
  [CW-first-exchange], [79.811], [2.230], [954.984],
  [CW-best-insert], [80.191], [2.206], [303.321],
  [CW-best-exchange], [80.693], [2.212], [163.582],
  [CW-first-insert], [81.113], [2.216], [347.190],
  [random-best-transpose], [98.452], [0.698], [0.037],
  [random-first-transpose], [98.565], [0.844], [0.015],
  [CW-best-transpose], [98.714], [0.506], [0.038],
  [CW-first-transpose], [98.806], [0.523], [0.015],
)

The best quality is obtained by random-first-exchange. Exchange and insert neighborhoods systematically outperform transpose in quality. Transpose is by far the fastest neighborhood and the least accurate.

#figure(
  image("../../../data/output/figs/it_imp_summary.svg", width: 100%),
  caption: [Iterative improvement summary (average relative percentage deviation and total runtime).],
)

#figure(
  image("../../../data/output/figs/it_imp_boxplot.svg", width: 100%),
  caption: [Iterative improvement boxplot of per-instance relative percentage deviation distributions.],
)

== Iterative improvement: statistical tests

Pairwise statistical testing among the 12 iterative-improvement variants yields 66 comparisons.
- Wilcoxon significant at 5%: 65 / 66 comparisons.
- Only non-significant comparison:
  CW-best-transpose vs random-first-transpose
  (t-test p = 0.154579, Wilcoxon p = 0.652164).

This indicates that most algorithmic choices lead to statistically distinguishable solution quality, except between the two very weak transpose combinations.

== VND results and statistical test

#table(
  columns: (3.2fr, 1fr, 1fr, 1fr),
  table.header([VND neighborhood order], [Avg. relative percentage deviation (%)], [Std. dev.], [Total time (s)]),
  [transpose -> exchange -> insert], [79.417], [2.246], [997.520],
  [transpose -> insert -> exchange], [80.617], [2.248], [439.200],
)

#table(
  columns: (2.3fr, 2.3fr, 0.7fr, 1.2fr, 1.2fr, 0.8fr),
  table.header([Algorithm 1], [Algorithm 1], [n], [t-test p-value], [Wilcoxon p-value], [sigma 5%]),
  [transpose -> exchange -> insert], [transpose -> insert -> exchange], [78], [1.2998e-25], [3.2444e-14], [Yes],
)

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
