# INF584 project - implementation of a research paper

## Description

This project was realized as part of the course INF584 - Image Synthesis at Ecole Polytechnique.
In this project, I implemented the Horizon-Based Ambient Occlusion paper _Image-Space Horizon-Based Ambient Occlusion_ by Louis Bavoil, Miguel Sainz and Rouslan Dimitrov for the NVIDIA Corporation.

## Installation

The details about how to install the code are given in the README.md file in the code folder. The `<path-to-MyRenderer-directory>` should be the path to the `code` folder of this repository.

I have observed that the ambient occlusion display did not work when my computer ran the program on the integrated GPU (Intel i7) but worked on my NVIDIA GPU. I did not attempt to solve this problem as it was not the goal of the project.

## Running and using the project

The details about how to install the code are given in the README.md file in the code folder. The `<path-to-MyRenderer-directory>` should be the path to the `code` folder of this repository.

Once the project is launched, here are the possible actions:
Mouse commands:

-  Left button: rotate camera
-  Middle button: zoom
-  Right button: pan camera

Keyboard commands:

-  ESC: quit the program
-  H: print the helper that also displays this information
-  F12: reload GPU shaders
-  F: decrease field of view
-  G: increase field of view
-  TAB: switch between rasterization, ray tracing, Zbuffer, normal display, Horizon-Based Ambient Occlusion (HBAO) and HBAO with rasterization
-  S: switch between rasterization and rasterization + HBAO
-  SPACE: execute ray tracing (to avoid unless very low poly mesh).

The image that is displayed when launching the program is the result of the rasterization.
The console displays a message when changing the display mode, indicating what is the current display mode.

## Results

The program yields the foolowing results for the `Resources/Models/denis.off` file:

The rasterized image:

![](/media/denis.png)

The Horizon-Based Ambient Occlusion Result:

![](/media/denis_hbao.png)

The rasterization combined with the Horizon-Based Ambient Occlusion:

![](/media/denis_with.png)

The report for the project is available [here](/media/final_report.pdf).

## Author and aknowledgment

Author: Marie Audouard, on the code base created and provided by Tamy Boubekeur for the INF584 course (Copyright (C) 2022 Tamy Boubekeur. All rights reserved).
