# DETERMINISTIC FINITE AUTOMATON

This small project attempts to provide tools to make and play with deterministic finite automatons. This project is mainly for educational purposes.

This project offers the following solutions:

1. A way to define deterministic finite automatons in a concise descriptive manner using established notation.

2. A solution that can read and create DFA given the established notation.

N.B. this solution is not aimed to scale, but rather to help and serve as an excercise to create small DFAs and see them in action.

## Notation

We can describe a DFA as a tuple of (S, A, T, F, ss) where:
 - S is a set of states
 - A is the alphabet
 - T is a transition function that takes a state and a letter from the alphabet and yields another state.
 - F is the subset of S and represents the set of finite states.
 - **ss** is the initial / starting state.

 We make the following assumption in our practical solution:
 - States will be represented as numbers and more specifically, S = \{ 0, 1, ... n-1 \} where |S| = n
  - Alphabet consists of ASCII characters from with their respective ASCII codes 0 to 255.
  - Transition function, hence can be represented as a matrix of |S| rows and 256 columns.
  - Initial state is 0

In order to specify a DFA that follows the abovementioned simplifying assumptions we only need to give the following information:
 - Number of states
 - Array of numbers denoting which states are final OR a way to tell that no state is a final state  (always rejecting DFA).
 - Transitions

 As such we tailor the DFA description file to be of the following format:
 1. On the first line we provide a non-zero number `N` representing the number of states that the DFA has.
 2. On the second line we provide a space separated sequence of numbers denoting the final states. Bear in mind that these numbers should be in valid range (between `0` and `N-1` inclusive). Alternatively, if we wish to say that no state is final / accepting, we would write a string `NONE`.
 3. On the following lines, we provide a description of the transitions in the form `O -> D : c` where `O` and `D` are the origin and the destination state and `c` is the transition character. Bear in mind that we should avoid defining "conflicting transtions": that is, two transitions with the same origin state and transition character, but different destination state (we are building a DFA, not NFA).

 In addition, the DFA creation process will add one extra state (corresponding to the number `N`) which we refer to as the `garbage state`. This state is a trap state that will receive all other non-mentioned pairs of origin state and transition character. All the transitions from this state are going to itself (self-looping). In this way, we will have a proper DFA and will not have to bother defining a garbage state ourselves and we can focus on the states and transitions that matter.

 ## Running the program

 One can navigate to the folder titled "examples" and there can run a makefile:

 - `make prog` - to compile "prog.out" executable
 - `make run` - to run the example program that offers interactive session with the user, giving them the option to choose between 3 preset DFAs and trying out different input words to see which ones will get accepted / rejected.

 In the same file, there are 3 examples of DFA construction in `example-<N>.txt` (for N = 1, 2, 3). The DFAs are the following:

 1. A DFA that accepts only words that consist of letter `a` whose length is divisible by 3.
 2. A DFA that accepts only the following three words: "a", "b" and "c".
 3. A DFA that accepts words that satisfy regex `a*b*`