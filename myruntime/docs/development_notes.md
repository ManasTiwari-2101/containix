# CONTAINIX Development Notes

## Overview

## Understanding Fork():-

\# Notes — `fork()` in Linux (Container Runtime Foundation)



These notes are written from the perspective of:



> “Understanding containers by understanding Linux processes.”



\---



\# 1. What is a Process?



A process is:



> a running instance of a program.



When you run:



```bash id="b83z3w"

./myprogram

```



Linux kernel creates:



\* PID (Process ID)

\* memory space

\* stack

\* heap

\* registers

\* file descriptor table

\* execution context



The program becomes a \*\*process\*\*.



\---



\# 2. Why Processes Matter for Containers



Containers are fundamentally:



\# isolated Linux processes



Docker and container runtimes ultimately:



\* create processes

\* isolate them

\* limit resources



So understanding process creation is the foundation of container internals.



\---



\# 3. `fork()` — The Core Concept



`fork()` is a Linux system call that:



> creates a new process by duplicating the calling process.



Syntax:



```c id="m0rm53"

pid\_t fork(void);

```



Header:



```c id="vl6m8l"

\#include <unistd.h>

```



\---



\# 4. Mental Model of `fork()`



Before:



```text id="4svkz3"

Single process

```



After:



```text id="8t0gq7"

Parent process

Child process

```



Both continue execution from the SAME next line after `fork()`.



\---



\# 5. First `fork()` Program



\## Code



```c id="rt4b0o"

\#include <stdio.h>

\#include <unistd.h>



int main() {



    printf("Before fork\\n");



    fork();



    printf("After fork\\n");



    return 0;

}

```



\---



\## Compilation



```bash id="h8mx6g"

gcc fork1.c -o fork1

```



\---



\## Execution



```bash id="aydxr7"

./fork1

```



\---



\## Output



```text id="f9jlwm"

Before fork

After fork

After fork

```



\---



\# 6. Why "After fork" Prints Twice



Explanation:



\* `fork()` duplicates the process

\* both parent and child continue execution

\* both execute:



```c id="z4ytkn"

printf("After fork\\n");

```



Important concept:



```text id="lhbz0m"

Code before fork -> executes once

Code after fork  -> executes twice

```



\---



\# 7. Return Value of `fork()`



`fork()` returns different values to parent and child.



| Process | Return Value |

| ------- | ------------ |

| Parent  | Child PID    |

| Child   | 0            |

| Failure | -1           |



This allows processes to identify themselves.



\---



\# 8. Parent vs Child Example



\## Code



```c id="iwmns5"

\#include <stdio.h>

\#include <unistd.h>



int main() {



    pid\_t pid = fork();



    if(pid == 0) {

        printf("I am CHILD\\n");

    }

    else {

        printf("I am PARENT\\n");

    }



    return 0;

}

```



\---



\## Key Idea



Kernel conceptually does:



```text id="8x7zdr"

Create child process

Assign child PID

Return 0 to child

Return child PID to parent

```



\---



\# 9. Process IDs



Important functions:



| Function  | Meaning             |

| --------- | ------------------- |

| getpid()  | Current process PID |

| getppid() | Parent process PID  |



\---



\# 10. PID Example



\## Code



```c id="xwt3m9"

\#include <stdio.h>

\#include <unistd.h>



int main() {



    pid\_t pid = fork();



    if(pid == 0) {

        printf("I am CHILD\\n");

        printf("My PID: %d\\n", getpid());

    }

    else {

        printf("I am PARENT\\n");

        printf("My PID: %d\\n", getpid());

    }



    return 0;

}

```



\---



\## Example Output



```text id="ukp9oh"

I am PARENT

My PID: 2386



I am CHILD

My PID: 2387

```



Observation:



\* parent and child are separate processes

\* child gets new PID



\---



\# 11. Scheduler and Concurrent Execution



After `fork()`:



\* both parent and child become runnable

\* Linux scheduler decides execution order



Output order is NOT guaranteed.



Example:



```text id="muwn1i"

PARENT

CHILD

```



or:



```text id="7q2f2v"

CHILD

PARENT

```



Both are valid.



\---



\# 12. Concurrent Execution Example



\## Code



```c id="xjlwm5"

\#include <stdio.h>

\#include <unistd.h>



int main() {



    pid\_t pid = fork();



    for(int i = 0; i < 5; i++) {



        if(pid == 0) {

            printf("CHILD %d\\n", i);

        }

        else {

            printf("PARENT %d\\n", i);

        }



        sleep(1);

    }



    return 0;

}

```



\---



\## Example Output



```text id="hrjlwm"

PARENT 0

CHILD 0

PARENT 1

CHILD 1

CHILD 2

PARENT 2

```



\---



\# 13. Context Switching



Linux scheduler rapidly switches CPU between processes.



Conceptually:



```text id="eb2vwb"

Run parent

Pause parent

Save registers

Run child

Pause child

Resume parent

```



This is called:



\# context switching



\---



\# 14. Concurrent vs Parallel



\## Concurrent



Multiple execution flows managed together.



\## Parallel



Literally executing simultaneously on different CPU cores.



Your examples demonstrated:



\# concurrency



\---



\# 15. Process Memory After `fork()`



Each process gets its own logical memory space.



\---



\# 16. Memory Isolation Example



\## Code



```c id="7kg7nf"

\#include <stdio.h>

\#include <unistd.h>



int main() {



    int x = 10;



    pid\_t pid = fork();



    if(pid == 0) {



        x = 50;



        printf("CHILD: x = %d\\n", x);

        printf("%p\\n", \&x);



    }

    else {



        x = 20;



        printf("PARENT: x = %d\\n", x);

        printf("%p\\n", \&x);

    }



    return 0;

}

```



\---



\## Example Output



```text id="p8jlwm"

PARENT: x = 20

0x7ffc7cad5560



CHILD: x = 50

0x7ffc7cad5560

```



\---



\# 17. Virtual Memory



Even though:



\* virtual address same



values differ because:



\* each process has its own virtual address space



Conceptually:



```text id="jjlwm4"

Parent:

0x7ffc... -> 20



Child:

0x7ffc... -> 50

```



Same virtual address.

Different memory context.



\---



\# 18. Copy-On-Write (COW)



Linux optimizes `fork()` using:



\# Copy-On-Write



Initially:



\* parent and child share physical memory pages



When modification occurs:



\* kernel creates private copy



\---



\## Conceptual Diagram



Initially:



```text id="wbjlwm"

Parent ----\\

             ---> Shared Memory Page

Child  ----/

```



After modification:



```text id="4qjlwm"

Parent ---> Page A

Child  ---> Page B

```



\---



\# 19. Why Copy-On-Write Is Important



Without COW:



\* entire process memory copied immediately

\* very expensive



With COW:



\* memory copied only when modified

\* `fork()` becomes fast



This optimization is critical for Linux performance.



\---



\# 20. Key Connection to Containers



Container runtimes use process creation patterns like:



```text id="vjlwm9"

fork/clone

    ↓

setup isolation

    ↓

exec container process

```



Understanding `fork()` is essential because containers are fundamentally:



\# isolated Linux processes



\---



\# 21. Important System Calls Learned



| System Call / Function | Purpose         |

| ---------------------- | --------------- |

| fork()                 | Create process  |

| getpid()               | Current PID     |

| getppid()              | Parent PID      |

| sleep()                | Pause execution |



\---



\# 22. Important Insights Learned



\## Insight 1



`fork()` duplicates execution flow.



\## Insight 2



Parent and child execute independently.



\## Insight 3



Execution order is controlled by scheduler.



\## Insight 4



Processes use virtual memory.



\## Insight 5



Linux uses Copy-On-Write optimization.



\## Insight 6



Containers are built on Linux process primitives.



\---



\# 23. Final Core Intuition



The most important understanding from this section:



```text id="ajlwm0"

Containers are not magic.



They are isolated Linux processes created using kernel primitives.

```



And `fork()` is the first major primitive in that journey.



## Understanding exec():

\# Notes — `exec()` in Linux (Process Transformation)



These notes continue the process-management foundation required for understanding container runtimes.



\---



\# 1. Why `exec()` Exists



`fork()` creates a new process.



But after `fork()`:



\* parent and child still execute SAME program.



Example:



```text id="jlwm01"

Parent -> same code

Child  -> same code

```



Containers and shells need something more:



> run a completely different program inside the child process.



That is the purpose of:



\# `exec()`



\---



\# 2. Core Concept of `exec()`



`exec()` does NOT create a new process.



Instead:



\# it replaces the current process image with a new program.



\---



\# 3. Mental Model



Before `exec()`:



```text id="jlwm02"

PID 3000 -> my\_program

```



After `exec()`:



```text id="jlwm03"

PID 3000 -> /bin/ls

```



Important:



\* PID remains SAME

\* process becomes different program



\---



\# 4. Why This Is Important



Linux separates responsibilities cleanly:



| Function | Purpose                 |

| -------- | ----------------------- |

| fork()   | create process          |

| exec()   | replace process program |



This design powers:



\* Linux shell

\* Docker

\* Kubernetes

\* systemd

\* process managers



\---



\# 5. First `exec()` Program



\## Code



```c id="jlwm04"

\#include <stdio.h>

\#include <unistd.h>



int main() {



    printf("Before exec\\n");



    execl("/bin/ls", "ls", NULL);



    printf("After exec\\n");



    return 0;

}

```



\---



\# 6. Compilation



```bash id="jlwm05"

gcc exec\_intro.c -o exec\_intro

```



\---



\# 7. Execution



```bash id="jlwm06"

./exec\_intro

```



\---



\# 8. Output Observed



```text id="jlwm07"

Before exec



exec\_intro

fork\_independent\_parent\_child

fork\_intro

fork\_memory\_creation\_into

exec\_intro.c

fork\_independent\_parent\_child.c

fork\_intro.c

fork\_memory\_creation\_into.c

```



\---



\# 9. Important Observation



This line executed:



```text id="jlwm08"

Before exec

```



because it happened BEFORE:



```c id="jlwm09"

execl(...)

```



\---



\# 10. Why `"After exec"` Never Printed



This line:



```c id="jlwm10"

printf("After exec\\n");

```



never executed because:



\# successful `exec()` never returns



After exec:



\* old program destroyed

\* process memory replaced

\* execution transferred to new program



The original program no longer existed.



\---



\# 11. Understanding `execl()`



Syntax:



```c id="jlwm11"

execl(path, arg0, arg1, ..., NULL);

```



Example:



```c id="jlwm12"

execl("/bin/ls", "ls", NULL);

```



\---



\# 12. Meaning of Parameters



| Parameter | Meaning                   |

| --------- | ------------------------- |

| `/bin/ls` | executable path           |

| `"ls"`    | argv\[0] (program name)    |

| `NULL`    | argument list termination |



\---



\# 13. Passing Arguments



Modified example:



```c id="jlwm13"

execl("/bin/ls", "ls", "-l", NULL);

```



This passes:



```bash id="jlwm14"

ls -l

```



to the executable.



\---



\# 14. Internal Working of `exec()`



Conceptually kernel performs:



```text id="jlwm15"

Destroy old process memory

Load executable into memory

Create new stack

Load program sections

Jump to executable entry point

```



This process is called:



\# replacing the process image



\---



\# 15. Process Image



Process image includes:



\* code section

\* stack

\* heap

\* global variables

\* executable mappings



`exec()` replaces the entire image.



\---



\# 16. Important Things That Survive `exec()`



Even after process transformation:



| Property              | Survives? |

| --------------------- | --------- |

| PID                   | YES       |

| Parent relationship   | YES       |

| Some file descriptors | YES       |



\---



\# 17. PID Experiment



\## Code



```c id="jlwm16"

\#include <stdio.h>

\#include <unistd.h>



int main() {



    printf("PID before exec: %d\\n", getpid());



    execl("/bin/ls", "ls", NULL);



    return 0;

}

```



\---



\# 18. Output Observed



First execution:



```text id="jlwm17"

PID before exec: 2866



exec\_intro

fork\_independent\_parent\_child

fork\_intro

fork\_memory\_creation\_into

```



Second execution:



```text id="jlwm18"

PID before exec: 2873



total 80

\-rwxr-xr-x 1 manastiwari manastiwari 16056 May 27 05:31 exec\_intro

\-rw-r--r-- 1 manastiwari manastiwari   159 May 27 05:31 exec\_intro.c

\-rwxr-xr-x 1 manastiwari manastiwari 16072 May 27 05:17 fork\_independent\_parent\_child

\-rw-r--r-- 1 manastiwari manastiwari   288 May 27 05:16 fork\_independent\_parent\_child.c

\-rwxr-xr-x 1 manastiwari manastiwari 16096 May 27 05:06 fork\_intro

\-rw-r--r-- 1 manastiwari manastiwari   307 May 27 05:06 fork\_intro.c

\-rwxr-xr-x 1 manastiwari manastiwari 16072 May 27 05:13 fork\_memory\_creation\_into

\-rw-r--r-- 1 manastiwari manastiwari   306 May 27 05:12 fork\_memory\_creation\_into.c

```



\---



\# 19. Key Observation from PID Experiment



Each time:



```bash id="jlwm19"

./exec\_intro

```



was run:



\* shell created a NEW process

\* process got new PID



But INSIDE a single execution:



\# PID survives exec()



Meaning:



```text id="jlwm20"

process identity remains

program changes

```



\---



\# 20. How Shell Uses `fork()` + `exec()`



When user runs:



```bash id="jlwm21"

ls

```



Shell roughly does:



```text id="jlwm22"

fork()

    ↓

child process

    ↓

exec(ls)

```



Why?



Because shell itself must continue running.



If shell directly exec’d:



\* shell would disappear

\* become `ls`



\---



\# 21. Relationship Between `fork()` and `exec()`



These are complementary operations.



\## `fork()`



```text id="jlwm23"

Create another process

```



\## `exec()`



```text id="jlwm24"

Transform process into another program

```



Together:



```text id="jlwm25"

fork()

    ↓

child

    ↓

exec()

```



This is the core Linux execution model.



\---



\# 22. Connection to Containers



Container runtimes conceptually do:



```text id="jlwm26"

clone/fork

    ↓

child process

    ↓

setup namespaces

    ↓

mount filesystem

    ↓

exec(container process)

```



This is fundamentally how containers start.



\---



\# 23. Important `exec()` Variants



| Function | Description           |

| -------- | --------------------- |

| execl    | argument list         |

| execv    | argument array        |

| execvp   | searches PATH         |

| execve   | actual kernel syscall |



\---



\# 24. Important Insight About `exec()`



`exec()` does NOT:



\* spawn another process

\* create child process

\* run parallel program



Instead:



\# current process itself becomes another executable



\---



\# 25. Important System Calls / Functions Learned



| Function | Purpose               |

| -------- | --------------------- |

| execl()  | replace process image |

| getpid() | get process ID        |



\---



\# 26. Final Core Intuition



The most important understanding from this section:



```text id="jlwm27"

fork() creates process

exec() transforms process

```



And together:



```text id="jlwm28"

fork + exec

```



form the foundation of:



\* Linux shells

\* process managers

\* container runtimes

\* Docker

\* Kubernetes internals

\* system process execution model



## Understanding wait():-

\# Notes — `wait()` in Linux (Child Process Management)



These notes complete the foundational Linux process-management concepts required before moving toward container internals.



\---



\# 1. Why `wait()` Exists



After:



```c id="wt01"

fork();

```



Linux now has:



\* parent process

\* child process



Eventually child finishes execution.



Question:



> how does parent know child has exited?



That is the purpose of:



\# `wait()`



\---



\# 2. Core Concept of `wait()`



`wait()` allows parent process to:



\* pause until child finishes

\* collect child exit status

\* clean child process metadata



\---



\# 3. Header File



```c id="wt02"

\#include <sys/wait.h>

```



\---



\# 4. Syntax



```c id="wt03"

wait(NULL);

```



\---



\# 5. Meaning of `wait(NULL)`



This means:



```text id="wt04"

Parent waits for child termination

and ignores exit status

```



\---



\# 6. First `wait()` Program



\## Code



```c id="wt05"

\#include <stdio.h>

\#include <unistd.h>

\#include <sys/wait.h>



int main() {



    pid\_t pid = fork();



    if(pid == 0) {



        printf("CHILD running\\n");



        sleep(3);



        printf("CHILD exiting\\n");

    }

    else {



        printf("PARENT waiting\\n");



        wait(NULL);



        printf("PARENT resumed\\n");

    }



    return 0;

}

```



\---



\# 7. Compilation



```bash id="wt06"

gcc wait1.c -o wait1

```



\---



\# 8. Execution



```bash id="wt07"

./wait1

```



\---



\# 9. Expected Output



```text id="wt08"

PARENT waiting

CHILD running

CHILD exiting

PARENT resumed

```



\---



\# 10. Important Observation



Parent printed:



```text id="wt09"

PARENT waiting

```



Then stopped execution at:



```c id="wt10"

wait(NULL);

```



until child terminated.



After child exited:

kernel resumed parent process.



\---



\# 11. Internal Working of `wait()`



Conceptually kernel performs:



```text id="wt11"

Parent calls wait()

        ↓

Kernel blocks parent

        ↓

Child continues execution

        ↓

Child exits

        ↓

Kernel wakes parent

```



This is:



\# process synchronization



\---



\# 12. Blocking Behavior



`wait()` is a:



\# blocking system call



Meaning:

parent cannot continue until child state changes.



\---



\# 13. Why Linux Needs `wait()`



Suppose child exits immediately.



Kernel cannot completely remove child process because:



\* parent may still need exit status

\* parent may inspect termination reason



So kernel temporarily keeps child metadata.



\---



\# 14. Zombie Process



A zombie process is:



> a terminated child process whose exit status has not been collected by parent.



Important:



\* zombie is NOT executing

\* zombie uses minimal kernel resources

\* zombie still occupies process table entry



\---



\# 15. Zombie Creation Example



\## Code



```c id="wt12"

\#include <stdio.h>

\#include <unistd.h>



int main() {



    pid\_t pid = fork();



    if(pid == 0) {



        printf("CHILD exiting\\n");

    }

    else {



        printf("PARENT sleeping\\n");



        sleep(10);

    }



    return 0;

}

```



\---



\# 16. What Happens Here



Child exits immediately.



Parent remains alive for 10 seconds.



During this time:



\# child becomes zombie



because parent never called:



```c id="wt13"

wait()

```



\---



\# 17. Observing Zombie Processes



Commands:



```bash id="wt14"

ps aux | grep Z

```



or:



```bash id="wt15"

ps -el

```



Zombie processes appear with state:



```text id="wt16"

Z

```



\---



\# 18. Why Zombies Matter



Too many zombies:



\* fill process table

\* waste kernel resources

\* indicate poor process management



Proper applications must reap child processes.



\---



\# 19. Connection to Containers



This is EXTREMELY important for containers.



Inside containers:



\* PID 1 has special responsibilities

\* PID 1 must reap zombie children



If not:



\* zombie accumulation occurs inside container



This is why lightweight init systems exist:



\* tini

\* dumb-init



\---



\# 20. Relationship Between `fork()`, `exec()`, and `wait()`



Linux process model:



