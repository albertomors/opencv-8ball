# opencv-8ball

## Overview
The goal of this project is to develop a computer vision system for analyzing video
footage of “8-Ball” billiard games. This vision system aims to provide detailed,
high-level information about the status of the match, including the positions and
trajectories of the balls. These trajectories will be displayed in real-time through
a 2D top-view minimap, superimposed on the bottom-left corner of each video
frame.

## Authors
Fresco Eleonora, Girardello Sofia, Morselli Alberto \
Date: July 2024

## DEMO

<p align="center">
    <img src="res/track3.jpg" alt="img" width="48%"/>
    <img src="res/game1_clip1.gif" alt="gif" width="48%"/>
</p>

To look for all the output videos used as a test, check `build/output`

## Structure

```
FinalProject/
├── build/                 # Build directory for compiled files
    └── output/            # Directory for output videos
├── res/		           # Directory for resources
    └── Dataset/           # Directory for datasets and sample videos
├── include/               # Header files
├── src/                   # Source code files
├── README.txt             # Project overview 
└── CMakeLists.txt         # Build configuration
```
