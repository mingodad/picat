# author-classify
# by Moshe Lach

Based on this paper: http://people.oregonstate.edu/~kelberta/dima/PAPERS/published/llc/khmelev-tweedie-2001.pdf.

First run "preprocess.pi", which will preprocess the files according to the instructions in the above paper and generate new, processed files. (For example, from chesterton1.txt it will produce processed_chesterton1.txt.)

Next, run "transition_probabilities.pi" to create the training data, which will be stored in "transition_probabilities.data". As this program runs, provide to it the newly processed files.

Next, run "author_classify_train.pi" to create and train a NN.

Finally, run "author_classify_test.pi" to make a prediction.

(The text files are from books by G.K. Chesterton and Herman Melville, respectively, from Project Gutenberg: http://gutenberg.net/.)