```text id="wt17"

fork()

    ↓

child process

    ↓

exec()

    ↓

run target program



parent

    ↓

wait()

```



This pattern powers:



\* Linux shells

\* container runtimes

\* systemd

\* Docker

\* Kubernetes internals



\---



\# 21. Exit Status



Child processes return exit codes.



Example:



```c id="wt18"

return 5;

```



Parent can retrieve this status using:



```c id="wt19"

int status;



wait(\&status);

```



Kernel stores child termination information until parent collects it.



\---



\# 22. Important System Calls / Functions Learned



| Function | Purpose                    |

| -------- | -------------------------- |

| wait()   | wait for child termination |

| sleep()  | pause execution            |

| fork()   | create process             |



\---



\# 23. Important Insights Learned



\## Insight 1



Child processes must be reaped.



\## Insight 2



`wait()` synchronizes parent and child.



\## Insight 3



Zombie processes occur when parent ignores child termination.



\## Insight 4



Kernel stores child exit metadata temporarily.



\## Insight 5



Containers depend heavily on proper child management.



\---



\# 24. Final Core Intuition



The most important understanding from this section:



```text id="wt20"

fork() creates process

exec() transforms process

wait() manages process lifecycle

```



These three primitives form the foundation of:



\* Linux process execution

\* shell architecture

\* container runtimes

\* process supervision systems

## Namespaces And Isolation

## Understanding clone():-

\# Notes — `clone()` and First Namespace Isolation (Beginning of Containers)



These notes mark the transition from:



\# normal Linux process management



to:



\# actual container internals



\---



\# 1. Why `clone()` Exists



Previously:



| syscall | purpose                |

| ------- | ---------------------- |

| fork()  | create process         |

| exec()  | replace process image  |

| wait()  | manage child lifecycle |



These are sufficient for normal Linux programs.



But containers require:



\* selective sharing

\* selective isolation



Example:



\* isolate hostname

\* isolate network

\* isolate process IDs

\* isolate filesystem



`fork()` cannot provide this flexibility.



That is why Linux provides:



\# `clone()`



\---



\# 2. Core Concept of `clone()`



`clone()` is:



\# customizable process creation



Unlike `fork()`:



\* behavior depends on flags

\* kernel resources can be shared or isolated selectively



\---



\# 3. Mental Model



\## `fork()`



```text id="cn01"

Duplicate almost everything

```



\---



\## `clone()`



```text id="cn02"

Choose what to share

Choose what to isolate

```



This flexibility enables:



\* threads

\* namespaces

\* containers



\---



\# 4. Important Insight



Linux threads are also created using:



\# `clone()`



Processes and threads in Linux are fundamentally:



\# kernel tasks with different sharing rules



This is one reason Linux process architecture is elegant.



\---



\# 5. First Namespace Goal



First container primitive:



\# isolated hostname



using:



```c id="cn03"

CLONE\_NEWUTS

```



\---



\# 6. UTS Namespace



UTS namespace isolates:



\* hostname

\* domain name



Meaning:

process inside namespace can see different hostname than host system.



\---



\# 7. Important Header Files



| Header     | Purpose                     |

| ---------- | --------------------------- |

| sched.h    | clone() and namespace flags |

| stdio.h    | printf                      |

| stdlib.h   | system()                    |

| unistd.h   | Linux system calls          |

| sys/wait.h | wait()                      |



\---



\# 8. First `clone()` Program



\## Code



```c id="cn04"

\#define \_GNU\_SOURCE



\#include <sched.h>

\#include <stdio.h>

\#include <stdlib.h>

\#include <unistd.h>

\#include <sys/wait.h>



\#define STACK\_SIZE 1024 \* 1024



char child\_stack\[STACK\_SIZE];



int child\_function() {



    printf("Child PID: %d\\n", getpid());



    sethostname("mycontainer", 11);



    printf("Inside child hostname:\\n");



    system("hostname");



    return 0;

}



int main() {



    printf("Parent hostname:\\n");



    system("hostname");



    pid\_t pid = clone(

        child\_function,

        child\_stack + STACK\_SIZE,

        CLONE\_NEWUTS | SIGCHLD,

        NULL

    );



    wait(NULL);



    printf("Back to parent hostname:\\n");



    system("hostname");



    return 0;

}

```



\---



\# 9. `\_GNU\_SOURCE`



```c id="cn05"

\#define \_GNU\_SOURCE

```



Enables GNU/Linux-specific features.



Needed because:



\* `clone()` is Linux-specific functionality.



\---



\# 10. Manual Stack Allocation



Unlike `fork()`:

`clone()` requires manually supplied stack memory.



\---



\## Stack Allocation



```c id="cn06"

char child\_stack\[STACK\_SIZE];

```



creates memory for child stack.



\---



\## Passing Stack Top



```c id="cn07"

child\_stack + STACK\_SIZE

```



passes TOP of stack.



\---



\# 11. Why Top of Stack Is Passed



On Linux/x86:



\# stacks grow downward



Conceptually:



```text id="cn08"

High memory

    ↓

Stack grows downward

```



So clone expects:



\* initial stack pointer at top.



\---



\# 12. Child Function



Unlike `fork()`:

where child continues same execution flow,



`clone()` starts execution from:



\# separate child function



Example:



```c id="cn09"

int child\_function()

```



This provides much finer control.



\---



\# 13. Important Namespace Flag



```c id="cn10"

CLONE\_NEWUTS

```



creates:



\# new UTS namespace



This isolates:



\* hostname

\* domain name



for child process.



\---



\# 14. Important Signal Flag



```c id="cn11"

SIGCHLD

```



tells kernel:



\* notify parent when child exits

\* allows normal wait() behavior



\---



\# 15. Hostname Isolation



Inside child:



```c id="cn12"

sethostname("mycontainer", 11);

```



changes hostname ONLY inside child namespace.



Host system remains unchanged.



\---



\# 16. Compilation



```bash id="cn13"

gcc clone\_uts.c -o clone\_uts

```



\---



\# 17. Execution



Requires root privileges:



```bash id="cn14"

sudo ./clone\_uts

```



\---



\# 18. Output Observed



```text id="cn15"

Parent PID: 2908

Container PID: 2909

mycontainer

Container exited

```



\---



\# 19. Step-by-Step Execution Flow



\---



\## Step 1 — Parent Process Exists



```text id="cn16"

Parent PID: 2908

```



Normal host process.



\---



\## Step 2 — `clone()` Creates Child



Kernel creates:



\* new process

\* new UTS namespace



Child receives:



```text id="cn17"

Container PID: 2909

```



\---



\## Step 3 — Namespace Isolation



Because of:



```c id="cn18"

CLONE\_NEWUTS

```



child receives isolated hostname state.



Conceptually:



```text id="cn19"

Parent hostname state != Child hostname state

```



\---



\## Step 4 — Child Changes Hostname



Inside child:



```c id="cn20"

sethostname("mycontainer", 11);

```



changes hostname ONLY inside namespace.



\---



\## Step 5 — Hostname Verification



Inside child:



```bash id="cn21"

hostname

```



returns:



```text id="cn22"

mycontainer

```



demonstrating namespace isolation.



\---



\# 20. Massive Insight



Namespaces allow:



\# different processes to see different versions of system state



This is the foundation of containers.



\---



\# 21. Important Clarification



This is NOT virtualization.



No:



\* virtual machine

\* separate kernel

\* hardware emulation



Instead:



\# Linux kernel provides isolated views



This makes containers lightweight.



\---



\# 22. What Isolated So Far?



Currently ONLY:



\# hostname



is isolated.



Everything else still shared:



\* filesystem

\* network

\* processes

\* users



Later namespaces will isolate these too.



\---



\# 23. Why Container PID Was Normal



Output:



```text id="cn23"

Container PID: 2909

```



was still normal host PID.



Because only:



```c id="cn24"

CLONE\_NEWUTS

```



was used.



NOT:



```c id="cn25"

CLONE\_NEWPID

```



yet.



\---



\# 24. Important Understanding of `clone()`



Behavior of `clone()` changes depending on:



\# flags



Example:



| Flag         | Purpose              |

| ------------ | -------------------- |

| CLONE\_NEWUTS | isolate hostname     |

| CLONE\_NEWPID | isolate process IDs  |

| CLONE\_NEWNS  | isolate mount points |

| CLONE\_NEWNET | isolate networking   |



\---



\# 25. Relationship to Containers



Container runtimes conceptually do:



```text id="cn26"

clone()

    ↓

create namespaces

    ↓

setup isolation

    ↓

exec(container process)

```



This is fundamentally how containers start.



\---



\# 26. Important System Calls / Functions Learned



| Function      | Purpose                       |

| ------------- | ----------------------------- |

| clone()       | customizable process creation |

| sethostname() | change hostname               |

| wait()        | wait for child                |

| system()      | execute shell command         |



\---



\# 27. Important Insights Learned



\## Insight 1



Containers are built on Linux namespaces.



\## Insight 2



`clone()` enables selective isolation.



\## Insight 3



Namespaces isolate kernel state.



\## Insight 4



Containers are NOT virtual machines.



\## Insight 5



Linux threads and containers both rely on `clone()`.



\---



\# 28. Final Core Intuition



The most important understanding from this section:



```text id="cn27"

Containers are isolated Linux processes created using namespaces.

```



And `clone()` is the syscall that begins that isolation journey.



## Understanding CLONE\_NEWPID in clone():-

\# Notes — PID Namespace (`CLONE\_NEWPID`) and Process Isolation



These notes introduce one of the MOST important Linux container concepts:



\# isolated process ID spaces



This is where containers begin to feel like separate systems.



\---



\# 1. Why PID Namespace Exists



Normally on Linux:



```bash id="pn01"

ps

```



shows:



\* all visible system processes

\* global process IDs



Example:



```text id="pn02"

systemd -> PID 1

bash    -> PID 2400

nginx   -> PID 3000

```



All processes belong to same global PID space.



\---



\# 2. Problem Containers Need to Solve



Inside a container:



\* processes should NOT see host processes

\* containers should have isolated process trees



Container should think:



```text id="pn03"

I have my own process world

```



That is exactly what:



\# PID namespaces



provide.



\---



\# 3. Core Concept of PID Namespace



PID namespace isolates:



\# process ID visibility



Meaning:

same process can have:



\* one PID on host

\* another PID inside container



\---



\# 4. Massive Linux Insight



Same process can have:



\# multiple identities depending on namespace view



Example:



| Host View | Container View |

| --------- | -------------- |

| PID 3000  | PID 1          |



This dual identity is fundamental to containers.



\---



\# 5. Most Important PID Namespace Rule



The FIRST process inside a new PID namespace becomes:



\# PID 1



This is extremely important.



\---



\# 6. Why PID 1 Matters



Inside Linux:



\# PID 1 is special



Responsibilities include:



\* reaping zombie processes

\* acting like init system



This is why containers often use:



\* tini

\* dumb-init



to properly manage child processes.



\---



\# 7. First PID Namespace Program



\## Code



```c id="pn04"

\#define \_GNU\_SOURCE



\#include <sched.h>

\#include <stdio.h>

\#include <stdlib.h>

\#include <unistd.h>

\#include <sys/wait.h>



\#define STACK\_SIZE 1024 \* 1024



char child\_stack\[STACK\_SIZE];



int child\_function() {



    printf("Inside container\\n");



    printf("Container PID: %d\\n", getpid());



    system("ps");



    return 0;

}



int main() {



    printf("Parent PID: %d\\n", getpid());



    pid\_t pid = clone(

        child\_function,

        child\_stack + STACK\_SIZE,

        CLONE\_NEWPID | SIGCHLD,

        NULL

    );



    wait(NULL);



    printf("Container exited\\n");



    return 0;

}

```



\---



\# 8. Important Namespace Flag



```c id="pn05"

CLONE\_NEWPID

```



creates:



\# new PID namespace



for child process.



\---



\# 9. Compilation



```bash id="pn06"

gcc clone\_pid.c -o clone\_pid

```



\---



\# 10. Execution



```bash id="pn07"

sudo ./clone\_pid

```



\---



\# 11. Output Observed



```text id="pn08"

Parent PID: 2929

Inside container

Container PID: 1

fatal library error, lookup self

Container exited

```



\---



\# 12. Most Important Observation



Inside namespace:



```text id="pn09"

Container PID: 1

```



This proves:



\# PID namespace isolation worked



The child process became:



\# PID 1 inside its namespace



even though host still sees normal PID.



\---



\# 13. Conceptual Process View



\## Host Perspective



```text id="pn10"

PID 2929 -> parent

PID 2930 -> child

```



\---



\## Container Perspective



```text id="pn11"

PID 1 -> container process

```



Same process.

Different namespace views.



\---



\# 14. Important Realization



Namespaces do NOT create:



\* virtual machines

\* separate kernels



Instead:



\# kernel changes process perception



Containers are isolated views provided by same Linux kernel.



\---



\# 15. Why `ps` Crashed



Output:



```text id="pn12"

fatal library error, lookup self

```



occurred because:



\# `/proc` filesystem was not namespaced properly



\---



\# 16. What Is `/proc`?



`/proc` is a:



\# virtual filesystem generated by Linux kernel



Contains:



\* process information

\* memory information

\* CPU information

\* namespace-aware process data



Example:



```bash id="pn13"

ls /proc

```



shows process directories like:



```text id="pn14"

1  2  3 ...

```



\---



\# 17. Important Problem



Inside PID namespace:



\* process IDs changed

\* but `/proc` still referenced host view



So programs like:



```bash id="pn15"

ps

```



became confused.



\---



\# 18. Major Container Insight



PID namespace alone is NOT sufficient.



Containers also require:



\* mount namespace

\* isolated `/proc`

\* filesystem isolation



This is why real container runtimes additionally mount:



```bash id="pn16"

mount -t proc proc /proc

```



inside container.



\---



\# 19. Important Learning Moment



This experiment revealed:



\# namespaces depend on each other



PID namespace requires:



\* proper filesystem isolation

\* proper proc mounting



to behave correctly.



\---



\# 20. Relationship Between Namespaces So Far



| Namespace    | Isolates    |

| ------------ | ----------- |

| CLONE\_NEWUTS | hostname    |

| CLONE\_NEWPID | process IDs |



\---



\# 21. Relationship to Containers



Real container runtimes conceptually do:



```text id="pn17"

clone()

    ↓

create PID namespace

    ↓

create mount namespace

    ↓

mount /proc

    ↓

exec container process

```



This is fundamentally how container process isolation works.



\---



\# 22. Important System Calls / Functions Learned



| Function | Purpose                       |

| -------- | ----------------------------- |

| clone()  | customizable process creation |

| getpid() | current process ID            |

| system() | execute shell command         |

| wait()   | wait for child process        |



\---



\# 23. Important Insights Learned



\## Insight 1



PID namespaces isolate process visibility.



\## Insight 2



Same process can have different PIDs in different namespaces.



\## Insight 3



First process in PID namespace becomes PID 1.



\## Insight 4



PID 1 has special responsibilities.



\## Insight 5



PID namespaces alone are incomplete without proper `/proc` mounting.



\---



\# 24. Final Core Intuition



The most important understanding from this section:



```text id="pn18"

Containers create isolated process worlds using PID namespaces.

```



And inside that isolated world:



\# processes believe they are running in their own independent system.



## Understanding Mount Namespace (CLONE\_NEWNS) and Filesystem Isolation in clone():-

\# Notes — Mount Namespace (`CLONE\_NEWNS`) and Filesystem Isolation



These notes introduce one of the core filesystem isolation mechanisms used by containers:



\# mount namespaces



This namespace solved the `/proc` issue encountered during PID namespace experiments.



\---



\# 1. Problem Recap



During PID namespace experiment:



```bash id="mnn01"

ps

```



produced:



```text id="mnn02"

fatal library error, lookup self

```



even though:



\* PID namespace worked correctly

\* process became PID 1



\---



\# 2. Why `ps` Failed



Because:



\# `/proc` filesystem still belonged to host mount namespace



Meaning:



\* process IDs were isolated

\* filesystem view was NOT isolated



Userspace tools became confused because:



\* PID namespace expected namespaced `/proc`

\* host `/proc` still exposed host process information



\---



\# 3. What is a Mount Namespace?



Mount namespace isolates:



\# filesystem mount points



Meaning:

different processes can have:



\* different mounted filesystems

\* different mount tables

\* different filesystem views



\---



\# 4. Core Concept



Without mount namespace:



```text id="mnn03"

All processes share same mount table

```



With mount namespace:



```text id="mnn04"

Parent mount table != Child mount table

```



This is one of the most important container isolation primitives.



\---



\# 5. Why Containers Need Mount Namespaces



Containers rely heavily on mount namespaces for:



\* isolated root filesystem

\* isolated `/proc`

\* bind mounts

\* overlay filesystems

\* filesystem sandboxing



Without mount namespaces:

containers would directly affect host filesystem mounts.



\---



\# 6. Important Header File



```c id="mnn05"

\#include <sys/mount.h>

```



Needed for:



```c id="mnn06"

mount()

umount()

```



\---



\# 7. Combined PID + Mount Namespace Program



\## Code



```c id="mnn07"

\#define \_GNU\_SOURCE



\#include <sched.h>

\#include <stdio.h>

\#include <stdlib.h>

\#include <unistd.h>

\#include <sys/wait.h>

\#include <sys/mount.h>



\#define STACK\_SIZE 1024 \* 1024



char child\_stack\[STACK\_SIZE];



int child\_function() {



    printf("Inside container\\n");



    printf("Container PID: %d\\n", getpid());



    mount("proc", "/proc", "proc", 0, NULL);



    system("ps");



    umount("/proc");



    return 0;

}



int main() {



    printf("Parent PID: %d\\n", getpid());



    pid\_t pid = clone(

        child\_function,

        child\_stack + STACK\_SIZE,

        CLONE\_NEWPID | CLONE\_NEWNS | SIGCHLD,

        NULL

    );



    wait(NULL);



    printf("Container exited\\n");



    return 0;

}

```



\---



\# 8. Important Namespace Flag



```c id="mnn08"

CLONE\_NEWNS

```



creates:



\# new mount namespace



for child process.



\---



\# 9. Why Mount Namespace Was Necessary



Without:



```c id="mnn09"

CLONE\_NEWNS

```



this line:



```c id="mnn10"

mount("proc", "/proc", "proc", 0, NULL);

```



would affect:



\# entire host system



which would be extremely dangerous.



Mount namespace isolates mount operations safely.



\---



\# 10. What `mount()` Does



Syntax used:



```c id="mnn11"

mount("proc", "/proc", "proc", 0, NULL);

```



Meaning:



| Argument  | Meaning           |

| --------- | ----------------- |

| `"proc"`  | source filesystem |

| `"/proc"` | mount point       |

| `"proc"`  | filesystem type   |

| `0`       | flags             |

| `NULL`    | extra data        |



This mounts:



\# proc filesystem



inside container namespace.



\---



\# 11. What is `/proc`?



`/proc` is:



\# virtual filesystem generated by Linux kernel



Contains:



\* process information

\* PID mappings

\* CPU info

\* memory info

\* namespace-aware process data



Programs like:



```bash id="mnn12"

ps

top

htop

```



