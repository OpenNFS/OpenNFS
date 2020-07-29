<img src="doc/logo.png" align="right" height="300" width="300"/>

# OpenNFS 

#### Linux [![Build Status](https://type2labs.visualstudio.com/OpenNFS/_apis/build/status/GCC%20Linux%20OpenNFS%20Build)](https://type2labs.visualstudio.com/OpenNFS/_build/latest?definitionId=4) Windows [![Build status](https://type2labs.visualstudio.com/OpenNFS/_apis/build/status/MinGW%20Windows%20OpenNFS%20Build)](https://type2labs.visualstudio.com/OpenNFS/_build/latest?definitionId=5) Mac [![Build status](https://type2labs.visualstudio.com/OpenNFS/_apis/build/status/Clang%20Mac%20OpenNFS%20Build)](https://type2labs.visualstudio.com/OpenNFS/_build/latest?definitionId=3)

An attempt to recreate the classic Need for Speed Games (1-6), unpacking their original data files into a modern scratch built engine. Extremely early in development, current features include:
 
  * Reverse engineered asset loaders (tracks, cars, images, music etc) for NFS 1-6 PC and PSX (with the possibility for other systems as well), soon to be separated into LibOpenNFS for the creation of mod tools/converters through a stable API
  * A barebones game engine designed with enough configurability to accurately replicate each titles respective graphics and gampelay
  * Bullet Physics Integration for vehicle dynamics
  * New shaders that allow for dynamic lighting and shadows
  
## Planned Features

  * An actual implementation of gameplay that replicates the original titles
  * AI generated through neuroevolution for both the police and racers
  * Custom(izable), modular menus built from moddable configuration files
  * Automated import of game assets from CD, ISO or provided URLs
  * A track editor and automated converter between classic NFS titles, utlilising LibOpenNFS
  * Free Roam of an open world amalgamation of all classic NFS tracks
  * Multiplayer!

## Current Loadable Assets

Below is a table better describing the current level of asset load into OpenNFS:

| Game     | Cars | Tracks | Music |
|----------|------|--------|-------|
| NFS6 PC  |      |        |       |
| NFS5 PC  | 40%* | 20%*   |       |
| NFS4 PC  | 90%* | 90%*   |       |
| NFS4 PS1 | 50%* |        |       |
| NFS3 PC  | 100% | 100%   | 95%   |
| NFS3 PS1 | 40%  | 90%    | 95%   |
| NFS2 SE  | 90*  | 90%    |       |
| NFS2 PC  | 90%* | 90%    |       |
| NFS2 PS1 | 90% *| 90%    |       |
| NFS1     |      |        |       |

Some titles fail to load in current builds of OpenNFS due to the undertaking of a large parser refactor. 

NFS5 Track formats have been reverse engineered, but a stable parser has not yet been written.

![Screenshot](../main/doc/BuildProgress.png)

## Releases:

OpenNFS does not bundle any EA intellectual property. A copy of the original games must be provided to OpenNFS in order to function, instructions for this are available on the release page.

[View the latest stable feature releases Here](https://github.com/AmrikSadhra/OpenNFS/releases)

### For development builds on CI:
All versions (Windows/Mac/Linux) are built on VSTS, but the current build artifacts only contain the OpenNFS.exe. To use these builds, it is recommended to download a Github Release from the 'Releases' page, and replace the OpenNFS exe with that produced by CI. You will most likely also need to update the 'shaders' directory, using the latest available from Git. I will alter my CI scripts soon to produce zips that can be ran without these extra steps.

#### VSTS Windows 
[![Build status](https://type2labs.visualstudio.com/OpenNFS/_apis/build/status/MinGW%20Windows%20OpenNFS%20Build)](https://type2labs.visualstudio.com/OpenNFS/_build/latest?definitionId=5)

#### VSTS Linux 
[![Build Status](https://type2labs.visualstudio.com/OpenNFS/_apis/build/status/GCC%20Linux%20OpenNFS%20Build)](https://type2labs.visualstudio.com/OpenNFS/_build/latest?definitionId=4)

#### VSTS Mac 
[![Build status](https://type2labs.visualstudio.com/OpenNFS/_apis/build/status/Clang%20Mac%20OpenNFS%20Build)](https://type2labs.visualstudio.com/OpenNFS/_build/latest?definitionId=3)

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
Models, textures, tracks, cars by EA Seattle (C) 1998, 1999, 2002. EA Canada (C) 2000, EA Blackbox (C) 2002.
OpenNFS is not affiated in any way with EA

Released under the MIT License.

