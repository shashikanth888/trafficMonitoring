The project aims to minimize the number of security cameras required to monitor traffic at intersections. This problem is modelled as a minimum vertex cover problem and is analyzed using various algorithms. The behaviour and efficiency of each of the below algorithm for different inputs are analyzed. 

CNF-SAT: In this method, a polynomial reduction of the decision version of Vertex cover to CNF-SAT is created, which is a Boolean satisfaction problem for formulas in conjunctive normal form (CNF). The reduction is implemented using the MiniSat SAT solver as a library.
APPROX-1:This method first calculates the most frequently occurring vertex, stores it into a vertex cover container, and removes every edge incident on that vertex. The process is repeated until no edge remains.
APPROX-2:This method first selects an edge and stores its vertices in a vertex cover container. It finally deletes every edge incident on both the vertices. This entire process is repeated until no edge remains.

The reports are presented in report.pdf