depend heavily on `/proc`.



\---



\# 12. Why `/proc` Must Match PID Namespace



PID namespace changes:



\# process visibility



Therefore `/proc` must also reflect:



\* container PID mappings

\* container process tree



Otherwise userspace tools malfunction.



\---



\# 13. Important Cleanup



```c id="mnn13"

umount("/proc");

```



removes proc filesystem before container exits.



Good resource-management practice.



\---



\# 14. Compilation



```bash id="mnn14"

gcc container\_proc.c -o container\_proc

```



\---



\# 15. Execution



```bash id="mnn15"

sudo ./container\_proc

```



\---



\# 16. Output Observed



```text id="mnn16"

Parent PID: 2944



Inside container

Container PID: 1



    PID TTY          TIME CMD

      1 pts/2    00:00:00 container\_proc

      2 pts/2    00:00:00 sh

      3 pts/2    00:00:00 ps



Container exited

```



\---



\# 17. Most Important Observation



Inside container:



```text id="mnn17"

Container PID: 1

```



and:



```bash id="mnn18"

ps

```



only displayed:



\* container processes



NOT host processes.



This proved:



\# process isolation was functioning correctly.



\---



\# 18. Process Tree Inside Container



Conceptually:



```text id="mnn19"

container\_proc (PID 1)

    ↓

sh (PID 2)

    ↓

ps (PID 3)

```



inside PID namespace.



\---



\# 19. Why `sh` Appeared



Because:



```c id="mnn20"

system("ps");

```



internally performs roughly:



```text id="mnn21"

fork()

    ↓

exec("/bin/sh")

    ↓

shell executes "ps"

```



Therefore:



```text id="mnn22"

sh

ps

```



both appeared in process list.



\---



\# 20. Understanding `pts/2`



Output:



```text id="mnn23"

pts/2

```



refers to:



\# pseudo terminal session



Meaning:

processes are attached to terminal device:



```text id="mnn24"

/dev/pts/2

```



\---



\# 21. Why All Processes Shared Same `pts/2`



Because:



\* child processes inherited terminal file descriptors from parent



Inherited descriptors include:



\* stdin

\* stdout

\* stderr



which were connected to:



```text id="mnn25"

/dev/pts/2

```



\---



\# 22. Important Linux Insight



Even though:



\* PID namespace isolated processes

\* mount namespace isolated filesystem mounts



terminal session was STILL shared.



Because:



\* terminal devices were not isolated yet.



\---



\# 23. Massive Container Insight



Namespaces cooperate together.



PID namespace alone was insufficient.



Mount namespace fixed:



\# filesystem view consistency



This layered isolation approach is exactly how real containers work.



\---



\# 24. Relationship Between Namespaces So Far



| Namespace    | Purpose              |

| ------------ | -------------------- |

| CLONE\_NEWUTS | isolate hostname     |

| CLONE\_NEWPID | isolate process IDs  |

| CLONE\_NEWNS  | isolate mount points |



\---



\# 25. Relationship to Containers



Real container runtimes conceptually perform:



```text id="mnn26"

clone()

    ↓

create namespaces

    ↓

mount isolated /proc

    ↓

setup filesystem

    ↓

exec container process

```



This is fundamentally how container isolation is constructed.



\---



\# 26. Important System Calls / Functions Learned



| Function | Purpose                       |

| -------- | ----------------------------- |

| clone()  | customizable process creation |

| mount()  | mount filesystem              |

| umount() | unmount filesystem            |

| system() | execute shell command         |

| wait()   | wait for child                |



\---



\# 27. Important Insights Learned



\## Insight 1



Mount namespaces isolate filesystem mount tables.



\## Insight 2



PID namespaces require properly namespaced `/proc`.



\## Insight 3



Filesystem isolation is essential for containers.



\## Insight 4



Namespaces cooperate together to build container isolation.



\## Insight 5



Containers are layered kernel isolation mechanisms.



\---



\# 28. Final Core Intuition



The most important understanding from this section:



```text id="mnn27"

Containers create isolated filesystem views using mount namespaces.

```



And this isolation allows processes inside containers to perceive:



\# their own independent filesystem environment.



## chroot() and Root Filesystem Isolation:-

    	# Notes — `chroot()` and Root Filesystem Isolation



These notes introduce one of the most important historical filesystem isolation mechanisms in Linux:



\# `chroot()`



This is where processes begin to perceive:



\# a different root filesystem



which is foundational to container design.



\---



\# 1. Problem Before `chroot()`



Even after:



\* PID namespace

\* mount namespace

\* UTS namespace



container processes still shared:



\# host root filesystem



Meaning processes could still access:



\* host directories

\* host files

\* host binaries



Example:



```text id="chr01"

/home

/etc

/usr

```



from host machine.



This is NOT proper container isolation.



\---



\# 2. Why Filesystem Isolation Matters



Containers must believe:



```text id="chr02"

I have my own filesystem

```



This is achieved by:



\# changing process root filesystem



\---



\# 3. Historical Context



Before Docker:



\* jails

\* FTP sandboxes

\* restricted environments



commonly used:



\# `chroot()`



Modern containers evolved from these older isolation ideas.



\---



\# 4. Core Concept of `chroot()`



`chroot()` changes:



\# apparent root directory



for a process.



\---



\# 5. Mental Model



Normally:



```text id="chr03"

/ -> real system root

```



After:



```c id="chr04"

chroot("./rootfs");

```



process sees:



```text id="chr05"

./rootfs -> becomes /

```



for that process.



\---



\# 6. Important Insight



`chroot()` does NOT:



\* create filesystem

\* copy files automatically

\* create virtual machine



It ONLY changes:



\# process filesystem perception



\---



\# 7. Creating Minimal Root Filesystem



First create rootfs:



```bash id="chr06"

mkdir rootfs

```



\---



\# 8. Create Required Directories



```bash id="chr07"

mkdir -p rootfs/bin

mkdir -p rootfs/lib

mkdir -p rootfs/lib64

```



\---



\# 9. Why These Directories Were Needed



Programs like:



```bash id="chr08"

/bin/sh

```



depend on:



\* shared libraries

\* runtime linker



Linux executables are usually:



\# dynamically linked



\---



\# 10. Copy Shell Binary



```bash id="chr09"

cp /bin/sh rootfs/bin/

```



\---



\# 11. Discovering Dependencies



Command:



```bash id="chr10"

ldd /bin/sh

```



Output observed:



```text id="chr11"

linux-vdso.so.1

libc.so.6 => /lib/x86\_64-linux-gnu/libc.so.6

/lib64/ld-linux-x86-64.so.2

```



\---



\# 12. Important Dynamic Linking Insight



Linux executables often depend on:



\* libc

\* runtime linker

\* additional shared libraries



Executable alone is usually insufficient.



\---



\# 13. Copy Required Libraries



Copied:



```bash id="chr12"

cp /lib/x86\_64-linux-gnu/libc.so.6 rootfs/lib/

```



and:



```bash id="chr13"

cp /lib64/ld-linux-x86-64.so.2 rootfs/lib64/

```



\---



\# 14. Massive Container Insight



Container images fundamentally contain:



\# filesystem bundles



including:



\* binaries

\* libraries

\* configs

\* runtime dependencies



This is fundamentally what Docker images are.



\---



\# 15. Verifying Rootfs



Filesystem structure became roughly:



```text id="chr14"

rootfs/

├── bin

│   └── sh

├── lib

│   └── libc.so.6

└── lib64

    └── ld-linux-x86-64.so.2

```



\---



\# 16. First `chroot()` Container Program



\## Code



```c id="chr15"

\#define \_GNU\_SOURCE



\#include <sched.h>

\#include <stdio.h>

\#include <stdlib.h>

\#include <unistd.h>

\#include <sys/wait.h>



\#define STACK\_SIZE 1024 \* 1024



char child\_stack\[STACK\_SIZE];



int child\_function() {



    printf("Inside container\\n");



    chroot("./rootfs");



    chdir("/");



    execl("/bin/sh", "sh", NULL);



    return 0;

}



int main() {



    pid\_t pid = clone(

        child\_function,

        child\_stack + STACK\_SIZE,

        SIGCHLD,

        NULL

    );



    wait(NULL);



    return 0;

}

```



\---



\# 17. Important Function



\# `chroot()`



```c id="chr16"

chroot("./rootfs");

```



changes:



\# apparent root directory



for current process.



\---



\# 18. Why `chdir("/")` Was Necessary



After changing root:

working directory might still point outside new root.



Therefore:



```c id="chr17"

chdir("/");

```



moves process safely into:



```text id="chr18"

/

```



inside new root filesystem.



\---



\# 19. Launching Shell Inside Rootfs



```c id="chr19"

execl("/bin/sh", "sh", NULL);

```



launches shell INSIDE isolated filesystem.



Shell now perceives:



\# rootfs as complete operating system



\---



\# 20. Compilation



```bash id="chr20"

gcc chroot\_container.c -o chroot\_container

```



\---



\# 21. Execution



```bash id="chr21"

sudo ./chroot\_container

```



\---



\# 22. Initial Shell Behavior



Inside shell:



```bash id="chr22"

ls

```



initially failed:



```text id="chr23"

sh: 1: ls: not found

```



\---



\# 23. Why `ls` Failed



Because rootfs only contained:



\* shell

\* libc

\* loader



There was NO:



```text id="chr24"

/bin/ls

```



inside root filesystem.



\---



\# 24. Important Container Insight



Containers only contain:



\# files copied into image/rootfs



Nothing is magically available.



This is why different container images contain different tools.



\---



\# 25. Copying `ls`



Copied executable:



```bash id="chr25"

cp /bin/ls rootfs/bin/

```



\---



\# 26. Discovering `ls` Dependencies



Command:



```bash id="chr26"

ldd /bin/ls

```



Output observed:



```text id="chr27"

libselinux.so.1

libc.so.6

libpcre2-8.so.0

ld-linux-x86-64.so.2

```



\---



\# 27. Copy Missing Libraries



Copied:



```bash id="chr28"

cp /lib/x86\_64-linux-gnu/libselinux.so.1 rootfs/lib/

```



and:



```bash id="chr29"

cp /lib/x86\_64-linux-gnu/libpcre2-8.so.0 rootfs/lib/

```



\---



\# 28. Successful Filesystem Isolation



Inside container:



```bash id="chr30"

ls /

```



Output observed:



```text id="chr31"

bin  lib  lib64

```



\---



\# 29. Most Important Observation



Container process NO LONGER saw:



\* `/home`

\* `/etc`

\* `/usr`



from host filesystem.



Instead it only saw:



\# isolated rootfs



\---



\# 30. Massive Linux Insight



You are NOT:



\* emulating Linux

\* creating VM



You are running:



\# real Linux binaries inside isolated filesystem perception



This is one of the deepest ideas behind containers.



\---



\# 31. Important Understanding of PATH



Shell command:



```bash id="chr32"

ls

```



works because shell searches executable directories using:



```bash id="chr33"

$PATH

```



including:



```text id="chr34"

/bin

```



inside isolated rootfs.



\---



\# 32. Relationship to Docker Images



Your rootfs now behaves similarly to:



\# minimal container image



Examples:



| Image Type | Characteristics   |

| ---------- | ----------------- |

| Ubuntu     | large, many tools |

| Alpine     | minimal           |

| Scratch    | almost empty      |



Your manually built rootfs resembled:



\# scratch-style minimal image



\---



\# 33. Important Limitation of `chroot()`



`chroot()` alone is NOT secure enough for production containers.



Processes may sometimes:



\# escape chroot



using filesystem tricks.



Modern containers therefore additionally use:



\* mount namespaces

\* pivot\_root()

\* capabilities

\* user namespaces



\---



\# 34. Relationship Between Isolation Mechanisms So Far



| Feature         | Purpose                          |

| --------------- | -------------------------------- |

| UTS namespace   | isolate hostname                 |

| PID namespace   | isolate process IDs              |

| Mount namespace | isolate mount tables             |

| chroot()        | isolate apparent root filesystem |



\---



\# 35. Important System Calls / Functions Learned



| Function | Purpose                         |

| -------- | ------------------------------- |

| chroot() | change apparent root directory  |

| chdir()  | change working directory        |

| execl()  | replace process image           |

| clone()  | create customized process       |

| ldd      | inspect executable dependencies |



\---



\# 36. Important Insights Learned



\## Insight 1



Containers fundamentally depend on filesystem isolation.



\## Insight 2



Linux executables usually require shared libraries.



\## Insight 3



Container images are filesystem bundles.



\## Insight 4



Processes perceive filesystem through kernel abstractions.



\## Insight 5



`chroot()` changes filesystem perception, not actual filesystem.



\---



\# 37. Final Core Intuition



The most important understanding from this section:



```text id="chr35"

Containers create isolated filesystem worlds by manipulating process root perception.

```



And inside that isolated root:



\# processes believe they are running in their own independent Linux system.



## pivot\_root() and Real Container Root Filesystem Switching:-



\# Notes — `pivot\_root()` and Real Container Root Filesystem Switching



These notes cover one of the most important low-level filesystem operations used in modern container runtimes:



\# `pivot\_root()`



This moves from:



\# educational filesystem isolation (`chroot()`)



to:



\# real container-style root switching



used by runtimes like:



\* runc

\* containerd

\* Docker internals



\---



\# 1. Why `chroot()` Was Not Enough



Previously:



```c id="pvn01"

chroot("./rootfs");

```



changed:



\# apparent root directory



for process.



But:



\* old root filesystem still existed underneath

\* privileged processes could sometimes escape

\* mount hierarchy was unchanged



So `chroot()` alone is NOT sufficient for production containers.



\---



\# 2. Core Idea of `pivot\_root()`



`pivot\_root()`:



\# swaps root filesystems



Conceptually:



Before:



```text id="pvn02"

/               -> old host root

/rootfs         -> future container root

```



After:



```text id="pvn03"

/               -> new container root

/oldrootfs      -> old host root

```



Then old root is unmounted and detached.



\---



\# 3. Massive Linux Insight



`pivot\_root()` does NOT merely hide filesystem.



It fundamentally changes:



\# mount hierarchy



This is why it is much stronger than `chroot()`.



\---



\# 4. Important Requirement



\# New root must be mount point



Kernel requires:



\# new root filesystem must already be mounted



A normal directory is insufficient.



\---



\# 5. Bind Mount Preparation



To make `rootfs` a mount point:



```bash id="pvn04"

sudo mount --bind rootfs rootfs

```



\---



\# 6. Why This Weird Command Was Necessary



Before bind mount:



```text id="pvn05"

rootfs = ordinary directory

```



After bind mount:



```text id="pvn06"

rootfs = mount point

```



Kernel now treats:



\# rootfs as mount boundary



allowing `pivot\_root()`.



\---



\# 7. Verifying Bind Mount



Command:



```bash id="pvn07"

mount | grep rootfs

```



Output observed:



```text id="pvn08"

rootfs on /init type rootfs (ro,size=6039768k,nr\_inodes=1509942)



/dev/sdd on /home/manastiwari/container\_in\_c/rootfs type ext4

(rw,relatime,discard,errors=remount-ro,data=ordered)

```



Important observation:



\# rootfs became mounted filesystem entry



\---



\# 8. First `pivot\_root()` Program



\## Code



```c id="pvn09"

\#define \_GNU\_SOURCE



\#include <sched.h>

\#include <stdio.h>

\#include <stdlib.h>

\#include <unistd.h>

\#include <sys/wait.h>

\#include <sys/mount.h>

\#include <sys/syscall.h>

\#include <sys/stat.h>



\#define STACK\_SIZE 1024 \* 1024



char child\_stack\[STACK\_SIZE];



int child\_function() {



    printf("Inside container\\n");



    mount(NULL, "/", NULL, MS\_REC | MS\_PRIVATE, NULL);



    mount("rootfs", "rootfs", NULL, MS\_BIND | MS\_REC, NULL);



    mkdir("rootfs/oldrootfs", 0777);



    syscall(SYS\_pivot\_root, "rootfs", "rootfs/oldrootfs");



    chdir("/");



    umount2("/oldrootfs", MNT\_DETACH);



    rmdir("/oldrootfs");



    execl("/bin/sh", "sh", NULL);



    return 0;

}



int main() {



    pid\_t pid = clone(

        child\_function,

        child\_stack + STACK\_SIZE,

        CLONE\_NEWNS | CLONE\_NEWPID | SIGCHLD,

        NULL

    );



    wait(NULL);



    return 0;

}

```



\---



\# 9. Important Header Files



| Header        | Purpose               |

| ------------- | --------------------- |

| sys/mount.h   | mount APIs            |

| sys/syscall.h | raw syscall interface |

| sys/stat.h    | mkdir()               |



\---



\# 10. Error Encountered During Compilation



Compilation warning:



```text id="pvn10"

warning: implicit declaration of function ‘mkdir’

```



\---



\# 11. Cause of Error



Compiler did not know declaration of:



\# `mkdir()`



because required header file was missing.



\---



\# 12. Resolution



Added:



```c id="pvn11"

\#include <sys/stat.h>

```



This fixed warning.



\---



\# 13. Important C Programming Insight



In C:

functions are declared through:



\# header files



Missing headers cause:



\* implicit declarations

\* compiler warnings

\* possible type mismatches



This becomes extremely important in systems programming.



\---



\# 14. First Important Mount Operation



```c id="pvn12"

mount(NULL, "/", NULL, MS\_REC | MS\_PRIVATE, NULL);

```



makes mounts:



\# private



\---



\# 15. Why Private Mounts Matter



Without this:

mount changes inside container could:



\# propagate back to host



Containers usually isolate mount propagation before modifying mount tree.



\---



\# 16. Bind Mounting Rootfs



```c id="pvn13"

mount("rootfs", "rootfs", NULL, MS\_BIND | MS\_REC, NULL);

```



bind mounts rootfs onto itself.



This guarantees:



\# rootfs is valid mount point



for `pivot\_root()`.



\---



\# 17. Creating Old Root Location



```c id="pvn14"

mkdir("rootfs/oldrootfs", 0777);

```



creates directory where:



\# old root filesystem will be moved



after pivot.



\---



\# 18. Most Important Line



\# `pivot\_root()`



```c id="pvn15"

syscall(SYS\_pivot\_root, "rootfs", "rootfs/oldrootfs");

```



This swaps:



\* new root

\* old root



\---



\# 19. Conceptual Filesystem View



Before pivot:



```text id="pvn16"

/               -> host root

/rootfs         -> future container root

```



After pivot:



```text id="pvn17"

/               -> container rootfs

/oldrootfs      -> previous host root

```



\---



\# 20. Changing Working Directory



```c id="pvn18"

chdir("/");

```



moves process safely into:



\# new root filesystem



after pivot.



\---



\# 21. Detaching Old Host Root



```c id="pvn19"

umount2("/oldrootfs", MNT\_DETACH);

```



disconnects:



\# previous host root filesystem



from container.



This is a HUGE isolation step.



\---



\# 22. Cleanup



```c id="pvn20"

rmdir("/oldrootfs");

```



removes leftover directory after unmount.



\---



\# 23. Launching Shell



```c id="pvn21"

execl("/bin/sh", "sh", NULL);

```



starts shell inside:



