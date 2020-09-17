# Spam detector
# by Neng-Fa Zhou, November 29, 2019

This program uses a neural network to detect if an email is a spam or a ham. The original labeled emails are available at:

https://archive.ics.uci.edu/ml/datasets.php

The file "Train.csv" is a database of attributes extracted from the original labeled data set. See "spambase.names" for the
meaning of each of the attributes.

First, convert the csv file to the Fann format:

    picat csv2fann Train.csv 1 > Train.txt

where 1 means that there is only one output attribute.

Second, create and train a neural network:

    picat train Train.txt
	
Finally, test the neural network on the test dataset:

    picat test TestX.csv



