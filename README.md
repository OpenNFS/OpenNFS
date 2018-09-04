# OpenNFS Linux

Linux [![CircleCI](https://circleci.com/gh/AmrikSadhra/OpenNFS.svg?style=svg)](https://circleci.com/gh/AmrikSadhra/OpenNFS) Windows [![Build status](https://ci.appveyor.com/api/projects/status/87px0nmkxcfpatkd?svg=true)](https://ci.appveyor.com/project/AmrikSadhra/opennfs)

An attempt to recreate the classic Need for Speed Games (1-5), unpacking their original data files into a modern scratch built engine. Extremely early in development, current features include:
 
  * Fully working NFS3, NFS2, NFS2SE, NFS3 PS1 track load (bugged High Stakes)
  * Explore TR02b! (if you have NFS_3_PS1 folder filled with the ISO contents)
  * Drive the car with WASD, Reset the car with 'R'
  * "Hermite Curve camera" tick box and untick car cam to flythrough track. Unselect both to have a 'free camera', moveable with WASD and viewpoint change by holding right mouse button, move faster by holding "Shift" key.
  * Select car and track from installed Need for Speeds using menu bar (your mileage may vary, major bug related to car load for non high-stakes)
  * Select Car models (Low LOD, Med LOD, Misc parts) in Debug Menu, by expanding "Car Models" dropdown and playing with checkboxes
  * Set Draw Distance by unchecking NBData and adjusting slider (Neighbour data only available for NFS3). High draw distances will drop FPS substantially due to an inefficient rendering method in use.
  * For NFS3 tracks, change between 'Classic Graphics' and a unfinished slightly more dynamic implementation.
  * Go to free camera, and expand the "Engine Entity" dialog box on an NFS3 track, to adjust lighting parameters for lights after targeting them with the cursor and * clicking. This will affect the colour of the track in non classic graphics mode. Adjust Track Specular Damper and Reflectivity similarly from Debug Menu.
  * Similarly, click the car to tweak physics parameters in real time in the "Engine Entity" dialog, and view data from the car.
  * Pan and zoom around the car with mouse wheel and LMB/RMB in "Car Cam" mode (ensure Hermite Curve cam is unchecked)
  * Set Sky Colour
  * View what the Physics Engine sees with "Bullet Debug View" checkbox.

## Current Loadable Assets

Below is a table better describing the current level of asset load into OpenNFS:

| Game     | Cars | Tracks | Music |
|----------|------|--------|-------|
| NFS6 PC  |      |        |       |
| NFS5 PC  |      |        |       |
| NFS4 PC  | 90%  | 60%    |       |
| NFS3 PC  | 100% | 100%   | 95%   |
| NFS3 PS1 | 40%  | 100%   | 95%   |
| NFS2 SE  | 90%  | 90%    |       |
| NFS2 PC  | 90%  | 90%    |       |
| NFS2 PC  | 90%  | 90%    |       |
| NFS2 PS1 | 10%  | 10%    |       |
| NFS1     |      |        |       |

![Screenshot](../master/doc/BuildProgress.png)

## Releases:

OpenNFS does not bundle any EA intellectual property. A copy of the original games must be provided to OpenNFS in order to function, instructions for this are available on the release page.

[View the latest stable feature releases Here](https://github.com/AmrikSadhra/OpenNFS/releases)

For development builds on CI:

[Windows](https://ci.appveyor.com/project/AmrikSadhra/opennfs/build/artifacts)

[Linux](https://circleci.com/gh/AmrikSadhra/OpenNFS/tree/master) Visit the Latest Build -> Artifacts -> "project/cmake-build-debug/OpenNFS.exe"

Mac - No CI support currently as all Mac CI providers cost money :( Hold tight for official releases on the [Github Releases page](https://github.com/AmrikSadhra/OpenNFS/releases)

## Trello Development Board

Check out what I'm working on by looking at the task board here:
https://trello.com/b/01KK3JMr/opennfs

## Dependencies

The CMake files are currently configured to detect external libraries from within the 'lib' folder at the same level as 'src', just do a 'git submodule update --recursive --remote' to get them.

* GLEW 2.1.0
* GLFW 3.2.1
* GLM 0.9.9-a2
* Bullet3
* Boost

## Thanks

Massive thanks to Denis Auroux, Vitaly 'Necromancer', EdasX, Ian Brownm, Jesper Juul-Mortensen and Arushan for their work on reverse engineering various NFS formats.

Tom 'VapeTrick' Klapwijk - sarcasm and support and snaking my dev updates

## Legal:
Models, textures, tracks, cars by EA Seattle (C) 1998.
OpenNFS is not affiated in any way with EA

Released under the MIT License.