\# fully pivoted root filesystem



\---



\# 24. Compilation



```bash id="pvn22"

gcc pivot\_root\_container.c -o pivot\_root\_container

```



\---



\# 25. Execution



```bash id="pvn23"

sudo ./pivot\_root\_container

```



\---



\# 26. Output Observed



```text id="pvn24"

Inside container



\# ls

bin  lib  lib64



\# ls /

bin  lib  lib64



\# exit

```



\---



\# 27. Most Important Observation



Inside container:



```bash id="pvn25"

ls /

```



only displayed:



```text id="pvn26"

bin

lib

lib64

```



NOT host directories such as:



\* `/home`

\* `/etc`

\* `/usr`



This proved:



\# root filesystem switching succeeded



\---



\# 28. Massive Container Insight



At this point the runtime implemented:



| Feature          | Status |

| ---------------- | ------ |

| Process creation | YES    |

| exec()           | YES    |

| wait()           | YES    |

| UTS namespace    | YES    |

| PID namespace    | YES    |

| Mount namespace  | YES    |

| proc mounting    | YES    |

| chroot()         | YES    |

| pivot\_root()     | YES    |



This is already primitive container runtime architecture.



\---



\# 29. Runtime Error Encountered



After exiting shell:



```text id="pvn27"

sh: 3: Cannot set tty process group (No such process)

```



\---



\# 30. Cause of Error



Shell expected:



\* proper controlling terminal

\* foreground process group

\* session management



But runtime had NOT yet implemented:



\* `setsid()`

\* terminal control

\* job control

\* process groups



\---



\# 31. Why Shell Complained



When shell exited:

it attempted to restore terminal process group state.



Since container runtime did not manage terminal sessions fully:

shell printed warning.



\---



\# 32. Important Clarification



This warning was:



\# NOT dangerous



Filesystem isolation and namespaces still worked correctly.



\---



\# 33. Massive Runtime Insight



Real runtimes like Docker/runc additionally handle:



\* terminal forwarding

\* session control

\* signal forwarding

\* stdin/stdout piping

\* process groups

\* cgroups

\* cleanup



The current implementation focused primarily on:



\# namespace and filesystem isolation



\---



\# 34. Why `pivot\_root()` Is Better Than `chroot()`



| Feature                    | chroot() | pivot\_root() |

| -------------------------- | -------- | ------------ |

| Changes root perception    | YES      | YES          |

| Changes mount hierarchy    | NO       | YES          |

| Old root detachable        | NO       | YES          |

| Production container usage | LIMITED  | YES          |



\---



\# 35. Relationship to Real Containers



Modern container runtimes conceptually perform:



```text id="pvn28"

clone()

    ↓

create namespaces

    ↓

mount rootfs

    ↓

pivot\_root()

    ↓

unmount old root

    ↓

exec container process

```



This is fundamentally how Linux containers isolate filesystem environments.



\---



\# 36. Important System Calls / Functions Learned



| Function     | Purpose                   |

| ------------ | ------------------------- |

| mount()      | mount filesystem          |

| umount2()    | detach filesystem         |

| pivot\_root() | swap root filesystems     |

| mkdir()      | create directory          |

| chdir()      | change working directory  |

| clone()      | create customized process |



\---



\# 37. Important Insights Learned



\## Insight 1



`pivot\_root()` fundamentally changes mount hierarchy.



\## Insight 2



Container rootfs must be mount point.



\## Insight 3



Bind mounts are heavily used in container runtimes.



\## Insight 4



Filesystem isolation depends on mount namespaces.



\## Insight 5



Real container runtimes manipulate kernel mount trees directly.



\---



\# 38. Final Core Intuition



The most important understanding from this section:



```text id="pvn29"

Containers isolate filesystem reality by manipulating Linux mount hierarchy.

```



And `pivot\_root()` is one of the key mechanisms that allows processes to believe:



\# an isolated root filesystem is the entire operating system.



## Undestanding Cgroups (Control Groups) and Resource Limiting:-

\# Notes — Cgroups (Control Groups) and Resource Limiting



These notes introduce the second major pillar of containers:



\# resource control



Previously namespaces isolated:



\# what process can see



Cgroups isolate:



\# what process can consume



Together:



```text id="cgn01"

Containers = Namespaces + Cgroups

```



This is one of the most important Linux container concepts.



\---



\# 1. Why Cgroups Exist



Even with namespaces:

containers could still:



\* consume all CPU

\* consume all RAM

\* create unlimited processes



This would allow:



\* denial-of-service

\* host instability

\* resource exhaustion



Linux solves this using:



\# cgroups (Control Groups)



\---



\# 2. Core Concept of Cgroups



Cgroups provide:



\# kernel-enforced resource accounting and limits



They control:



\* CPU

\* memory

\* process counts

\* IO

\* scheduling priorities



\---



\# 3. Important Distinction



| Feature    | Purpose                |

| ---------- | ---------------------- |

| Namespaces | isolate visibility     |

| Cgroups    | isolate resource usage |



\---



\# 4. Massive Container Insight



Namespaces answer:



```text id="cgn02"

"What can process see?"

```



Cgroups answer:



```text id="cgn03"

"How much can process use?"

```



Both are necessary for real containers.



\---



\# 5. Modern Linux Uses Cgroup v2



System inspection:



```bash id="cgn04"

mount | grep cgroup

```



Output observed:



```text id="cgn05"

cgroup2 on /sys/fs/cgroup type cgroup2

(rw,nosuid,nodev,noexec,relatime,nsdelegate)

```



This confirmed:



\# cgroup v2 unified hierarchy



was enabled.



\---



\# 6. Important Linux Philosophy



Linux exposes many kernel subsystems through:



\# virtual filesystems



Examples:



\* `/proc`

\* `/sys`

\* cgroups



Cgroups are controlled using:



\# normal file operations



\---



\# 7. Inspecting Cgroup Filesystem



Command:



```bash id="cgn06"

ls /sys/fs/cgroup

```



Output observed included:



```text id="cgn07"

cgroup.procs

cpu.max

memory.max

pids.max

memory.current

cpu.stat

```



\---



\# 8. Important Cgroup Files



| File           | Purpose                 |

| -------------- | ----------------------- |

| cgroup.procs   | processes inside cgroup |

| pids.max       | process limit           |

| pids.current   | current process count   |

| memory.max     | memory limit            |

| memory.current | current RAM usage       |

| cpu.max        | CPU quota               |



\---



\# 9. Massive Linux Insight



These are NOT ordinary files.



They are:



\# live kernel control interfaces



Reading/writing them directly interacts with kernel resource manager.



\---



\# 10. Creating First Cgroup



Command:



```bash id="cgn08"

sudo mkdir /sys/fs/cgroup/mycontainer

```



This created:



\# new kernel-managed cgroup



\---



\# 11. Verifying Cgroup



Command:



```bash id="cgn09"

ls /sys/fs/cgroup/mycontainer

```



Output observed:



```text id="cgn10"

cgroup.controllers

cgroup.procs

cpu.max

memory.max

pids.max

pids.current

...

```



\---



\# 12. Important Insight



Creating directory inside:



```text id="cgn11"

/sys/fs/cgroup

```



literally creates:



\# new resource-control group in kernel



This demonstrates deep kernel integration.



\---



\# 13. First Resource Limit



\# Process Count Limit



Goal:

limit maximum processes inside container.



\---



\# 14. Setting PID Limit



Command:



```bash id="cgn12"

echo 5 | sudo tee /sys/fs/cgroup/mycontainer/pids.max

```



Meaning:



\# maximum 5 processes allowed



inside this cgroup.



\---



\# 15. Verifying Limit



Command:



```bash id="cgn13"

cat /sys/fs/cgroup/mycontainer/pids.max

```



Expected output:



```text id="cgn14"

5

```



\---



\# 16. Important Kernel Insight



This is NOT userspace checking.



Linux kernel itself enforces:



\# process creation limits



\---



\# 17. Moving Shell Into Cgroup



Command:



```bash id="cgn15"

echo $$ | sudo tee /sys/fs/cgroup/mycontainer/cgroup.procs

```



\---



\# 18. Meaning of `$$`



In shell:



```bash id="cgn16"

$$

```



means:



\# current shell PID



This command moved:



\# current shell process



into cgroup.



\---



\# 19. Important Cgroup Inheritance Rule



All child processes inherit:



\# parent cgroup membership



Meaning:



```text id="cgn17"

shell

   ↓

fork\_bomb\_test

   ↓

all children

```



automatically remain inside same cgroup.



\---



\# 20. Fork Bomb Test Program



\## Code



```c id="cgn18"

\#include <stdio.h>

\#include <unistd.h>



int main() {



    int count = 0;



    while(1) {



        pid\_t pid = fork();



        if(pid < 0) {



            printf("Fork failed at count = %d\\n", count);

            break;

        }



        if(pid == 0) {



            while(1)

                sleep(1);

        }



        count++;

    }



    return 0;

}

```



\---



\# 21. What This Program Did



Program continuously:



\# created child processes



until:



```c id="cgn19"

fork()

```



failed.



\---



\# 22. Compilation



```bash id="cgn20"

gcc fork\_bomb\_test.c -o fork\_bomb\_test

```



\---



\# 23. Execution



```bash id="cgn21"

./fork\_bomb\_test

```



\---



\# 24. Output Observed



```text id="cgn22"

Fork failed at count = 3

```



\---



\# 25. Why Failure Happened at 3 Instead of 5



Cgroup already contained:



\* shell process

\* fork\_bomb\_test process



Example breakdown:



```text id="cgn23"

bash              -> 1

fork\_bomb\_test    -> 2

child1            -> 3

child2            -> 4

child3            -> 5

```



Next fork exceeded:



\# pids.max = 5



Kernel rejected process creation.



\---



\# 26. Most Important Insight



Failure occurred because:



\# Linux kernel blocked fork()



NOT because application checked limits.



Kernel internally did:



```text id="cgn24"

Check cgroup process count

        ↓

Would limit be exceeded?

        ↓

YES

        ↓

Reject fork()

```



\---



\# 27. Massive Container Runtime Insight



This is EXACTLY how:



\* Docker enforces PID limits

\* Kubernetes enforces quotas

\* cloud systems prevent abuse



\---



\# 28. Important Side Effect Experienced



After moving shell into limited cgroup:

commands involving:



\* sudo

\* tee

\* additional forks



began failing unexpectedly.



Example:



```text id="cgn25"

No space left / sudden shell exit

```



\---



\# 29. Why Shell Became Unstable



Because shell itself was inside restricted cgroup.



Additional commands needed:



\* helper processes

\* forks

\* child processes



But:



\# no PID slots remained



Kernel blocked further forks.



\---



\# 30. Massive Linux Insight



Cgroups affect:



\# entire process trees



not just one process.



\---



\# 31. Recovery Method



Opened:



\# fresh WSL terminal



New shell was outside restricted cgroup.



Problem solved immediately.



\---



\# 32. Cleanup Problems Encountered



Attempted removal:



```bash id="cgn26"

sudo rmdir /sys/fs/cgroup/mycontainer

```



produced:



```text id="cgn27"

Device or resource busy

```



\---



\# 33. Why Cleanup Failed



Kernel refuses deleting cgroup while:



\# processes are still attached



\---



\# 34. Inspecting Remaining Processes



Command:



```bash id="cgn28"

cat /sys/fs/cgroup/mycontainer/cgroup.procs

```



Output observed:



```text id="cgn29"

9610

9611

9612

```



Meaning:



\# processes still remained inside cgroup



\---



\# 35. Killing Remaining Processes



Command:



```bash id="cgn30"

sudo pkill fork\_bomb\_test

```



\---



\# 36. Verifying Cleanup



Command:



```bash id="cgn31"

cat /sys/fs/cgroup/mycontainer/cgroup.procs

```



Eventually became empty.



Meaning:



\# no processes remained attached



\---



\# 37. Successful Cgroup Removal



Command:



```bash id="cgn32"

sudo rmdir /sys/fs/cgroup/mycontainer

```



finally succeeded.



\---



\# 38. Important Cleanup Procedure Summary



\## Step 1 — Remove limits if necessary



```bash id="cgn33"

echo max | sudo tee /sys/fs/cgroup/mycontainer/pids.max

```



\---



\## Step 2 — Inspect remaining processes



```bash id="cgn34"

cat /sys/fs/cgroup/mycontainer/cgroup.procs

```



\---



\## Step 3 — Kill remaining processes



```bash id="cgn35"

sudo pkill fork\_bomb\_test

```



or kill individual PIDs.



\---



\## Step 4 — Verify cgroup empty



```bash id="cgn36"

cat /sys/fs/cgroup/mycontainer/cgroup.procs

```



should produce no output.



\---



\## Step 5 — Remove cgroup



```bash id="cgn37"

sudo rmdir /sys/fs/cgroup/mycontainer

```



\---



\# 39. Important Linux Insight About Cleanup



Cgroups are:



\# live kernel-managed resource hierarchies



Kernel protects:



\* accounting integrity

\* hierarchy consistency



by preventing deletion of active cgroups.



\---



\# 40. Relationship to Real Containers



Real container runtimes must cleanup:



\* namespaces

\* mounts

\* cgroups

\* zombies

\* process trees



Otherwise systems accumulate:



\* leaked cgroups

\* leaked mounts

\* zombie processes



Container cleanup is one of the hardest runtime engineering problems.



\---



\# 41. Relationship Between Namespaces and Cgroups



| Mechanism  | Controls       |

| ---------- | -------------- |

| Namespaces | visibility     |

| Cgroups    | resource usage |



Together:



```text id="cgn38"

Container

    =

Namespaces

    +

Cgroups

```



\---



\# 42. Important System Calls / Concepts Learned



| Concept                  | Purpose                        |

| ------------------------ | ------------------------------ |

| cgroup.procs             | attach processes               |

| pids.max                 | process limit                  |

| fork()                   | process creation               |

| kernel enforcement       | deny resource overuse          |

| hierarchical inheritance | child processes inherit cgroup |



\---



\# 43. Important Insights Learned



\## Insight 1



Cgroups are enforced directly by kernel.



\## Insight 2



Cgroups affect entire process hierarchies.



\## Insight 3



Resource limits can destabilize shells/process trees.



\## Insight 4



Kernel prevents deletion of active cgroups.



\## Insight 5



Containers require both isolation and resource control.



\---



\# 44. Final Core Intuition



The most important understanding from this section:



```text id="cgn39"

Namespaces isolate what processes can see.

Cgroups isolate what processes can consume.

```



Together they form:



\# the foundation of modern Linux containers.



## Understanding Linux Capabilities and Container Security :-

\# Notes — Linux Capabilities and Container Security



These notes cover:



\# Linux Capabilities



which are one of the most important security mechanisms used in:



\* Docker

\* Kubernetes

\* container runtimes

\* Linux sandboxing systems



Capabilities solve a major Linux security problem:



\# traditional root user was too powerful



\---



\# 1. Traditional Linux Security Model



Older Linux security model:



| User Type   | Privilege  |

| ----------- | ---------- |

| normal user | limited    |

| root        | all powers |



Root could:



\* mount filesystems

\* reboot machine

\* load kernel modules

\* inspect processes

\* configure networking

\* bypass permissions



This became dangerous for containers.



\---



\# 2. Problem With Containers



Inside containers:

processes often run as:



```text id="cpn01"

UID = 0

```



meaning:



\# root user



But container root should NOT be allowed to:



\* reboot host

\* load kernel modules

\* manipulate host kernel

\* access sensitive kernel operations



So Linux introduced:



\# capabilities



\---



\# 3. Core Idea of Capabilities



Instead of:



```text id="cpn02"

root = all privileges

```



Linux split root powers into:



\# smaller individual privileges



called:



\# capabilities



\---



\# 4. Modern Linux Security Model



Modern Linux:



```text id="cpn03"

root =

    CAP\_SYS\_ADMIN

    CAP\_NET\_ADMIN

    CAP\_SYS\_PTRACE

    CAP\_MKNOD

    ...

```



Each capability controls:



\# specific privileged operations



\---



\# 5. Important Capability Examples



| Capability     | Purpose                                |

| -------------- | -------------------------------------- |

| CAP\_SYS\_ADMIN  | mount operations, namespace operations |

| CAP\_NET\_ADMIN  | networking configuration               |

| CAP\_SYS\_PTRACE | inspect/debug processes                |

| CAP\_MKNOD      | create device files                    |

| CAP\_SYS\_MODULE | load kernel modules                    |

| CAP\_CHOWN      | change file ownership                  |



\---



\# 6. Most Important Capability



\# `CAP\_SYS\_ADMIN`



This capability is extremely powerful.



It controls:



\* mount operations

\* filesystem administration

\* namespace operations

\* many kernel-level administrative actions



It is often jokingly called:



```text id="cpn04"

"The new root"

```



because it grants many sensitive powers.



\---



\# 7. Massive Container Security Insight



Container runtimes aggressively drop dangerous capabilities such as:



\* `CAP\_SYS\_ADMIN`

\* `CAP\_SYS\_PTRACE`

\* `CAP\_NET\_ADMIN`



to reduce:



\# attack surface



and improve container isolation.



\---



\# 8. Inspecting Current Capabilities



Command used:



```bash id="cpn05"

capsh --print

```



\---



\# 9. Output Observed



```text id="cpn06"

Bounding set =

cap\_chown,

cap\_dac\_override,

cap\_sys\_admin,

cap\_net\_admin,

...

```



\---



\# 10. Important Observation



Current process was NOT root:



```text id="cpn07"

uid=1000(manastiwari)

```



meaning:



\# no effective elevated capabilities active



\---



\# 11. Capability Sets



Linux does NOT store capabilities in one simple list.



Processes have multiple capability sets.



\---



\# 12. Important Capability Sets



| Set         | Meaning                           |

| ----------- | --------------------------------- |

| Effective   | currently active capabilities     |

| Permitted   | capabilities process may activate |

| Bounding    | maximum capability ceiling        |

| Inheritable | survives exec()                   |

| Ambient     | special inherited capabilities    |



\---



\# 13. Effective Set



The:



\# currently active privileges



used by kernel during permission checks.



\---



\# 14. Permitted Set



Capabilities process:



\# owns and may activate



Effective capabilities must come from permitted set.



\---



\# 15. Bounding Set



Bounding set defines:



\# maximum capabilities process may ever obtain



Even root cannot exceed:



\# bounding set



This is very important for containers.



\---



\# 16. Massive Container Insight



Container runtimes often remove dangerous capabilities from:



\# bounding set



because once removed:



\# process can never regain them



This creates strong security boundaries.



\---



\# 17. Capability-Based Kernel Checks



Modern Linux kernel checks:



```text id="cpn08"

"Does process possess required capability?"

```



NOT merely:



```text id="cpn09"

"Is process root?"

```



This is one of the deepest Linux security concepts.



\---



\# 18. First Capability Experiment



\# Mount Operation



Created program:



```c id="cpn10"

\#include <stdio.h>

\#include <sys/mount.h>



int main() {



    int result = mount(

        "none",

        "/tmp",

        "tmpfs",

        0,

        ""

    );



    if(result == 0)

        printf("Mount successful\\n");

    else

        perror("Mount failed");



    return 0;

}

```



