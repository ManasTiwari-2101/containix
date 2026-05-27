# CONTAINIX

CONTAINIX is a lightweight Linux container runtime written in C as a systems programming learning project.

The goal of the project was not to build a production-ready container platform, but to understand how container runtimes work internally by implementing core Linux isolation and networking primitives directly using low-level system calls.

The runtime gradually evolved from simple namespace experiments into a working single-container runtime supporting filesystem isolation, virtual networking, NAT, DNS resolution, and basic lifecycle cleanup.

---

# Why This Project Was Built

Modern container platforms hide a large amount of Linux internals behind high-level tooling. While using Docker is straightforward, understanding what actually happens underneath is much more difficult.

This project was built to explore questions such as:

* How does a container get its own process tree?
* How does filesystem isolation actually work?
* How are virtual ethernet devices connected?
* How does NAT allow containers to access the internet?
* Why are cleanup and lifecycle management important?
* What problems appear when trying to scale from one container to many?

The project became a practical way to study Linux namespaces, mounts, networking, and process management through direct implementation.

---

# Features Implemented

Current stable version includes:

* UTS namespace isolation
* PID namespace isolation
* Mount namespace isolation
* Network namespace isolation
* Root filesystem setup using `pivot_root`
* `/proc` filesystem mounting
* Capability dropping
* `PR_SET_NO_NEW_PRIVS` support
* Virtual ethernet pair setup (`veth`)
* Linux bridge networking
* NAT configuration using iptables
* DNS resolution inside the container
* Internet access from inside isolated container
* Basic runtime lifecycle cleanup
* Stale network interface cleanup after container exit

---

# Networking Architecture

The runtime creates a dedicated network namespace for the container and connects it to the host using a veth pair.

The host-side interface is attached to a Linux bridge (`br0`) and outbound traffic is routed through NAT rules configured on the host.

Simplified flow:

```text
Container
   │
veth pair
   │
Linux bridge (br0)
   │
iptables NAT
   │
Host network
   │
Internet
```

The container is able to:

* communicate through the bridge
* resolve DNS queries
* access external internet resources

while remaining isolated inside its own network namespace.

---

# Project Structure

```text
CONTAINIX/
├── README.md
├── .gitignore
├── assets/
├── experiments/
│   ├── bin/
│   ├── rootfs_artifacts/
│   └── src/
├── archive/
│   ├── main_bridge_working.c
│   ├── main_full_networking_working.c
│   ├── main_lifecycle_networking_stable.c
│   └── main_working_v1.c
└── myruntime/
   ├── Makefile
   ├── build/
   ├── docs/
   │   └── document_notes.md
   ├── include/
   │   └── .gitkeep
   ├── rootfs/
   └── src/
      └── main.c
```

* `myruntime/` contains the current stable runtime implementation
* `archive/` contains older runtime snapshots from earlier development stages
* `experiments/` stores smaller isolated tests and supporting artifacts used during development

---

# Building

```bash
cd myruntime
make
```

---

# Running

```bash
cd myruntime
sudo make run
```

---

# Example Runtime Behavior

Inside the container:

```bash
/bin/ip addr
/bin/ip route
/bin/busybox nslookup google.com
```

The runtime configures:

* isolated namespaces
* virtual networking
* routing
* NAT
* DNS

automatically during startup.

---

# Problems Encountered During Development

One of the biggest lessons from this project was realizing how quickly runtime complexity increases.

Early stages involving namespaces and filesystem isolation were manageable, but later stages introduced more difficult problems involving:

* runtime lifecycle management
* cleanup orchestration
* networking teardown
* synchronization
* terminal/session handling
* multi-container orchestration

An attempt to move toward multiple simultaneous interactive containers exposed issues related to shared terminal handling and process orchestration, which showed that container runtimes involve far more than namespace creation alone.

This project was intentionally paused at the last stable single-container version instead of continuing with increasingly unstable architectural changes.

---

# Current Limitations

The runtime is intentionally minimal and has several limitations:

* not OCI compliant
* no cgroup resource isolation
* limited error handling/logging
* hard-coded runtime paths in some places
* minimal root filesystem tooling
* no image management
* no daemon architecture
* no support for multi-container orchestration

The project should be viewed as a learning/runtime exploration project rather than a production runtime.

---

# What This Project Helped Me Learn

Working on this runtime provided practical exposure to:

* Linux namespaces
* low-level process management
* filesystem isolation
* Linux networking internals
* bridge and veth networking
* NAT and routing
* lifecycle cleanup
* runtime architecture tradeoffs
* debugging low-level Linux behavior

It also highlighted how much engineering complexity exists behind modern container platforms.

---

# Future Work

Possible future improvements include:

* cgroups v2 integration
* PTY/terminal management
* improved runtime state management
* configurable networking
* better logging/debugging
* OCI-style runtime structure
* automated integration testing
* image/rootfs tooling

The project may be continued in the future as my systems programming knowledge improves further.

## Additional Notes

Detailed development notes and learning notes from different stages of the project are included inside:

```text
myruntime/docs/document_notes.md
```

These notes contain:

* debugging observations
* networking experiments
* lifecycle cleanup learnings
* namespace behavior
* runtime design decisions
* issues encountered during implementation

The notes were intentionally preserved as part of the learning process instead of only keeping the final code.

---

## Acknowledgement

This project was heavily inspired by:

Lizzie Dixon’s blog:

“Linux containers in 500 LOC”
https://blog.lizzie.io/linux-containers-in-500-loc/

The article was extremely helpful in understanding the early structure of Linux container runtimes and served as an important reference during the initial stages of the project.
