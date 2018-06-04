# OpenNFS3

An attempt to recreate the original Need for Speed 3, unpacking it's original data files into a modern scratch build engine. Extremely early in development, current features include:
 
* Full Car Viv file loading. 
* Full NFS3 Track .frd/.col loading (inc. PSX w/ PSH textures)
* Preliminary NFS2 Track .trk/.col loading
* Preliminary MUS/MAP/LIN dynamic music playback

![Screenshot](../master/doc/BuildProgress.png)

## To run (for now):

Place car.viv in 'resources' directory, one level above your build output and run.

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

