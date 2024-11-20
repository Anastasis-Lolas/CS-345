# README FILE CSD5197 LOLAS ANASTASIS
    find my email address csd5197@csd.uoc.gr

# Assignment2 POSIX Threads 
    this assignment was used so we can easily can use to the POSIX threads and semaphores for multi-threaded programms

# How it works
    We have two separate thread categories: one for the students and one for the bus.The bus is only responsible for going to stopA 
    and notify the students ...check if there is anyone to board and then go stop B - University and do the same thing until every student
    studied and went home.So it starts boarding the students when it is done it leaves for the Uni it deboards its students and makes the go 1-1 to Uni.Than checks if 
    there is anyone to board from stopB.if it is it takes them to stopA where they go home.The only thing that the bus does not do is handle the study session.This is 
    only responsible the students in which after they went to university there are being notified with a state number and each one goes to sleep for 5-15 seconds randomly.
    When they wake up they go back to stopB in which they wait for the bus to arrive.Lastly,because the bus is checking the students 1-1 the fifo is being kept the whole time
    and also the fifo changes when the students sleep-study in which the first to arrive after the study is the first in line for the bus.