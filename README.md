# OpenNFS

An attempt to recreate the classic Need for Speed Games (1-5), unpacking their original data files into a modern scratch built engine. Extremely early in development, current features include:
 
* Full NFS3/NFS4 Car Viv file loading. 
* Full NFS3/4 Track .frd/.col loading (inc. PSX w/ PSH textures)
* Preliminary NFS2 Track .trk/.col loading
* Preliminary NFS3 PS1 car loading
* Preliminary MUS/MAP/LIN dynamic music playback

Below is a table better describing the current level of asset load into OpenNFS3:

| Game     | Cars | Tracks | Music |
|----------|------|--------|-------|
| NFS5     |      |        |       |
| NFS4 PC  | 90%  | 60%    |       |
| NFS4 PS1 | 5%   | 5%     |       |
| NFS3 PC  | 100% | 100%   | 95%   |
| NFS3 PS1 | 40%  | 100%   | 95%   |
| NFS2 SE  | 80%  | 80%    |       |
| NFS2 PC  | 80%  | 80%    |       |
| NFS2 PS1 |      |        |       |
| NFS1     |      |        |       |

![Screenshot](../master/doc/BuildProgress.png)

## To run (for now):

* Place NFS3 "GAMEDATA" folder into a subfolder named "NFS3" inside the "resources" folder e.g. resources/NFS3/GAMEDATA.
* Place NFS3 PS1 contents (ecm to iso) into a subfolder named "NFS3_PS1" inside the "resources" folder.
* Place NFS2 "GAMEDATA" folder into a subfolder named "NFS2" inside the "resources" folder e.g. resources/NFS2/GAMEDATA.
* Place NFS4 "DATA" folder into a subfolder named "NFS4" inside the "resources" folder e.g. resources/NFS4/DATA.

## Trello Development Board

Check out what I'm working on by looking at the task board here:
https://trello.com/b/01KK3JMr/opennfs3

## Dependencies

The CMake files are currently configured to detect external libraries from within the 'lib' folder at the same level as 'src', just do a 'git submodule update --recursive --remote' to get them.

* GLEW 2.1.0
* GLFW 3.2.1
* GLM 0.9.9-a2
* Bullet3
* Boost

## TODO

* Add object picking w/ Bullet physics
* Integrate T3ED track loading code from Denis Auroux, bind into OpenGL - (80%)
* Collision Detection, carp.txt performance model load into Physics
* MUS File reading (http://www.vgmpf.com/Wiki/index.php/MUS_(Electronic_Arts))
* 3D API Independence for eventual Vulkan/3Dfx Glide support ;)
* TensorFlow AI model 

## Thanks

Massive thanks to Denis Auroux, Vitaly 'Necromancer', EdasX, Ian Brown and Jesper Juul-Mortensen for their work on reverse engineering the FCE format. Check out their work here:

https://sites.google.com/site/2torcs/labs/need-for-speed-3---hot-pursuit/nfs3-the-unofficial-file-format-descriptions

Tom 'VapeTrick' Klapwijk - sarcasm and support and snaking my dev updates

## Legal:
Models, textures, tracks, cars by PEA Seattle (C) 1998.
OpenNFS3 is not affiated in any way with EA

Released under the MIT License.