\---



\# 19. Compilation



```bash id="cpn11"

gcc mount\_test.c -o mount\_test

```



\---



\# 20. Running as Normal User



Command:



```bash id="cpn12"

./mount\_test

```



Output observed:



```text id="cpn13"

Mount failed: Operation not permitted

```



\---



\# 21. Why Mount Failed



Mount operations require:



\# `CAP\_SYS\_ADMIN`



Normal user process lacked effective:



\# CAP\_SYS\_ADMIN



Kernel denied syscall.



\---



\# 22. Running as Root



Command:



```bash id="cpn14"

sudo ./mount\_test

```



Output observed:



```text id="cpn15"

Mount successful

```



\---



\# 23. Important Insight



Root process possessed:



\# effective CAP\_SYS\_ADMIN



Therefore kernel allowed:



\# mount syscall



\---



\# 24. Most Important Realization



Kernel checks:



\# capabilities



NOT simply:



```text id="cpn16"

UID == 0

```



This is modern Linux privilege architecture.



\---



\# 25. Next Experiment



\# Dropping Capabilities



Installed development library:



```bash id="cpn17"

sudo apt install libcap-dev

```



This provided:



\* capability APIs

\* userspace capability manipulation library



\---



\# 26. Capability Dropping Program



\## Code



```c id="cpn18"

\#include <stdio.h>

\#include <stdlib.h>

\#include <sys/capability.h>

\#include <sys/mount.h>



int main() {



    cap\_t caps;



    caps = cap\_get\_proc();



    cap\_value\_t cap\_list\[1] = {CAP\_SYS\_ADMIN};



    cap\_set\_flag(

        caps,

        CAP\_EFFECTIVE,

        1,

        cap\_list,

        CAP\_CLEAR

    );



    cap\_set\_flag(

        caps,

        CAP\_PERMITTED,

        1,

        cap\_list,

        CAP\_CLEAR

    );



    cap\_set\_proc(caps);



    cap\_free(caps);



    printf("Dropped CAP\_SYS\_ADMIN\\n");



    int result = mount(

        "none",

        "/tmp",

        "tmpfs",

        0,

        ""

    );



    if(result == 0)

        printf("Mount successful\\n");

    else

        perror("Mount failed");



    return 0;

}

```



\---



\# 27. Important APIs Learned



| Function       | Purpose                          |

| -------------- | -------------------------------- |

| cap\_get\_proc() | get current process capabilities |

| cap\_set\_flag() | modify capability sets           |

| cap\_set\_proc() | apply modified capabilities      |

| cap\_free()     | cleanup capability object        |



\---



\# 28. Compilation



Important:

capability library must be linked.



Command:



```bash id="cpn19"

gcc drop\_capability.c -lcap -o drop\_capability

```



\---



\# 29. Execution



Command:



```bash id="cpn20"

sudo ./drop\_capability

```



\---



\# 30. Output Observed



```text id="cpn21"

Dropped CAP\_SYS\_ADMIN

Mount failed: Operation not permitted

```



\---



\# 31. Massive Security Insight



Even though process ran as:



\# root



mount still failed because:



\# CAP\_SYS\_ADMIN was removed



This demonstrated:



\# restricted root user



\---



\# 32. Most Important Container Security Concept



Containers often run:



\# UID 0



BUT:

dangerous capabilities are removed.



Therefore:



```text id="cpn22"

container root != unrestricted host root

```



This is a core Docker security idea.



\---



\# 33. Capability Removal Scope



Capability removal affected:



\# only that running process



NOT entire Ubuntu system.



When process exited:



\# capability restrictions disappeared



\---



\# 34. Important Clarification



Capabilities are:



\# process-local



not global kernel settings.



Meaning:



\* sudo still works normally

\* host root unaffected

\* Ubuntu not modified permanently



\---



\# 35. Important Capability Recovery Insight



Capabilities may or may not be recoverable depending on:



\# which capability set was modified



\---



\# 36. Effective Set Removal



Removing capability only from:



\# effective set



temporarily disables it.



If still present in:



\# permitted set



process may reactivate it later.



\---



\# 37. Permitted Set Removal



Removing capability from:



\# permitted set



usually prevents process from reactivating it.



Because:



```text id="cpn23"

effective ⊆ permitted

```



\---



\# 38. Bounding Set Removal



Removing capability from:



\# bounding set



is strongest restriction.



Process and children can NEVER regain it.



Even if:



\* process becomes root

\* exec() occurs

\* setuid used



\---



\# 39. Massive Runtime Insight



Container runtimes often remove dangerous capabilities from:



\# bounding set



to permanently restrict container process tree.



\---



\# 40. Important Mental Model



| Capability Set | Analogy                       |

| -------------- | ----------------------------- |

| Effective      | tools currently in hand       |

| Permitted      | tools owned in backpack       |

| Bounding       | tools allowed inside building |



\---



\# 41. Relationship to Containers



Typical runtime flow:



```text id="cpn24"

runtime starts privileged

        ↓

creates namespaces

        ↓

sets up cgroups

        ↓

drops dangerous capabilities

        ↓

execs container process

```



Result:



\# restricted container root



\---



\# 42. Relationship With exec()



Capabilities interact deeply with:



\# exec()



Capability inheritance depends on:



\* executable flags

\* inheritable set

\* ambient set

\* securebits



This becomes advanced Linux security territory.



\---



\# 43. Important Insights Learned



\## Insight 1



Modern Linux security is capability-based.



\## Insight 2



Root is no longer absolute.



\## Insight 3



Capabilities split root privileges into smaller units.



\## Insight 4



Containers use restricted root users.



\## Insight 5



Kernel checks capabilities during privileged syscalls.



\## Insight 6



Capabilities are process-local.



\## Insight 7



Bounding set creates irreversible privilege ceiling.



\---



\# 44. Final Core Intuition



The most important understanding from this section:



```text id="cpn25"

Modern Linux security is capability-based, not merely UID-based.

```



And containers achieve security by:



\# running restricted root processes with dangerous capabilities removed.



## Understanding Seccomp (Secure Computing Mode) and Syscall Sandboxing:-

\# Notes — Seccomp (Secure Computing Mode) and Syscall Sandboxing



These notes cover:



\# Seccomp



which is one of the most important Linux security mechanisms used in:



\* Docker

\* Kubernetes

\* Chrome sandbox

\* gVisor

\* Firecracker

\* browser sandboxes

\* secure container runtimes



Seccomp provides:



\# syscall filtering



and allows processes to become:



\# sandboxed



\---



\# 1. Why Seccomp Exists



Even with:



\* namespaces

\* cgroups

\* capabilities



a compromised process could still invoke dangerous:



\# system calls



Examples:



\* `ptrace`

\* `mount`

\* `bpf`

\* `reboot`

\* `clone`

\* `kexec\_load`



This increases:



\# kernel attack surface



Linux solves this using:



\# seccomp



\---



\# 2. What Are Syscalls?



Userspace programs cannot directly access kernel.



They request kernel services through:



\# system calls (syscalls)



\---



\# 3. Common Syscalls



| Syscall | Purpose          |

| ------- | ---------------- |

| read    | read file/input  |

| write   | write output     |

| open    | open file        |

| fork    | create process   |

| execve  | execute program  |

| mount   | mount filesystem |

| getpid  | get process ID   |



\---



\# 4. Core Idea of Seccomp



Seccomp allows process to define:



\# syscall filter rules



Kernel checks:



```text id="scp01"

"Is this syscall allowed?"

```



before executing syscall.



\---



\# 5. Tiny Mental Model



Without seccomp:



```text id="scp02"

process

    ↓

any syscall

    ↓

kernel

```



With seccomp:



```text id="scp03"

process

    ↓

seccomp filter

    ↓

allowed?

    ↓

YES → execute syscall

NO  → block/kill process

```



\---



\# 6. Massive Container Insight



Docker containers by default already run with:



\# seccomp profiles



which block many dangerous syscalls automatically.



Most developers use seccomp without realizing it.



\---



\# 7. First Goal



Create:



\# kernel-enforced syscall sandbox



that blocks:



\# `getpid()`



and kills process when syscall occurs.



\---



\# 8. Installing Seccomp Library



Command:



```bash id="scp04"

sudo apt install libseccomp-dev

```



This installed:



\* seccomp userspace APIs

\* seccomp headers

\* filtering library



\---



\# 9. First Seccomp Program



\## Code



```c id="scp05"

\#include <stdio.h>

\#include <seccomp.h>

\#include <unistd.h>



int main() {



    scmp\_filter\_ctx ctx;



    ctx = seccomp\_init(SCMP\_ACT\_ALLOW);



    seccomp\_rule\_add(

        ctx,

        SCMP\_ACT\_KILL,

        SCMP\_SYS(getpid),

        0

    );



    seccomp\_load(ctx);



    printf("About to call getpid()\\n");



    pid\_t pid = getpid();



    printf("PID: %d\\n", pid);



    seccomp\_release(ctx);



    return 0;

}

```



\---



\# 10. Important Headers



| Header    | Purpose                      |

| --------- | ---------------------------- |

| seccomp.h | seccomp APIs                 |

| unistd.h  | syscall wrappers like getpid |



\---



\# 11. Important Types Learned



| Type            | Purpose                |

| --------------- | ---------------------- |

| scmp\_filter\_ctx | seccomp filter context |



\---



\# 12. Creating Filter Context



```c id="scp06"

ctx = seccomp\_init(SCMP\_ACT\_ALLOW);

```



Meaning:



```text id="scp07"

Default action = allow syscalls

```



All syscalls allowed initially.



\---



\# 13. Important Seccomp Philosophy



Seccomp usually works by:



\* allowing safe syscalls

\* blocking dangerous syscalls



This creates:



\# reduced kernel attack surface



\---



\# 14. Adding Filter Rule



```c id="scp08"

seccomp\_rule\_add(...)

```



adds:



\# syscall filtering rule



\---



\# 15. Rule Meaning



```c id="scp09"

SCMP\_ACT\_KILL

```



means:



```text id="scp10"

"If this syscall occurs, kill process."

```



\---



\# 16. Target Syscall



```c id="scp11"

SCMP\_SYS(getpid)

```



represents:



\# syscall number for getpid()



\---



\# 17. Important Linux Insight



Kernel internally identifies syscalls using:



\# syscall numbers



Seccomp filters operate at:



\# syscall layer



not library-function layer.



\---



\# 18. Loading Filter



```c id="scp12"

seccomp\_load(ctx);

```



This is VERY important.



Before this:

filter existed only in userspace memory.



After this:



\# kernel begins enforcing seccomp policy



\---



\# 19. Releasing Context



```c id="scp13"

seccomp\_release(ctx);

```



frees userspace seccomp structures.



\---



\# 20. Compilation



Important:

seccomp library must be linked.



Command:



```bash id="scp14"

gcc seccomp\_test.c -lseccomp -o seccomp\_test

```



\---



\# 21. Execution



Command:



```bash id="scp15"

./seccomp\_test

```



\---



\# 22. Output Observed



```text id="scp16"

About to call getpid()

Bad system call (core dumped)

```



\---



\# 23. What Happened Internally



Program attempted:



```c id="scp17"

getpid()

```



Kernel intercepted syscall BEFORE execution.



Seccomp filter checked:



```text id="scp18"

Syscall = getpid

Allowed?

NO

```



Kernel response:



```text id="scp19"

Terminate process

```



\---



\# 24. Meaning of “Bad system call”



Shell printed:



```text id="scp20"

Bad system call

```



because process was terminated due to:



\# forbidden syscall



\---



\# 25. Meaning of “core dumped”



```text id="scp21"

(core dumped)

```



means:

kernel generated debugging snapshot before process termination.



This was NOT dangerous.



Ubuntu/WSL remained completely safe.



\---



\# 26. Massive Security Insight



This demonstrated:



\# kernel-enforced syscall filtering



This is MUCH stronger than:



\* userspace checks

\* application validation

\* library restrictions



Because:



\# kernel itself blocks syscall execution



\---



\# 27. Relationship to Containers



Docker seccomp profiles commonly block:



\* `ptrace`

\* `kexec\_load`

\* dangerous `bpf`

\* `open\_by\_handle\_at`

\* certain namespace operations



This reduces:



\# container escape possibilities



\---



\# 28. Relationship to Capabilities



| Mechanism    | Controls              |

| ------------ | --------------------- |

| Capabilities | privileged operations |

| Seccomp      | syscall availability  |



\---



\# 29. Important Difference



Capabilities ask:



```text id="scp22"

"May process perform privileged action?"

```



Seccomp asks:



```text id="scp23"

"May process invoke this syscall at all?"

```



\---



\# 30. Massive Container Security Insight



Even if attacker gains:



\* root inside container

\* arbitrary code execution



seccomp can STILL block dangerous syscalls.



This greatly improves:



\# sandbox security



\---



\# 31. Important Realization



Seccomp effectively creates:



```text id="scp24"

restricted Linux kernel API

```



for process.



This is true:



\# sandboxing



\---



\# 32. Important APIs Learned



| Function           | Purpose                 |

| ------------------ | ----------------------- |

| seccomp\_init()     | create filter context   |

| seccomp\_rule\_add() | add syscall rule        |

| seccomp\_load()     | load filter into kernel |

| seccomp\_release()  | cleanup filter          |



\---



\# 33. Important Seccomp Actions



| Action         | Meaning        |

| -------------- | -------------- |

| SCMP\_ACT\_ALLOW | allow syscall  |

| SCMP\_ACT\_KILL  | kill process   |

| SCMP\_ACT\_ERRNO | return error   |

| SCMP\_ACT\_TRAP  | trigger signal |



\---



\# 34. Important Insights Learned



\## Insight 1



Seccomp filters syscalls at kernel level.



\## Insight 2



Kernel intercepts syscalls BEFORE execution.



\## Insight 3



Containers use seccomp for sandboxing.



\## Insight 4



Seccomp reduces kernel attack surface.



\## Insight 5



Capabilities and seccomp solve different security problems.



\## Insight 6



Seccomp policies are enforced directly by kernel.



\---



\# 35. Relationship to Modern Sandboxes



Seccomp is heavily used in:



\* Docker

\* Chrome

\* Android

\* Firecracker microVMs

\* gVisor

\* sandboxed browsers



\---



\# 36. Four Core Container Security Pillars Learned



| Mechanism    | Purpose             |

| ------------ | ------------------- |

| Namespaces   | isolation           |

| Cgroups      | resource limits     |

| Capabilities | privilege reduction |

| Seccomp      | syscall filtering   |



These form:



\# foundation of Linux container security



\---



\# 37. Final Core Intuition



The most important understanding from this section:



```text id="scp25"

Seccomp allows processes to access only a restricted subset of Linux syscalls.

```



This creates:



\# kernel-enforced syscall sandboxing



which is one of the most important security layers in modern container runtimes.



## no\_new\_privs and Preventing Privilege Escalation:-

\# Notes — `no\_new\_privs` and Preventing Privilege Escalation



These notes cover:



\# `no\_new\_privs`



which is one of the most important Linux sandboxing and container security features.



It is heavily used in:



\* Docker

\* Kubernetes

\* Chrome sandbox

\* systemd

\* seccomp sandboxes

\* container runtimes



`no\_new\_privs` prevents:



\# future privilege escalation



inside process trees.



\---



\# 1. Why `no\_new\_privs` Exists



Normally in Linux:

a process may gain privileges during:



\# `exec()`



through:



\* setuid binaries

\* file capabilities

\* privileged executable transitions



This can become dangerous for sandboxed/containerized processes.



\---



\# 2. Example Problem



Suppose restricted process executes:



```bash id="nnpn01"

/usr/bin/passwd

```



`passwd` is:



\# setuid root



Meaning:

process suddenly gains:



\# root privileges



after:



\# `exec()`



\---



\# 3. Why This Is Dangerous



Imagine container runtime:



\* creates sandbox

\* drops capabilities

\* enables seccomp



BUT process later executes:



\# privileged binary



and regains powers.



This could:



\# weaken sandbox security



\---



\# 4. Solution



\# `no\_new\_privs`



Linux introduced:



\# `PR\_SET\_NO\_NEW\_PRIVS`



to tell kernel:



```text id="nnpn02"

"This process and all descendants may NEVER gain additional privileges."

```



\---



\# 5. Massive Security Insight



After enabling:



\# `no\_new\_privs`



even:



\* setuid binaries

\* file capabilities

\* exec privilege gains



become ineffective.



This is extremely important for:



\# secure sandboxing



\---



\# 6. Tiny Mental Model



Without `no\_new\_privs`:



```text id="nnpn03"

process

    ↓ exec()

may gain privileges

```



With `no\_new\_privs`:



```text id="nnpn04"

process

    ↓ exec()

privileges can NEVER increase

```



\---



\# 7. Important Linux API



\# `prctl()`



Linux uses:



```c id="nnpn05"

prctl()

```



for:



\# process-level kernel controls



It manages:



\* seccomp

\* process behaviors

\* capabilities

\* sandbox flags

\* privilege restrictions



This syscall is heavily used in runtimes.



\---



\# 8. First `no\_new\_privs` Program



\## Code



```c id="nnpn06"

\#define \_GNU\_SOURCE



\#include <stdio.h>

\#include <unistd.h>

\#include <sys/prctl.h>



int main() {



    int result;



    result = prctl(

        PR\_SET\_NO\_NEW\_PRIVS,

        1,

        0,

        0,

        0

    );



    if(result == 0)

        printf("no\_new\_privs enabled\\n");

    else

        perror("prctl failed");



    execl("/bin/sh", "sh", NULL);



    return 0;

}

```



\---



\# 9. Important Header



| Header      | Purpose                    |

| ----------- | -------------------------- |

| sys/prctl.h | process control operations |



\---



\# 10. Important Function Learned



| Function | Purpose                                |

| -------- | -------------------------------------- |

| prctl()  | process-level kernel control interface |



\---



\# 11. Meaning of `PR\_SET\_NO\_NEW\_PRIVS`



```c id="nnpn07"

PR\_SET\_NO\_NEW\_PRIVS

```



means:



```text id="nnpn08"

Enable irreversible no\_new\_privs flag

```



for current process tree.



\---



\# 12. Meaning of Argument `1`



```c id="nnpn09"

1

```



means:



\# enable feature



\---



\# 13. Important Property



\# Inherited By Children



After enabling:

all child processes inherit:



\# no\_new\_privs state



This is critical for:



\# container process trees



\---



\# 14. Important Property



\# Irreversible



Once enabled:



\# cannot be disabled



for current process tree.



Kernel intentionally designed this to prevent:



\* privilege escalation

\* sandbox escapes

\* exec tricks



\---



\# 15. Compilation



Command:



```bash id="nnpn10"

gcc no\_new\_privs\_test.c -o no\_new\_privs\_test

```



\---



\# 16. Execution



Command:



```bash id="nnpn11"

./no\_new\_privs\_test

```



\---



\# 17. Output Observed



```text id="nnpn12"

no\_new\_privs enabled

$

```



Shell launched normally.



\---



\# 18. Important Observation



At first glance:

nothing visible changed.



But internally:

kernel permanently marked process tree as:



```text id="nnpn13"

"No future privilege escalation allowed."

```



