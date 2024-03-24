# INF584 project - implementation of a research paper

## Description

This project was realized as part of the course INF584 - Image Synthesis at Ecole Polytechnique.
In this project, I implemented the Horizon-Based Ambient Occlusion paper _Image-Space Horizon-Based Ambient Occlusion_ by Louis Bavoil, Miguel Sainz and Rouslan Dimitrov for the NVIDIA Corporation.

## Installation

The details about how to install and run the code are given in the README.md file in the code folder.

I have observed that the ambient occlusion display did not work when my computer ran the program on the integrated GPU (Intel i7) but worked on my NVIDIA GPU. I did not attempt to solve this problem as I did not understand its origin.

## Results

The program yields the foolowing results for the denis.off file:

The rasterized image:

![](/media/denis.png)

The Horizon-Based Ambient Occlusion Result:

![](/media/denis_hbao.png)

The rasterization combined with the Horizon-Based Ambient Occlusion:

![](/media/denis_with.png)

The report for the project is available [here](/media/final_report.pdf).

## Author and aknowledgment

Author: Marie Audouard, on the code base created and provided by Tamy Boubekeur for the INF584 course (Copyright (C) 2022 Tamy Boubekeur. All rights reserved).
