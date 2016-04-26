# Check Mate
A completely evoluntary Neural Network AI for playing checkers.
Includes board logic, Alpha Beta Pruning, and Neural Network board evaluation.
GUI is implemented in OpenGL
Generation 281 is also included.

##Introduction

Throughout Blondie24's book, he points out that so many of their design decisions were basically blind choices.  Even after the fact, they still didn't really know why their method produced that level of performance and not another, or if even a slight difference in their method would produce very different results.  This seems to be a consequence of the black-box nature of neural networks, but we can still (slowly, painfully) explore other methods.  Our team thought to use our more up-to-date resources to our advantage.



##Goals
Our overarching goal was always the same: improve Blondie24's method in order to build a better checkers player.  How we would “improve” upon Blondie24 went through some thought and revision.
Initially, we thought to use an idea suggested in class: the graphics card.  Performing the board evaluations on the card could get nearly a ten-fold speed increase, therefore much faster training.  With the increased training speed, we could explore further along the network's skill plateau than Blondie24, and still have some left over.  What we'd do with the excess was never resolved, since we ultimately dropped the graphics card idea since it was difficult to implement for our systems and we'd come to favor another idea.
This idea was multiple network sizes, and would be the one we carried forward.  We had five computers between us, and each would train a different size of network.  Each would be trained independently, and compared after training was complete.  We hoped in this way to answer the enduring question, does size matter?


##Method
We implemented everything in C++ as that was the language we were collectively most comfortable with.  We took advantage of  its low-level capabilities like bitshifting to write some pretty fast code.  We also used a more OOP-oriented design: everything had it's own class.  A class for the board, for the genetic algorithm, for the move generator, etc.
We chose our search depth as the deepest possible that would still allow us to get a “good” number of generations out.  Due to differences in the size of the networks and performance of our machines, (i.e. a small network training on a fast multicore machine) some networks trained at three ply (three turns for each player) and some at four.
We were concerned that a single-elimination style training tournament would be too volatile and would favor “lucky” networks over more skilled ones.  For this reason we went with a round robin tournament.  We also wanted to discourage overly defensive playing, so we made a win worth 1 point, a draw -1, and a loss -2.
It was when we were considering what values to pick for population size and how many winners to carry over each generation, that we truly understood what Blondie24 meant about blind choices.  Seemed like every decision was a shot in the dark.  Based on purely intuition, we chose a population size of 30 and to carry over the best half each generation, filling back up to 30 with children spawned from the winners.
We chose to use a genetic algorithm for mutation.  It was basically copied from ai-junkie.com, so credit goes to the author.  In order to better determine the effect of network size, we tried to control the differences between our method and Blondie24's.  On the other hand, this code was already functional and even worked with our existing code.  We did make some modifications, but these were basically just removing extra features, trimming it down until it more closely matched Blondie24.  However, we didn't change the mutation rate or magnitude, which we think had an overwhelming impact on our results.