\---



\# 19. Verification Through `/proc`



Command executed inside shell:



```bash id="nnpn14"

grep NoNewPrivs /proc/self/status

```



\---



\# 20. Output Observed



```text id="nnpn15"

NoNewPrivs:     1

```



This proved:



\# kernel flag successfully enabled



\---



\# 21. Important Linux Insight



`/proc/self/status` exposes:



\# live kernel process metadata



Linux heavily exposes process state through:



\# virtual filesystem interfaces



\---



\# 22. Massive Container Runtime Insight



Most secure runtimes do:



```text id="nnpn16"

create namespaces

        ↓

setup cgroups

        ↓

drop capabilities

        ↓

enable no\_new\_privs

        ↓

load seccomp filters

        ↓

exec container process

```



This creates:



\# hardened sandboxed process tree



\---



\# 23. Relationship to Seccomp



Modern Linux strongly connects:



\# seccomp + no\_new\_privs



because without:



\# no\_new\_privs



process might later:



\* gain privileges

\* bypass assumptions

\* execute privileged binaries



So secure seccomp sandboxes almost always enable:



\# no\_new\_privs first



\---



\# 24. Relationship to Capabilities



| Mechanism    | Purpose                       |

| ------------ | ----------------------------- |

| Capabilities | restrict current privileges   |

| no\_new\_privs | prevent future privilege gain |



These mechanisms complement each other.



\---



\# 25. Important Security Insight



Even if attacker gains:



\* code execution

\* container shell



they still cannot:



\# regain elevated privileges



through:



\* setuid binaries

\* exec privilege escalation



when:



\# no\_new\_privs enabled



\---



\# 26. Important Runtime Insight



`no\_new\_privs` helps guarantee:



\# privilege monotonicity



Meaning:



```text id="nnpn17"

Privileges may decrease,

but may NEVER increase again.

```



This is extremely valuable for:



\# secure sandboxing



\---



\# 27. Important APIs / Concepts Learned



| API / Concept       | Purpose                        |

| ------------------- | ------------------------------ |

| prctl()             | process control                |

| PR\_SET\_NO\_NEW\_PRIVS | enable privilege lock          |

| exec()              | normally may change privileges |

| /proc/self/status   | inspect process kernel state   |



\---



\# 28. Important Insights Learned



\## Insight 1



Processes may normally gain privileges through exec().



\## Insight 2



`no\_new\_privs` permanently blocks future privilege escalation.



\## Insight 3



Feature is inherited by child processes.



\## Insight 4



Feature is irreversible.



\## Insight 5



Container runtimes heavily rely on this feature.



\## Insight 6



Seccomp and no\_new\_privs are tightly connected.



\---



\# 29. Relationship to Modern Sandboxes



This mechanism is widely used in:



\* Docker

\* Kubernetes

\* Chrome sandbox

\* browser isolation

\* gVisor

\* Firecracker

\* Linux application sandboxes



\---



\# 30. Current Security Mechanisms Learned



| Mechanism    | Purpose                       |

| ------------ | ----------------------------- |

| Namespaces   | isolation                     |

| Cgroups      | resource limits               |

| Capabilities | privilege reduction           |

| Seccomp      | syscall filtering             |

| no\_new\_privs | prevent future privilege gain |



These form:



\# foundation of Linux container security



\---



\# 31. Final Core Intuition



The most important understanding from this section:



```text id="nnpn18"

no\_new\_privs guarantees that a process tree can never gain additional privileges in the future.

```



This creates:



\# stronger and safer sandbox guarantees



which is essential for modern container runtimes and secure Linux sandboxing.



## First Success in Building myruntime v1 (Integrated Linux Container Runtime) :-

\# Notes — Building `myruntime` v1 (Integrated Linux Container Runtime)



This section marks the transition from:



\# isolated Linux experiments



to:



\# actual runtime engineering



Until now, individual concepts were learned separately:



\* fork()

\* exec()

\* wait()

\* clone()

\* namespaces

\* pivot\_root()

\* cgroups

\* capabilities

\* seccomp

\* no\_new\_privs



In this step, all major container primitives were:



\# integrated together



into one working runtime:



\# `myruntime`



\---



\# 1. Goal of `myruntime`



The goal was to build a minimal Linux container runtime that could:



```bash id="rt01"

sudo ./build/myruntime

```



and create:



\* isolated process tree

\* isolated hostname

\* isolated filesystem

\* isolated procfs

\* isolated shell



similar to:



\* Docker

\* runc

\* LXC



conceptually.



\---



\# 2. Major Architectural Shift



Before this step:



```text id="rt02"

learning individual Linux syscalls

```



After this step:



```text id="rt03"

engineering complete container lifecycle

```



This was the biggest conceptual transition in the project.



\---



\# 3. Runtime Directory Structure



Project structure:



```text id="rt04"

container\_in\_c/

└── myruntime/

    ├── src/

    │   └── main.c

    ├── build/

    ├── rootfs/

    ├── include/

    └── docs/

```



\---



\# 4. Why Everything Stayed Inside `main.c`



Initially considered:



\* mounts.c

\* seccomp.c

\* capabilities.c



But decision made:



\# keep everything in single main.c



Reason:



\* easier debugging

\* easier lifecycle understanding

\* easier syscall tracing

\* easier interview explanation



This matches:



\# educational runtime design



similar to:



\# “Linux containers in 500 lines”



\---



\# 5. Root Filesystem Creation



A minimal rootfs was created manually.



Commands:



```bash id="rt05"

mkdir -p rootfs/bin

```



Initially:



\* shared-library approach attempted

\* dynamic dependencies became difficult



This led to:



\# BusyBox-based rootfs



\---



\# 6. BusyBox Integration



Installed:



```bash id="rt06"

sudo apt install busybox-static

```



Verification:



```bash id="rt07"

ldd /bin/busybox

```



Output:



```text id="rt08"

not a dynamic executable

```



This proved:



\# BusyBox is statically linked



No shared libraries required.



\---



\# 7. BusyBox Rootfs Setup



Commands:



```bash id="rt09"

rm -rf rootfs



mkdir -p rootfs/bin



cp /bin/busybox rootfs/bin/

```



Created symlinks:



```bash id="rt10"

cd rootfs/bin



ln -s busybox sh

ln -s busybox ls

ln -s busybox ps

ln -s busybox hostname

```



\---



\# 8. Massive Runtime Insight Learned



BusyBox uses:



\# argv\[0]



to determine behavior.



Example:



```text id="rt11"

sh -> busybox

ps -> busybox

ls -> busybox

```



Same binary.

Different applets.



\---



\# 9. Namespace Setup



Runtime used:



```c id="rt12"

CLONE\_NEWUTS

CLONE\_NEWPID

CLONE\_NEWNS

```



Meaning:



| Namespace | Purpose               |

| --------- | --------------------- |

| UTS       | hostname isolation    |

| PID       | isolated process tree |

| NEWNS     | isolated mount table  |



\---



\# 10. clone() Usage



Container process created using:



```c id="rt13"

clone()

```



instead of:



```text id="rt14"

fork()

```



because:



\# namespaces require clone()



\---



\# 11. Proper Runtime Lifecycle Learned



Correct container order discovered:



```text id="rt15"

clone()

    ↓

sethostname()

    ↓

mount namespace setup

    ↓

MS\_PRIVATE remount

    ↓

bind mount rootfs

    ↓

pivot\_root()

    ↓

unmount old root

    ↓

mount /proc

    ↓

no\_new\_privs

    ↓

drop capabilities

    ↓

exec shell

```



This ordering became:



\# one of the most important runtime lessons



\---



\# 12. Mount Propagation Isolation



Implemented:



```c id="rt16"

mount(NULL, "/", NULL,

      MS\_REC | MS\_PRIVATE, NULL);

```



Purpose:



\# prevent mount propagation to host



Without this:



\* container mounts leak to host

\* unmount issues occur



This is heavily used in:



\* Docker

\* runc

\* LXC



\---



\# 13. pivot\_root() Integration



Implemented:



```c id="rt17"

syscall(SYS\_pivot\_root, newroot, oldroot);

```



This replaced:



\# chroot()



Important realization:



| chroot()         | pivot\_root()           |

| ---------------- | ---------------------- |

| path trick       | real root switch       |

| weaker isolation | actual mount isolation |

| easier escape    | safer                  |



\---



\# 14. oldroot Cleanup



After pivot\_root():



```c id="rt18"

umount2("/oldroot", MNT\_DETACH);

rmdir("/oldroot");

```



This detached:



\# host filesystem completely



from container.



\---



\# 15. `/proc` Mounting



Implemented:



```c id="rt19"

mount("proc", "/proc", "proc", 0, NULL);

```



Purpose:



\# namespace-local process visibility



Without `/proc`:



\* ps fails

\* process tools fail

\* procfs unavailable



\---



\# 16. `no\_new\_privs`



Implemented:



```c id="rt20"

prctl(PR\_SET\_NO\_NEW\_PRIVS, 1, 0, 0, 0);

```



Purpose:



\# prevent future privilege escalation



This blocks:



\* setuid escalation

\* exec privilege gain

\* sandbox escape attempts



\---



\# 17. Capability Dropping



Implemented:



```c id="rt21"

cap\_set\_proc()

```



Purpose:



\# reduce container privileges



Important lesson learned:



Capabilities must be dropped:



\# AFTER mounts



because:



\* mount()

\* pivot\_root()



require:



\# CAP\_SYS\_ADMIN



\---



\# 18. Critical Debugging Journey



Several major debugging issues occurred.



\---



\# Issue 1 — BusyBox “not found”



Observed:



```text id="rt22"

/bin/busybox: not found

```



even though binary existed.



Root cause:



\# incorrect pivot\_root relative paths



NOT missing binary.



Massive Linux insight learned:



```text id="rt23"

"No such file or directory"

```



may actually mean:



\# ELF interpreter or path resolution failure



\---



\# Issue 2 — Relative Path Failure



Original code used:



```c id="rt24"

../rootfs

```



Problem:



\# relative paths become unreliable



after:



\* mount namespace

\* pivot\_root()

\* cwd changes



Fix:



\# switched to absolute paths



\---



\# Issue 3 — `ps` Failure



Initially:



```text id="rt25"

fatal library error, lookup self

```



Root cause:



\# missing procfs



Fix:



\# mounted /proc inside namespace



\---



\# 19. Final Successful Runtime Output



Execution:



```bash id="rt26"

sudo ./build/myruntime

```



Output:



```text id="rt27"

Starting myruntime...

Inside container

```



Inside container:



```bash id="rt28"

/bin/hostname

```



Output:



```text id="rt29"

myruntime

```



\---



\# 20. PID Namespace Verification



Command:



```bash id="rt30"

/bin/ps

```



Output:



```text id="rt31"

PID   USER     COMMAND

    1 0        sh

    3 0        /bin/ps

```



This proved:



\# PID namespace isolation works



Container shell became:



\# PID 1



exactly like real containers.



\---



\# 21. Filesystem Isolation Verification



Command:



```bash id="rt32"

ls /

```



Output:



```text id="rt33"

bin   proc

```



This proved:



\# host filesystem completely isolated



\---



\# 22. Major Container Runtime Concepts Learned



| Concept            | Understanding                |

| ------------------ | ---------------------------- |

| Namespaces         | process isolation            |

| pivot\_root()       | real filesystem switching    |

| procfs             | namespace process visibility |

| BusyBox            | minimal userspace            |

| capabilities       | privilege reduction          |

| no\_new\_privs       | future privilege lock        |

| mount propagation  | host/container isolation     |

| lifecycle ordering | runtime architecture         |



\---



\# 23. Biggest Engineering Realization



Containerization is NOT:



\# “just namespaces”



Real containers require:



\* filesystem setup

\* procfs

\* privilege management

\* mount isolation

\* runtime lifecycle ordering

\* userspace environment



\---



\# 24. Current Runtime Features Achieved



| Feature             | Status |

| ------------------- | ------ |

| PID namespace       | ✅      |

| UTS namespace       | ✅      |

| Mount namespace     | ✅      |

| pivot\_root()        | ✅      |

| Isolated rootfs     | ✅      |

| BusyBox userspace   | ✅      |

| /proc mount         | ✅      |

| Capability dropping | ✅      |

| no\_new\_privs        | ✅      |

| Shell execution     | ✅      |



\---



\# 25. Massive Milestone Reached



At this stage:



\# a real minimal Linux container runtime exists



Conceptually similar to:



\* runc

\* LXC

\* Docker runtime layer



although simplified for learning purposes.



\---



\# 26. Next Planned Phase



Next subsystem to learn:



\# container networking



Including:



\* network namespaces

\* veth pairs

\* bridges

\* IP assignment

\* NAT

\* internet access inside container



This will make containers:



\# fully isolated execution environments



instead of only:



\# isolated process environments.



## Container Networking (Phase 1) Complete :-

\# Notes — Container Networking (Phase 1)



This phase introduced:



\# Linux container networking fundamentals



Until this point, the runtime only had:



\* process isolation

\* filesystem isolation

\* namespace isolation



But containers still had:



\# no network connectivity



This phase solved the first part of that problem.



\---



\# 1. Why Networking Matters



A container without networking is basically:



\# isolated shell



Real containers must:



\* communicate with host

\* communicate with internet

\* communicate with other containers



This requires:



\# Linux virtual networking



\---



\# 2. First Major Concept Learned



\# Network Namespace



Linux provides:



```c id="net01"

CLONE\_NEWNET

```



which creates:



\# isolated network stack



for a process.



\---



\# 3. What Gets Isolated In Network Namespace



A network namespace contains separate:



| Resource        | Isolated? |

| --------------- | --------- |

| interfaces      | ✅         |

| routing tables  | ✅         |

| ARP cache       | ✅         |

| firewall rules  | ✅         |

| sockets         | ✅         |

| loopback device | ✅         |



This means:



\# each namespace has its own networking world



\---



\# 4. Manual Namespace Experiment



Created namespace:



```bash id="net02"

sudo ip netns add ns1

```



Checked namespaces:



```bash id="net03"

ip netns list

```



Output:



```text id="net04"

ns1

```



\---



\# 5. Entering Namespace



Entered namespace shell:



```bash id="net05"

sudo ip netns exec ns1 bash

```



This launched:



\# shell inside isolated network namespace



\---



\# 6. First Observation — Only Loopback Exists



Command:



```bash id="net06"

ip addr

```



Output:



```text id="net07"

1: lo: <LOOPBACK> mtu 65536 qdisc noop state DOWN

```



Important observations:



\* only `lo` existed

\* loopback was DOWN

\* no ethernet interface existed

\* no internet access existed



\---



\# 7. Major Networking Insight Learned



Even:



\# localhost belongs to namespace



This surprised many beginners.



Each namespace has:



\# independent loopback device



Meaning:



```text id="net08"

127.0.0.1

```



inside namespace:



\# is NOT host localhost



\---



\# 8. Ping Failure



Command:



```bash id="net09"

ping 127.0.0.1

```



Output:



```text id="net10"

ping: connect: Network is unreachable

```



Root cause:



\# loopback interface was DOWN



This was a MASSIVE Linux insight.



Even localhost requires:



\* active interface

\* routing

\* operational network device



\---



\# 9. Enabling Loopback



Command:



```bash id="net11"

ip link set lo up

```



This changed:



```text id="net12"

DOWN → UP

```



for loopback device.



\---



\# 10. Successful Localhost Ping



Command:



```bash id="net13"

ping 127.0.0.1

```



Output:



```text id="net14"

64 bytes from 127.0.0.1

```



This proved:



\# namespace-local networking became operational



\---



\# 11. Major Concept Learned



\# Interfaces Exist Independently Of State



| State            | Meaning               |

| ---------------- | --------------------- |

| interface exists | kernel created device |

| interface UP     | device operational    |



This applied to:



\* loopback

\* veth devices

\* ethernet interfaces



\---



\# 12. Next Major Concept



\# Virtual Ethernet Pair (veth)



Created veth pair:



```bash id="net15"

sudo ip link add veth-host type veth peer name veth-cont

```



Linux created:



```text id="net16"

veth-host <=======> veth-cont

```



This behaves like:



\# virtual ethernet cable



Packets entering one side:



\# emerge from the other side



\---



\# 13. Interface Verification



Command:



```bash id="net17"

ip link

```



Output included:



```text id="net18"

veth-cont@veth-host

veth-host@veth-cont

```



This proved:



\# kernel internally paired interfaces



\---



\# 14. Important Networking Insight



veth pairs behave conceptually similar to:



```text id="net19"

socketpair()

```



but at:



\# Ethernet device layer



instead of:



\# process socket layer



\---



\# 15. Moving Interface Into Namespace



Command:



```bash id="net20"

sudo ip link set veth-cont netns ns1

```



This was one of the MOST important container networking concepts.



Linux literally changed:



\# namespace ownership



of:



```text id="net21"

veth-cont

```



\---



\# 16. Interface Ownership After Move



| Interface | Namespace |

| --------- | --------- |

| veth-host | host      |

| veth-cont | ns1       |



This is EXACTLY how containers receive:



\# isolated network interfaces



\---



\# 17. Host Interface Configuration



Commands:



```bash id="net22"

sudo ip addr add 10.0.0.1/24 dev veth-host

sudo ip link set veth-host up

```



Host side became:



```text id="net23"

10.0.0.1/24

```



\---



\# 18. Namespace Interface Configuration



Entered namespace again:



```bash id="net24"

sudo ip netns exec ns1 bash

```



Configured namespace interface:



```bash id="net25"

ip addr add 10.0.0.2/24 dev veth-cont

ip link set veth-cont up

ip link set lo up

```



Namespace side became:



```text id="net26"

10.0.0.2/24

```



\---



\# 19. Subnet Understanding



Both interfaces belonged to:



```text id="net27"

10.0.0.0/24

```



Meaning:



\* same network

\* direct communication possible



\---



\# 20. Major Networking Insight



\# Interfaces Start DOWN By Default



This applied to:



\* loopback

\* veth-host

\* veth-cont



Interfaces must be explicitly enabled using:



```bash id="net28"

ip link set <interface> up

```



\---



\# 21. Successful Cross-Namespace Communication



Inside namespace:



```bash id="net29"

ping 10.0.0.1

```



Output:



```text id="net30"

64 bytes from 10.0.0.1

```



This proved:



\# packets successfully crossed namespaces



through:



\# virtual ethernet pair



\---



\# 22. Actual Packet Flow Learned



Packet path:



```text id="net31"

namespace process

        ↓

veth-cont

        ↓

kernel virtual cable

        ↓

veth-host

        ↓

host namespace

```



This is EXACTLY how container networking fundamentally works.



\---



\# 23. Major Docker Insight



Docker internally performs conceptually:



```text id="net32"

container eth0

      ↓

veth pair

      ↓

docker0 bridge

      ↓

host network

      ↓

internet

```



This experiment recreated:



\# Docker’s core networking mechanism manually



\---



\# 24. Major Realization



Containers do NOT get networking magically.



Linux requires:



\* namespace creation

\* virtual interface creation

\* interface movement

\* IP assignment

\* route setup



All explicitly configured.



\---



\# 25. Important Difference From Virtual Machines



