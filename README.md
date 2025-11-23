[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/N3PJGR5G)
# Homework2. CC Simulator

## Overview

In this project, your task is to implement database concurrency control simulator using C++.
This project includes two major concurrency control scheme: rigorous two-phase locking (R2PL) and
opEmisEc concurrency control (OCC).

See this [PDF](hw2.pdf) file.

## Build

```
cd [repository-name]
mkdir bld && cd bld
cmake ..
make -j
```

## Run
```
cd [repository-name]
cd bld

# R2PL test
./lock ../tests/test1.txt &> test1.txt
diff test1.txt ../solutions/lock-sol1.out

# OCC test
./occ ../tests/test1.txt
diff test1.txt ../solutions/occ-sol1.out
```


## Submission
| Due: 2025.12.4 (Thu) 23:59:59   
| Warning: DO NOT LEAVE THE GITHUB CLASSROOM

1. Complete your task for homeowrk 2
2. Commit and Push

-   You can commit and push as you can before the deadline For final commit message for final submission, please set the commit message as submission-student-id (e.g., `git commit -m "submission-1234567"`)

```
git add .
git commit -m "submission-student-id"
git push
```

## Late Submission Policy

- 75% : 1 day late
- 50% : 2 days late
- 25% : 3 days late
- 0% : 4days and more

## Warning

- Do not use ChatGPT
- Submit your homework Github Classroom only.
- Do not need to submit any assignments on KU LMS.
- Do not upload the project to public including source code files and documents
- Do not copy other student’s answer
- Do not collaborate other students. This is an individual project (No groups)
- Do not modify the database file (i.e., Do not insert/delete/update in the database arbitrarily)
- For your query, the order of output columns (attribute) is very important. Please follow the
instruction of the problem carefully.
- Again, we will evaluate the answer by comparing the output files. Please make sure to always
verify your program works properly. (No partial points)
- Do not change your Github ID used in homework 0.