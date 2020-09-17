# language-classify
# by Moshe Lach

Based on the FANN tutorial (http://fann.sourceforge.net/fann_en.pdf). 

The goal is to create a NN that given a text file of English, French, or Spanish will be able to determine which of those 3 languages it is. It does so based on the frequencies of the 26 letters of the alphabet. 

The "lang_classify_train.pi" program creates and trains a NN using the dataset "frequencies.data". 

The program in "lang_classify_test.pi" can then be compiled and used; it asks for the name of a text file, finds the frequencies, and runs that information through the NN to classify the language as either English, French, or Spanish.