| Virtual Machines     | Containers                   |

| -------------------- | ---------------------------- |

| emulate NIC hardware | reuse host kernel            |

| heavier              | lighter                      |

| full OS              | isolated process environment |



Containers isolate networking using:



\# namespaces



instead of:



\# hardware emulation



\---



\# 26. Biggest Networking Concepts Learned



| Concept                      | Understanding                  |

| ---------------------------- | ------------------------------ |

| network namespace            | isolated network stack         |

| loopback isolation           | namespace-local localhost      |

| veth pair                    | virtual ethernet cable         |

| interface movement           | namespace ownership transfer   |

| IP assignment                | independent namespace identity |

| interface UP state           | operational activation         |

| cross-namespace connectivity | packet routing through veth    |



\---



\# 27. Current Networking Progress



| Feature                              | Status |

| ------------------------------------ | ------ |

| network namespaces                   | ✅      |

| isolated loopback                    | ✅      |

| veth pairs                           | ✅      |

| moving interfaces between namespaces | ✅      |

| IP assignment                        | ✅      |

| namespace ↔ host communication       | ✅      |



\---



\# 28. Major Milestone Reached



At this stage:



\# first real container network connection exists



The container namespace can now:



\* own interfaces

\* own IP address

\* communicate with host



This is foundational Linux container networking.



\---



\# 29. Next Planned Networking Phase



Next concepts to learn:



| Concept             | Purpose                |

| ------------------- | ---------------------- |

| Linux bridge        | virtual switch         |

| multiple containers | shared virtual LAN     |

| NAT                 | internet access        |

| iptables            | packet forwarding      |

| routing             | external communication |



This will allow:



\# real internet-connected containers



similar to:



\* Docker

\* Kubernetes pods

\* container orchestration systems.



## Container Networking (Phase 2: Linux Bridge Networking):-

\# Notes — Container Networking (Phase 2: Linux Bridge Networking)



This phase expanded networking from:



\# point-to-point namespace communication



to:



\# scalable multi-container virtual LAN



This is the networking model used conceptually by:



\* Docker bridge networking

\* container runtimes

\* Linux virtual switching



\---



\# 1. Problem With Direct Veth Connections



Earlier architecture:



```text id="bnote01"

host ↔ namespace

```



using:



\# one veth pair



worked for:



\* single container

\* point-to-point communication



But this does NOT scale.



\---



\# 2. Scaling Problem



Suppose there are:



```text id="bnote02"

container1

container2

container3

```



Without bridge:

every container would need direct connection with every other container.



This creates:



```text id="bnote03"

N² networking complexity

```



Very inefficient.



\---



\# 3. Linux Solution



\# Bridge Device



Linux provides:



\# bridge



which behaves like:



\# virtual Ethernet switch



This is one of the MOST important Linux networking components.



\---



\# 4. Important Bridge Insight



A bridge:



\* learns MAC addresses

\* forwards Ethernet frames

\* connects interfaces together



It behaves similarly to:



\* physical switch

\* Ethernet switch fabric



\---



\# 5. New Architecture Built



Final architecture:



```text id="bnote04"

                 br0

               /     \\

              /       \\

     veth1-host    veth2-host

          |              |

          |              |

       namespace1    namespace2

```



This created:



\# virtual container LAN



inside Linux kernel.



\---



\# 6. Cleanup Of Previous Networking



Old setup removed:



```bash id="bnote05"

sudo ip netns delete ns1

sudo ip link delete veth-host

```



Purpose:



\# avoid stale interfaces and namespaces



\---



\# 7. Created Two Network Namespaces



Commands:



```bash id="bnote06"

sudo ip netns add ns1

sudo ip netns add ns2

```



Result:



\# two isolated networking environments



Each namespace had:



\* separate interfaces

\* separate routing

\* separate loopback



\---



\# 8. Created Linux Bridge



Command:



```bash id="bnote07"

sudo ip link add br0 type bridge

```



This created:



\# virtual Layer-2 switch



inside host namespace.



\---



\# 9. Assigned IP To Bridge



Command:



```bash id="bnote08"

sudo ip addr add 10.0.0.1/24 dev br0

```



Important realization:



\# bridge itself can own IP address



Similar to:



\* router interface

\* switch management interface



\---



\# 10. Brought Bridge UP



Command:



```bash id="bnote09"

sudo ip link set br0 up

```



Important lesson:



\# interfaces exist but start DOWN



Bridge must be activated explicitly.



\---



\# 11. Created First Veth Pair



Command:



```bash id="bnote10"

sudo ip link add veth1-host type veth peer name veth1-cont

```



This created:



```text id="bnote11"

veth1-host <====> veth1-cont

```



\---



\# 12. Moved One Side Into Namespace



Command:



```bash id="bnote12"

sudo ip link set veth1-cont netns ns1

```



This transferred:



\# interface ownership



to:



```text id="bnote13"

ns1

```



\---



\# 13. Attached Host Side To Bridge



Command:



```bash id="bnote14"

sudo ip link set veth1-host master br0

```



This was a MASSIVE networking concept.



Linux connected:



\# virtual NIC → virtual switch



Exactly like:



\* plugging cable into physical switch



\---



\# 14. Brought Host Side UP



Command:



```bash id="bnote15"

sudo ip link set veth1-host up

```



Required because:



\# interfaces start DOWN



by default.



\---



\# 15. Configured First Namespace



Entered namespace:



```bash id="bnote16"

sudo ip netns exec ns1 bash

```



Configured interface:



```bash id="bnote17"

ip addr add 10.0.0.2/24 dev veth1-cont

ip link set veth1-cont up

ip link set lo up

```



Result:



\# namespace received independent IP address



\---



\# 16. Successful Namespace ↔ Bridge Ping



Command:



```bash id="bnote18"

ping 10.0.0.1

```



Output:



```text id="bnote19"

64 bytes from 10.0.0.1

```



This proved:



\# container ↔ bridge communication works



\---



\# 17. Created Second Veth Pair



Command:



```bash id="bnote20"

sudo ip link add veth2-host type veth peer name veth2-cont

```



\---



\# 18. Moved Second Interface Into Namespace



Command:



```bash id="bnote21"

sudo ip link set veth2-cont netns ns2

```



\---



\# 19. Attached Second Interface To Bridge



Commands:



```bash id="bnote22"

sudo ip link set veth2-host master br0

sudo ip link set veth2-host up

```



Now both namespaces connected to:



\# same bridge



\---



\# 20. Configured Second Namespace



Entered namespace:



```bash id="bnote23"

sudo ip netns exec ns2 bash

```



Configured interface:



```bash id="bnote24"

ip addr add 10.0.0.3/24 dev veth2-cont

ip link set veth2-cont up

ip link set lo up

```



\---



\# 21. Successful Container-to-Container Communication



Inside ns2:



```bash id="bnote25"

ping 10.0.0.2

```



Output:



```text id="bnote26"

64 bytes from 10.0.0.2

```



This proved:



\# bridge forwarding works



between namespaces.



\---



\# 22. Actual Packet Journey



Packet flow:



```text id="bnote27"

ns2 process

    ↓

veth2-cont

    ↓

veth2-host

    ↓

br0 switch lookup

    ↓

veth1-host

    ↓

veth1-cont

    ↓

ns1 process

```



This is EXACTLY how:



\# Docker bridge networking fundamentally works



\---



\# 23. Massive Linux Networking Insight



Linux bridge dynamically:



\# learns MAC addresses



similar to:



\# physical Ethernet switches



When packets arrive:



\* source MAC learned

\* forwarding table updated

\* future packets forwarded intelligently



\---



\# 24. Important Observation



No manual configuration was needed for:



\* MAC tables

\* switch forwarding logic

\* Ethernet learning



Linux kernel bridge subsystem handled:



\# all switching logic automatically



\---



\# 25. Docker Architecture Connection



Docker default bridge:



```text id="bnote28"

docker0

```



is literally:



\# Linux bridge device



Each container:



\* gets one veth pair

\* host side attached to bridge

\* container side moved into namespace



This phase manually recreated:



\# Docker bridge networking architecture



\---



\# 26. Major Networking Concepts Learned



| Concept                       | Understanding             |

| ----------------------------- | ------------------------- |

| Linux bridge                  | virtual Ethernet switch   |

| bridge membership             | attaching interfaces      |

| switched networking           | Layer-2 forwarding        |

| scalable container networking | multi-container LAN       |

| bridge forwarding             | automatic MAC learning    |

| virtual switch behavior       | kernel Ethernet switching |



\---



\# 27. Major Conceptual Upgrade



Previous networking:



\# point-to-point networking



Current networking:



\# switched Ethernet networking



This was a very major conceptual leap.



\---



\# 28. Current Networking Progress



| Feature                             | Status |

| ----------------------------------- | ------ |

| network namespaces                  | ✅      |

| loopback isolation                  | ✅      |

| veth pairs                          | ✅      |

| namespace interface ownership       | ✅      |

| host ↔ namespace communication      | ✅      |

| Linux bridge                        | ✅      |

| multi-container LAN                 | ✅      |

| namespace ↔ namespace communication | ✅      |

| Layer-2 switching                   | ✅      |



\---



\# 29. Major Milestone Achieved



At this stage:



\# multiple isolated namespaces can communicate



through:



\# virtual switched network



This is foundational architecture behind:



\* Docker bridge networking

\* Kubernetes pod networking

\* container communication systems



\---



\# 30. Next Networking Phase



Next concepts to learn:



| Concept               | Purpose               |

| --------------------- | --------------------- |

| default gateway       | external routing      |

| IP forwarding         | packet forwarding     |

| NAT                   | internet access       |

| iptables masquerade   | outbound translation  |

| internet connectivity | real-world networking |



This will allow containers to:



\# access external internet



exactly like real Docker containers.



## Container Networking (Phase 3: Internet Access, Routing, NAT):-

\# Notes — Container Networking (Phase 3: Internet Access, Routing, NAT)



This phase transformed networking from:



\# isolated container LAN



into:



\# fully internet-connected container networking



This is the architecture conceptually used by:



\* Docker bridge networking

\* containerd bridge mode

\* Linux container internet routing



\---



\# 1. Networking Status Before This Phase



At the end of previous phase:



| Capability                          | Status |

| ----------------------------------- | ------ |

| container ↔ host communication      | ✅      |

| container ↔ container communication | ✅      |

| Linux bridge                        | ✅      |

| shared subnet                       | ✅      |



BUT:



\# containers still had NO internet access



\---



\# 2. Why Internet Did NOT Work Initially



Containers used private subnet:



```text id="n3\_01"

10.0.0.0/24

```



These are:



\# private IP addresses



Internet routers NEVER route:



\* 10.x.x.x

\* 192.168.x.x

\* 172.16.x.x



publicly.



So packets from:



```text id="n3\_02"

10.0.0.2

```



would never receive valid replies from internet.



\---



\# 3. Core Problem



Packet path initially:



```text id="n3\_03"

container

    ↓

bridge

    ↓

host

    ↓

???

```



Host kernel did NOT:



\* forward packets automatically

\* NAT packets automatically

\* route private subnet externally



Therefore:



\# internet connectivity failed



\---



\# 4. Linux Solution



\# Router + NAT



Host machine must become:



| Role        | Purpose            |

| ----------- | ------------------ |

| router      | forward packets    |

| NAT gateway | rewrite source IPs |



This is EXACTLY how:



\* home WiFi routers

\* Docker bridge networking

\* cloud NAT gateways



work internally.



\---



\# 5. Major Networking Concepts Learned



| Concept         | Purpose                |

| --------------- | ---------------------- |

| default gateway | route unknown traffic  |

| IP forwarding   | Linux router mode      |

| NAT             | source IP translation  |

| MASQUERADE      | dynamic NAT            |

| FORWARD rules   | allow packet traversal |



\---



\# 6. Existing Network Architecture



Current setup before internet access:



```text id="n3\_04"

                 br0 (10.0.0.1)

               /               \\

              /                 \\

     veth1-host              veth2-host

          |                        |

     veth1-cont               veth2-cont

          |                        |

        ns1                      ns2

     (10.0.0.2)              (10.0.0.3)

```



This already provided:



\# virtual switched LAN



\---



\# 7. Default Gateway Concept



Containers knew only:



```text id="n3\_05"

10.0.0.0/24

```



They did NOT know:



\# where internet exists



So default route had to be added.



\---



\# 8. Added Default Route Inside Namespace



Entered namespace:



```bash id="n3\_06"

sudo ip netns exec ns1 bash

```



Added route:



```bash id="n3\_07"

ip route add default via 10.0.0.1

```



Meaning:



```text id="n3\_08"

"Send unknown traffic to bridge gateway"

```



This is EXACTLY like:



\* home router default gateway

\* cloud VM gateway



\---



\# 9. Verified Routing Table



Command:



```bash id="n3\_09"

ip route

```



Output:



```text id="n3\_10"

default via 10.0.0.1 dev veth1-cont

10.0.0.0/24 dev veth1-cont

```



Meaning:



| Route                | Purpose                  |

| -------------------- | ------------------------ |

| default via 10.0.0.1 | internet/default traffic |

| 10.0.0.0/24          | local subnet             |



\---



\# 10. Massive Networking Insight



The namespace now believed:



```text id="n3\_11"

10.0.0.1

```



was:



\# router/gateway



Exactly like real networking systems.



\---



\# 11. Host Routing Table Verification



Host command:



```bash id="n3\_12"

ip route

```



Output:



```text id="n3\_13"

default via 172.28.32.1 dev eth0

10.0.0.0/24 dev br0

172.28.32.0/20 dev eth0

```



Important discovery:



\# internet-facing interface = eth0



This interface became:



\# NAT outbound interface



\---



\# 12. First Major Issue



\# `iptables` Missing



Attempted:



```bash id="n3\_14"

sudo iptables ...

```



Error:



```text id="n3\_15"

sudo: iptables: command not found

```



\---



\# 13. Root Cause Of Issue



Modern Ubuntu often:



\* uses nftables backend

\* does NOT install iptables package by default



Important modern Linux networking insight:



\# iptables → nftables transition



\---



\# 14. Fix For Missing iptables



Installed package:



```bash id="n3\_16"

sudo apt update

sudo apt install iptables

```



Verified:



```bash id="n3\_17"

iptables --version

```



Output similar to:



```text id="n3\_18"

iptables v1.x.x (nf\_tables)

```



This is completely normal on modern Linux.



\---



\# 15. Enabling IP Forwarding



Command:



```bash id="n3\_19"

sudo sysctl -w net.ipv4.ip\_forward=1

```



Purpose:



\# allow Linux to behave like router



Without this:

Linux behaves only like:



\# endpoint host



\---



\# 16. Verified IP Forwarding



Command:



```bash id="n3\_20"

cat /proc/sys/net/ipv4/ip\_forward

```



Expected output:



```text id="n3\_21"

1

```



Meaning:



\# packet forwarding enabled



\---



\# 17. NAT / MASQUERADE Rule



Command:



```bash id="n3\_22"

sudo iptables -t nat -A POSTROUTING \\

    -s 10.0.0.0/24 \\

    -o eth0 \\

    -j MASQUERADE

```



This was one of the MOST important networking commands learned.



\---



\# 18. What MASQUERADE Does



Before NAT:



```text id="n3\_23"

SRC = 10.0.0.2

DST = 8.8.8.8

```



After NAT:



```text id="n3\_24"

SRC = 172.28.39.19

DST = 8.8.8.8

```



Meaning:



\# host rewrites container source IP



before sending packet to internet.



\---



\# 19. Massive Networking Insight



Host machine became:



\# NAT gateway



for containers.



Exactly like:



\* home router

\* cloud NAT gateway

\* Docker host networking



\---



\# 20. Forwarding Rule 1



Command:



```bash id="n3\_25"

sudo iptables -A FORWARD -i br0 -o eth0 -j ACCEPT

```



Purpose:



```text id="n3\_26"

allow container → internet traffic

```



\---



\# 21. Forwarding Rule 2



Command:



```bash id="n3\_27"

sudo iptables -A FORWARD \\

    -i eth0 \\

    -o br0 \\

    -m state \\

    --state RELATED,ESTABLISHED \\

    -j ACCEPT

```



Purpose:



```text id="n3\_28"

allow internet replies → containers

```



\---



\# 22. Final Internet Connectivity Test



Entered namespace:



```bash id="n3\_29"

sudo ip netns exec ns1 bash

```



Tested:



```bash id="n3\_30"

ping 8.8.8.8

```



Output:



```text id="n3\_31"

64 bytes from 8.8.8.8

```



This proved:



\# real internet connectivity exists inside namespace



\---



\# 23. Actual Packet Journey



Complete internet packet path:



```text id="n3\_32"

container process

    ↓

veth1-cont

    ↓

veth1-host

    ↓

Linux bridge br0

    ↓

host routing table

    ↓

iptables MASQUERADE

    ↓

eth0

    ↓

internet

    ↓

reply returns

    ↓

reverse NAT translation

    ↓

container receives reply

```



This is EXACTLY how:



\# Docker bridge NAT networking fundamentally works



\---



\# 24. Massive Docker Insight



Docker bridge mode internally performs:



\* bridge creation

\* veth setup

\* namespace connection

\* IP assignment

\* routing

\* iptables NAT

\* forwarding rules



This phase manually recreated:



\# Docker bridge + NAT internet architecture



from first principles.



\---



\# 25. Important Networking Concepts Learned



| Concept         | Understanding                 |

| --------------- | ----------------------------- |

| default gateway | route unknown traffic         |

| IP forwarding   | Linux router mode             |

| NAT             | private→public IP translation |

| MASQUERADE      | dynamic outbound NAT          |

| FORWARD chain   | packet traversal permission   |

| bridge routing  | container external networking |



\---



\# 26. Major Systems Insight



Containers do NOT magically get internet.



Internet works because:



\# host Linux kernel acts as router + NAT gateway



This is one of the MOST important Linux container networking understandings.



\---



\# 27. Important Difference Learned



| Local Bridge Networking | Internet Networking |

| ----------------------- | ------------------- |

| Layer-2 switching       | Layer-3 routing     |

| MAC forwarding          | IP routing          |

| bridge learning         | NAT translation     |

| same subnet             | external internet   |



This phase introduced:



\# actual packet routing architecture



\---



\# 28. Current Networking Status



| Feature                             | Status |

| ----------------------------------- | ------ |

| network namespaces                  | ✅      |

| veth pairs                          | ✅      |

| Linux bridge                        | ✅      |

| multi-container LAN                 | ✅      |

| container ↔ container communication | ✅      |

| default routing                     | ✅      |

| IP forwarding                       | ✅      |

| NAT/MASQUERADE                      | ✅      |

| internet-connected namespaces       | ✅      |



\---



\# 29. Major Milestone Achieved



At this stage:



\# fully internet-connected container networking exists



The namespace can now:



\* access internet

\* behave like real server

\* download packages

\* communicate externally



This is extremely close conceptually to:



\* Docker bridge networking

\* Linux container networking

\* Kubernetes node-level packet flow



\---



\# 30. Biggest Conceptual Realization



Docker networking is NOT magic.



Docker fundamentally uses:



\* namespaces

\* veth pairs

\* Linux bridges

\* routing

