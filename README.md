# Augmented Reality MRI Viewer

## Introduction
This repository contains the source code for a demo developed in 2009 that uses augmented reality to display MRI images overalyed on a patient's body, allowing medical professionals to visualize internal structures in real-time. The software captures images from one camera, identifies multiple AR markers, and computes their 3D positions.

## ⚠️ Important Note
As of now, it is practically impossible to get this code running due to outdated dependencies and libraries. However, it is preserved here in the hopes that it might be of use to someone, either for educational purposes or as a foundation for further development.

## Technology Stack
The application was built using the Cing library, a C++ library inspired by Processing for creative coding. Cing aimed to bring the power of C++ to designers and artists using Processing's syntax. Furthermore, Cing was built upon several other libraries including:
- OpenCV: For computer vision tasks.
- Ogre: Used for 3D rendering.
- FMOD: For audio handling.
- Bullet: For physics simulations.
 

## Features
- Real-Time MRI Image Display: Utilizes augmented reality to overlay MRI images in real-time based on the positioning of AR markers.
- 3D Position Calculation: Identifies multiple AR markers and computes their 3D positions in the scene.
- Energy-Based Tracking System: Adjusts the energy of tracked models based on their movement, ensuring a smooth and stable display.
- Position and Orientation Filtering: Uses average filters for position data and spherical linear interpolation for orientation data, resulting in a more stable and accurate AR display.

## Dependencies
- [Cing library](https://github.com/CingProject/Cing)

## Installation & Setup
Given the age and dependencies of this project, setting it up might be challenging. Refer to the original documentation of the Cing library for potential guidance.

