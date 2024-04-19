# Steel-making Production Line Scheduler (PLS)

## Introduction

The Steel-making Production Line Scheduler (PLS) aims to assist a medium-sized steel-making manufacturer in optimizing three production plants

## Group Member
- Chiu Tsz Ching 22088546D
- Luo Yuehan 20069609D
- Leung Chu Shing 23030897D

## Project Scope

- Process Management
- Interprocess Communication
- Scheduling algorithm
- System Exit

## Project Concept

The project utilizes scheduling algorithms like:
- First Come First Serve (FCFS)
- Priority (PR)
- Shortest Job First (SJF)
- Maximum Throughput Scheduling (MIS) [*Highest utilization]

## Software Structures

Acts as the parent process and coordinates the overall functioning of the application. It is responsible for collecting order details, processing user inputs, input exception handles, and managing interprocess communication through pipes.

### Input Module

Acts as the parent process and coordinates the overall functioning of the application. It is responsible for collecting order details, processing user inputs, mapping commands to corresponding functions, and managing interprocess communication through pipes.

### Scheduling Module

This module, running as a child process, receives data from the Input Module and applies scheduling algorithms to generate production schedules. It sends back the scheduling results to the parent process.

### Output Module

Another child process that receives scheduling data from the parent process and formats it for presentation. It outputs detailed schedules and analysis for the factory manager's review.

## Performance Analysis

Details on user manual preparation, environment setup, file checklist, program compilation and execution, and error handling.

## User Manual

- **Environment:** Linux / MacOS
- **File Checklist:**
    - `G50_PLS.c`
    - `LinkedList.h`
    - `date.h`
    - `test_data_G50[FCFS, MTS, PR, SJF].dat`
    - `algorithm`
    - `object`

### Command guide
1. `addPEIOD 2024-06-01 2024-06-30`

Format: addPERIOD [start date] [end date]

It is to specify the period, (start date and end date) for
scheduling the production. Date format is year-month-day, i.e.
YYYY-MM-DD.


2. `addORDER P0001 2024-06-10 2000 Product_A`

Format:  addORDER [Order Number] [Due Date] [Quantity] [Product Name]

This command is to add an order and the details to the scheduler.
Follow by the Order Number, Due Date, Quantity and Product Name

3. `addBATCH orderBATCH01.dat`

Format: addBATCH [Orders in a batch file]

[addBATCH] is to input multiple orders in one batch file which
is a basic text format. That is there are many lines of
“addORDER” in the file.

4. `runPLS FCFS | printREPORT > report_01_FCFS.txt`

Format: runPLS [Algorithm] | printREPORT > [Report file name]

This commadn is to generate a schedule with the specified
[Algorithm]. is to execute a scheduling algorithm.
By using the vertical bar [|], the schedule is passed to
[printREPORT] command and to print a report of that schedule
with the analysis details, for example, the utilization.
The greater than sign [>] is to export the report to the given
file name. The file name is composed by “report_” + [a sequence
number] + [algorithm used].

5. `exitPLS`

[exitPLS] is to terminate the program.

### Move this project to PolyU apollo
You can follow the command like below

1. Create required file in polyU apollo
```bash
mkdir COMP2432_project
cd COMP2432_project
mkdir object
mkdir algorithm
```

2. SCP to move file to apollo

```bash
scp /Your_file_path/COMP2432_project/object/* You_PolyU_ID_here@csdoor.comp.polyu.edu.hk:/home/23030897d/COMP2432_project/object/

scp /Your_file_path/COMP2432_project/algorithm/* You_PolyU_ID_here@csdoor.comp.polyu.edu.hk:/home/23030897d/COMP2432_project/algorithm/

scp /Your_file_path/COMP2432_project/* You_PolyU_ID_here@csdoor.comp.polyu.edu.hk:/home/23030897d/COMP2432_project/
```

### Compile the Program
You can follow the command like below

1.
```bash
gcc PLS_G50.c -o PLS_G50
```

2.
```bash
./PLS_G50
```