\* iptables NAT



You manually recreated:



\# real Linux container internet networking architecture



from scratch.



## Runtime Networking Integration Phase (myruntime):-

\# Notes — Runtime Networking Integration Phase (`myruntime`)



This phase was the transition from:



\# manual networking experiments



to:



\# runtime-managed container networking



This was one of the BIGGEST architectural shifts in the project.



\---



\# 1. Starting Point Before Integration



Before networking integration:



`myruntime` already supported:



| Feature         | Status |

| --------------- | ------ |

| PID namespace   | ✅      |

| UTS namespace   | ✅      |

| Mount namespace | ✅      |

| pivot\_root      | ✅      |

| proc mount      | ✅      |

| capabilities    | ✅      |

| no\_new\_privs    | ✅      |



But:



\# networking was NOT integrated



Container only had:



```text id="rn01"

lo

```



and no connectivity.



\---



\# 2. First Networking Integration Step



\# Add `CLONE\_NEWNET`



Original flags:



```c id="rn02"

CLONE\_NEWUTS |

CLONE\_NEWPID |

CLONE\_NEWNS

```



Updated to:



```c id="rn03"

CLONE\_NEWUTS |

CLONE\_NEWPID |

CLONE\_NEWNS |

CLONE\_NEWNET

```



\---



\# Result



Container now received:



\# isolated network namespace



Inside container:



```bash id="rn04"

/bin/ip addr

```



Output:



```text id="rn05"

1: lo: ...

```



Only loopback existed.



This proved:



\# network namespace isolation works



\---



\# 3. Important Observation



\# Loopback Was DOWN



Inside container:



```text id="rn06"

lo

```



was NOT UP initially.



This matched earlier manual namespace experiments.



\---



\# Fix



Inside container:



```bash id="rn07"

/bin/ip link set lo up

```



\---



\# Important Insight



Every new network namespace starts with:



\# independent loopback interface



which must be manually activated.



\---



\# 4. Capability Security Observation



Running:



```bash id="rn08"

ping 127.0.0.1

```



failed:



```text id="rn09"

permission denied

```



even though:



```bash id="rn10"

id

```



returned:



```text id="rn11"

uid=0(root)

```



\---



\# Root Cause



`drop\_capabilities()` removed:



\# CAP\_NET\_RAW



`ping` requires raw ICMP sockets.



\---



\# Major Security Insight Learned



```text id="rn12"

root != all-powerful

```



inside containers.



Capabilities and UID are separate.



\---



\# 5. Runtime Networking Architecture Shift



Earlier networking used:



```bash id="rn13"

ip netns exec ns1

```



But runtime-created namespaces are:



\# PID-based



So networking now had to target:



\# container process PID



\---



\# 6. First Runtime Networking Function



\# `setup\_veth()`



Runtime parent process began creating:



```text id="rn14"

veth-host <--> veth-cont

```



using:



```bash id="rn15"

ip link add veth-host type veth peer name veth-cont

```



\---



\# 7. Moving Interface Into Namespace



Parent runtime moved container interface using:



```bash id="rn16"

ip link set veth-cont netns <pid>

```



This was a MASSIVE runtime milestone.



\---



\# Important Runtime Insight



Real runtimes identify namespaces using:



\# container init PID



\---



\# 8. First Synchronization Problem



Initial error:



```text id="rn17"

ip: can't find device 'veth-cont'

```



\---



\# Root Cause



Child process configured interface BEFORE parent moved it.



Timeline:



```text id="rn18"

Child starts

    ↓

tries configuring veth-cont

    ↓

Parent still moving interface

```



\---



\# Temporary Fix



Added:



```c id="rn19"

sleep(1);

```



before interface configuration.



\---



\# Important Runtime Engineering Lesson



This introduced:



\# parent-child synchronization problems



Real runtimes later solve this using:



\* pipes

\* eventfds

\* socketpairs



\---



\# 9. Container-Side Interface Configuration



Inside container:



```bash id="rn20"

/bin/ip addr add 10.0.0.2/24 dev veth-cont

```



and:



```bash id="rn21"

/bin/ip link set veth-cont up

```



\---



\# Result



Container finally showed:



```text id="rn22"

veth-cont

10.0.0.2/24

```



\---



\# 10. Host-Side Interface Configuration



Parent runtime configured:



```bash id="rn23"

ip addr add 10.0.0.1/24 dev veth-host

```



and:



```bash id="rn24"

ip link set veth-host up

```



\---



\# Result



Interface state changed from:



```text id="rn25"

NO-CARRIER

```



to:



```text id="rn26"

LOWER\_UP

```



Meaning:



\# virtual ethernet link became operational



\---



\# 11. First Runtime Networking Architecture



At this stage runtime created:



```text id="rn27"

Host

  |

veth-host

  |

veth-cont

  |

Container

```



This provided:



\# point-to-point runtime networking



\---



\# 12. Transition To Bridge Networking



Point-to-point networking did not scale.



Architecture evolved into:



```text id="rn28"

             br0

               |

         veth-host

               |

\--------------------------------

         container

\--------------------------------

```



Exactly like:



\# Docker bridge model



\---



\# 13. Bridge Setup Function



\# `setup\_bridge()`



Runtime automatically created:



```bash id="rn29"

ip link add br0 type bridge

```



Assigned:



```bash id="rn30"

ip addr add 10.0.0.1/24 dev br0

```



and enabled:



```bash id="rn31"

ip link set br0 up

```



\---



\# Important Runtime Improvement



Bridge reuse support added using:



```bash id="rn32"

ip link show br0

```



checks.



\---



\# 14. Attaching Interface To Bridge



Parent runtime executed:



```bash id="rn33"

ip link set veth-host master br0

```



\---



\# Important Networking Insight



This was equivalent to:



\# plugging NIC into virtual switch



\---



\# 15. Runtime Routing Integration



Container automatically received:



```bash id="rn34"

/bin/ip route add default via 10.0.0.1

```



\---



\# Result



Container routing table:



```text id="rn35"

default via 10.0.0.1

10.0.0.0/24 dev veth-cont

```



This matched real networking architecture.



\---



\# 16. Host ↔ Container Connectivity Verification



Testing:



```bash id="rn36"

/bin/busybox wget http://10.0.0.1

```



returned:



```text id="rn37"

Connection refused

```



\---



\# Important Insight



```text id="rn38"

Connection refused

```



means:



\# routing works



because:



```text id="rn39"

Network unreachable

```



did NOT appear.



\---



\# 17. NAT Integration



\# `setup\_nat()`



Runtime automatically enabled:



```bash id="rn40"

sysctl -w net.ipv4.ip\_forward=1

```



\---



\# NAT Rule



```bash id="rn41"

iptables -t nat -A POSTROUTING \\

\-s 10.0.0.0/24 \\

\-o eth0 \\

\-j MASQUERADE

```



\---



\# Forwarding Rules



```bash id="rn42"

iptables -A FORWARD -i br0 -o eth0 -j ACCEPT

```



and:



```bash id="rn43"

iptables -A FORWARD \\

\-i eth0 -o br0 \\

\-m state \\

\--state RELATED,ESTABLISHED \\

\-j ACCEPT

```



\---



\# Important Runtime Improvement



Used:



```bash id="rn44"

iptables -C

```



before:



```bash id="rn45"

iptables -A

```



to avoid duplicate rules.



\---



\# 18. Internet Connectivity Success



Testing direct IP:



```bash id="rn46"

/bin/busybox wget http://93.184.216.34

```



showed:



```text id="rn47"

Connecting to 93.184.216.34

```



\---



\# Important Insight



This proved:



\# internet routing + NAT worked



even before DNS worked.



\---



\# 19. DNS Failure



Initial DNS errors:



```text id="rn48"

nslookup: write to '127.0.0.1'

```



\---



\# Root Cause



Container lacked:



```text id="rn49"

/etc/resolv.conf

```



\---



\# Important Networking Insight



Internet access requires:



\* routing/NAT

\* DNS separately



\---



\# 20. DNS Integration



Runtime created:



```text id="rn50"

/etc/resolv.conf

```



inside container.



Code:



```c id="rn51"

mkdir("/etc", 0755);



FILE \*resolv = fopen("/etc/resolv.conf", "w");



fprintf(resolv, "nameserver 8.8.8.8\\n");

```



\---



\# Important Filesystem Issue



Initially failed because:



```text id="rn52"

/etc

```



directory did not exist.



Learned:



\# fopen() cannot create parent directories



\---



\# 21. Final DNS Success



Testing:



```bash id="rn53"

/bin/busybox nslookup google.com

```



returned:



```text id="rn54"

Server: 8.8.8.8

Address: 8.8.8.8:53



google.com -> IPs

```



\---



\# MASSIVE Final Achievement



Runtime now automatically supports:



```text id="rn55"

container

    ↓

veth

    ↓

bridge

    ↓

routing

    ↓

NAT

    ↓

internet

    ↓

DNS

```



completely automatically.



\---



\# 22. Cleanup Problems Discovered



Frequent errors:



```text id="rn56"

RTNETLINK answers: File exists

```



\---



\# Root Cause



Failed runs left stale:



```text id="rn57"

veth-host

```



interfaces behind.



\---



\# Important Runtime Engineering Lesson



Real runtimes require:



\# cleanup infrastructure



for:



\* failed networking

\* leaked interfaces

\* zombie processes

\* failed mounts



\---



\# 23. Runtime Architecture At End Of Phase



Final runtime flow:



```text id="rn58"

setup bridge

    ↓

setup NAT

    ↓

clone()

    ↓

create namespaces

    ↓

create veth pair

    ↓

move interface into namespace

    ↓

assign IP

    ↓

setup routes

    ↓

setup DNS

    ↓

drop capabilities

    ↓

launch shell

```



\---



\# 24. Major Runtime Concepts Learned



| Concept                  | Understanding                |

| ------------------------ | ---------------------------- |

| PID-based namespaces     | runtime networking targeting |

| veth orchestration       | runtime-created NICs         |

| bridge networking        | virtual switch architecture  |

| runtime routing          | automatic gateway setup      |

| NAT integration          | internet connectivity        |

| DNS integration          | hostname resolution          |

| synchronization problems | parent-child coordination    |

| cleanup problems         | lifecycle engineering        |



\---



\# 25. Biggest Insight Of Entire Phase



This phase transformed project from:



```text id="rn59"

Linux syscall experiments

```



into:



```text id="rn60"

actual runtime orchestration engineering

```



This was the BIGGEST architectural shift so far.



## Runtime Lifecycle \& Cleanup Phase:-

\# Notes — Runtime Lifecycle \& Cleanup Phase



This phase focused on transforming the runtime from:



```text id="lcf01"

container launcher

```



into:



```text id="lcf02"

container lifecycle manager

```



The goal was:



\# stable startup + stable teardown



\---



\# 1. Why Cleanup Became Necessary



As networking became integrated,

runtime started creating real kernel resources:



| Resource           | Created By Runtime |

| ------------------ | ------------------ |

| veth interfaces    | yes                |

| bridge attachments | yes                |

| network namespaces | yes                |

| proc mounts        | yes                |

| child processes    | yes                |



\---



\# Problem



When runtime crashed or exited incorrectly:



```text id="lcf03"

resources remained alive

```



Examples seen:



```text id="lcf04"

RTNETLINK answers: File exists

```



\---



\# Root Cause



Old:



```text id="lcf05"

veth-host

```



interfaces survived previous runs.



\---



\# Major Runtime Insight



Real runtimes must manage:



\# full lifecycle



not just:



\# container startup



\---



\# 2. Runtime Lifecycle Model



Learned runtime lifecycle flow:



```text id="lcf06"

runtime start

    ↓

cleanup stale leftovers

    ↓

setup networking

    ↓

clone container

    ↓

container execution

    ↓

waitpid()

    ↓

teardown resources

```



This became the first:



\# complete orchestration cycle



\---



\# 3. First Cleanup Function



\# `cleanup\_network()`



Added:



```c id="lcf07"

void cleanup\_network()

{

    system("ip link delete veth-host 2>/dev/null");

}

```



\---



\# Important Linux Networking Insight



Deleting ONE side of veth:



```text id="lcf08"

veth-host

```



automatically deletes:



```text id="lcf09"

veth-cont

```



because:



\# veth pair is linked kernel object



\---



\# 4. Defensive Cleanup Before Startup



Inside:



```c id="lcf10"

setup\_veth()

```



BEFORE:



```bash id="lcf11"

ip link add ...

```



runtime now performs:



```c id="lcf12"

cleanup\_network();

```



\---



\# Why?



This prevents stale-interface collisions from:



\* crashes

\* failed runs

\* abrupt termination



\---



\# Result



Previous errors:



```text id="lcf13"

RTNETLINK answers: File exists

```



disappeared.



\---



\# 5. Cleanup After Container Exit



Original code:



```c id="lcf14"

waitpid(pid, NULL, 0);

```



Updated to:



```c id="lcf15"

waitpid(pid, \&status, 0);



cleanup\_network();

```



\---



\# Result



After container exit:



```bash id="lcf16"

ip link

```



no longer showed:



\* `veth-host`

\* `veth-cont`



\---



\# Important Runtime Insight



This was the first:



\# successful runtime teardown



\---



\# 6. Process Reaping



\# `waitpid()`



Runtime began properly collecting child process status:



```c id="lcf17"

int status;



waitpid(pid, \&status, 0);

```



\---



\# Important Systems Insight



Without:



```text id="lcf18"

waitpid()

```



child becomes:



\# zombie process



\---



\# Learned Earlier



Zombie:



\* process exited

\* BUT kernel metadata still exists

\* until parent reaps it



\---



\# 7. Exit Status Handling



Added:



```c id="lcf19"

WIFEXITED(status)

WEXITSTATUS(status)

```



\---



\# Result



Runtime now prints:



```text id="lcf20"

Exit status: 0

```



\---



\# Important Runtime Improvement



Runtime gained:



\# observability



\---



\# 8. Bridge Persistence



After cleanup:



```bash id="lcf21"

ip link

```



still showed:



```text id="lcf22"

br0

```



\---



\# Important Insight



Bridge should:



\# persist across containers



Exactly like:



```text id="lcf23"

docker0

```



in Docker.



\---



\# 9. Understanding `NO-CARRIER`



After cleanup:



```text id="lcf24"

br0: NO-CARRIER

```



appeared.



\---



\# Meaning



Bridge currently has:



\# no active interfaces attached



because runtime cleaned:



```text id="lcf25"

veth-host

```



This was determined to be:



\# correct behavior



\---



\# 10. Regression Bug Encountered



A later rewritten cleanup version accidentally removed:



\* bridge setup

\* NAT

\* DNS

\* veth movement

\* route setup



\---



\# Result



Container showed only:



```text id="lcf26"

lo

```



Networking completely failed.



\---



\# Important Engineering Lesson



This introduced:



\# regression bugs



New subsystem:



```text id="lcf27"

cleanup

```



accidentally broke:



```text id="lcf28"

network orchestration

```



\---



\# Major Lesson Learned



In systems programming:



\# never rewrite stable subsystems unnecessarily



Instead:



\# incrementally layer improvements



\---



\# 11. Correct Incremental Engineering Approach



Networking code restored.



Then ONLY minimal cleanup additions were added:



| Addition               | Purpose            |

| ---------------------- | ------------------ |

| `cleanup\_network()`    | stale veth removal |

| cleanup before startup | defensive setup    |

| cleanup after exit     | teardown           |

| waitpid status         | zombie prevention  |



WITHOUT touching:



\* bridge

\* NAT

\* DNS

\* routes

\* networking logic



\---



\# 12. Final Stable Runtime Lifecycle Version



Final runtime successfully supported:



| Feature               | Status |

| --------------------- | ------ |

| startup orchestration | ✅      |

| namespace creation    | ✅      |

| bridge networking     | ✅      |

| NAT                   | ✅      |

| DNS                   | ✅      |

| internet access       | ✅      |

| process isolation     | ✅      |

| cleanup               | ✅      |

| stale veth prevention | ✅      |

| zombie reaping        | ✅      |



\---



\# 13. Final Verification Results



Inside container:



```bash id="lcf29"

ip addr

```



showed:



\* `lo`

\* `veth-cont`

\* `10.0.0.2`



\---



\# Routing



```bash id="lcf30"

ip route

```



showed:



```text id="lcf31"

default via 10.0.0.1

```



\---



\# DNS



```bash id="lcf32"

nslookup google.com

```



successfully resolved real IPs.



\---



\# Internet



```bash id="lcf33"

wget http://93.184.216.34

```



successfully connected externally.



\---



\# Cleanup Verification



After exit:



```bash id="lcf34"

ip link

```



showed:



\* `br0`

\* NO `veth-host`

\* NO `veth-cont`



This proved:



\# lifecycle cleanup fully worked



\---



\# 14. Major Runtime Concepts Learned



| Concept                 | Understanding            |

| ----------------------- | ------------------------ |

| lifecycle orchestration | startup + teardown       |

| stale resource cleanup  | defensive runtime design |

| zombie reaping          | waitpid importance       |

| teardown sequencing     | cleanup ordering         |

| bridge persistence      | reusable infrastructure  |

| regression bugs         | subsystem interaction    |

| incremental engineering | safe runtime evolution   |



\---



\# 15. Biggest Insight Of This Phase



This phase transformed runtime from:



```text id="lcf35"

container launcher

```



into:



```text id="lcf36"

stable lifecycle-managed container runtime

```



This was the final major architectural milestone of the project.





##  Why I Stopped Here:-



At this stage, the runtime was able to create isolated containers with:



\* namespaces

\* filesystem isolation

\* basic networking

\* NAT and DNS

\* lifecycle cleanup



While trying to move further toward:



\* dynamic container management

\* multiple simultaneous containers

\* runtime metadata handling



the complexity increased much faster than I initially expected.



\---



\# Problems I Faced



The next steps introduced issues that were very different from the earlier Linux isolation concepts.



I started encountering problems related to:



\* terminal/session management

\* multiple interactive shells sharing the same TTY

\* process orchestration

\* runtime state tracking

\* synchronization between containers

\* increasing architectural complexity



For example, when trying to run multiple containers simultaneously, all of them attempted to interact with the same terminal session, which caused the runtime to become difficult to control and debug.



At that point I realized that container runtimes involve many additional layers beyond namespaces and networking.



\---



\# What I Learned



One important thing I learned during this phase was that:



```text id="note01"

complexity grows very quickly in systems programming

```



A small feature often affects:



\* process management

\* networking

\* cleanup logic

\* terminal handling

\* runtime state



all at the same time.



I also learned that rewriting stable code too aggressively can easily introduce regressions and break previously working subsystems.



\---



\# Why I Decided To Pause



I felt that I had reached a point where:



\* I understood the core Linux container mechanisms reasonably well

\* but I did not yet have enough knowledge to confidently continue building more advanced orchestration features



Instead of continuing to add unstable features, I decided to stop at the last stable version and properly document the work completed so far.



\---



\# Future Intention



I may continue this project in the future after learning more about:



\* PTYs and terminal handling

\* cgroups

\* OCI runtimes

\* daemon architectures

\* process orchestration



The project gave me a much better understanding of how much engineering complexity exists behind modern container runtimes.



