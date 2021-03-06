# 618 Spring 2022
# L-System Forest Generation
Joshua Mathews and Nolan Mass

## SUMMARY
We will draw forests of different kinds made up of plants generated by L-systems.

## BACKGROUND
An L-system is a way to write fractals. L-systems can be used to describe a variety of fractals and in particular, can create nice looking models of plants with relatively simple instructions. An L-system consists of an initial string (called an axiom) and a set of rules, which each describe how to replace a character with some more complex string.

For example, we can create a basic L-system as follows:
We have an axiom: A and rules: A -> AB, B -> A.
The initial string in the language would be the axiom A. After one step, we apply the rule A -> AB to A, which gives us the string AB. With this new string AB, we can apply the rule A -> AB to the A and the rule B -> A to the B, giving us the string ABA. Repeating this process, we get more and more complex strings with a fractal behavior.
|Depth|String|
|-|-|
|0|A
|1|AB
|2|ABA
|3|ABAAB
|4|ABAABABA
|...

These strings can be interpreted as instructions to draw shapes, which give images of plants or other fractal designs. Changing the rules allow us to create different plants.

We will be running this algorithm multiple times to create multiple plants that would make a forest. This gives us multiple levels of parallelism that could be exploited. One is parallelism across all the plants in the forest. Another is across the pixels when drawing the final image. We could also parallelize within the plants when generating the next iteration of a plant. All these methods would help speedup the render process compared to the sequential version, especially for dense forests with detailed plants.

## THE CHALLENGE

Since this is an iterative process, the new branches created are dependent on the previous tree created. There should be lots of spatial and temporal locality in multiple sections of the code. During rendering, pixels are read and written to multiple times, and the rendering process accesses groups of nearby pixels to draw them in. In the L-system computation, multiple characters are read from strings in each iteration, and strings are read by multiple pixels. The communication to computation ratio is probably high as the generated strings are read and new strings are written, but not much computation is done at each step. There is also lots of writing to image buffers which could worsen the ratio. There is also divergent execution as different rules cause different parts of the code to run while generating the different plants.

The divergence causes workload balance issues while generating plants as well as rendering the scene. Since we want visual diversity, we introduce stochasticity into the rules, which means that some plants may be more computation heavy than others, making the balancing the computation non trivial. Furthermore, placing different plants at various locations means that some pixels would have to do more work than others when rendering the scene, making this challenging to parallellize as well.

## RESOURCES

We will start the code from scratch while referencing rendering code from Assignments 1 and 2. We will be using GHC machines and personal machines for development.

## GOALS AND DELIVERABLES

75% Goal - Render forest of plants. Parallelize over each plant individually using OpenMP.

100% Goal - Use OpenMP to parallelize the drawing of single plants (i.e. across different branches of the same plant). Modify the L-systems to generate different looking plants without creating too much divergence. Examine speedup on different types of plants (i.e. with different branching factors, different levels of detail, or different types of rules).

125% Goal - Use CUDA or Metal to further speed up the drawing of scenes on a GPU on GHC machines or M1 mac respectively.
Compare performance of CUDA with different OpenMP approaches.

For the demo, we will generate multiple forests using our method and show various visually pleasing images. We will also have speedup graphs that show how the parallized versions compare as well as how different types of plants / forests affect performance.

## PLATFORM CHOICE

We will be mainly using the GHC machines and writing code in C++. We will be parallelizing using OpenMP as well as CUDA kernels as described in the previous section. 

## SCHEDULE

Week 1 (27th - 2nd)
* Decide a starting ruleset
* Write sequential code to generate plant based on rules

Week 2 (3rd - 9th)
* Write framework for displaying the generated plants
* Add in parallelization over plants

Week 3 (10th - 16th)
* Submit milestone report
* Add more types of rules for generating plants
* Add parallelization within a single plant


Week 4 (17th - 23rd)
* Experiment with more variety of plants
* Experiment with using CUDA
* Analyze how rulesets and different approaches to parallelization affect performance

Week 5 (24th - 29th)
* Analyze CUDA vs OpenMP performance, if viable
* Generate final images and write final report
* Submit final project

## MILESTONE
* We have written sequential code to generate the instructions that describe an L-system. The code takes an arbitrary starting axiom and a ruleset so its easy to generalize to any L-system. We've also experimented with adapting the opengl rendering code from assignment 2 to render the instructions generated by the L-system.
* We are about one week behind our initial weekly goals. This is mostly from a combination of work from other courses and carnival taking up most of the past week. We can push week 2-4 back by one week and plan to scale back on some of the `nice to haves` for now.
* For the poster session, we could have some rendered images and hopefully some live demo, where we can change the parameters of the plants depending on how fast the code runs.
* Much of the work in our project is devoted to setting up the graphics pipeline so we don't have any preliminary results yet.
* The computation for our project consists of two separate problems: generation and rendering. The computation in the two steps is mostly unrelated and its tough to know which one will create the bigger bottle neck. If we parallelize inside one plant, the generation step should be parallelizable using scan and parallel rewriting but the rendering step will require some more creativity since following the generated instructions naively is a very sequential process.
