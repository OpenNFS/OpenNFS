# Building OpenNFS from Source

Here you can find how to build OpenNFS from source and run it on supported platforms. It is still work in progress, so not all supported platforms are listed here yet.

## Getting the Code

Open a terminal in the directory where you want to have the code and run the following command to download the it:

```sh
git clone --recurse-submodules --remote-submodules https://github.com/OpenNFS/OpenNFS.git
```

There should be a newly created OpenNFS directory with the code in it.

## Installing dependencies

Most dependencies required are bundled with the code through git submodules, but there are still a few which will need to be downloaded. Below there are instructions specific for each platform.

### Ubuntu

To install the dependencies needed on Ubuntu, run the following commands in a terminal:

```sh
sudo apt-get update
sudo apt-get install build-essential cmake libgl-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libwayland-dev libxkbcommon-dev
```

## Building

Building should work the same on each platform. It can be done with the following commands from a terminal opened in the OpenNFS directory created in the `Getting the Code` step above:

```sh
mkdir build && cd build
cmake ..
cmake --build .
```

## Running

To be able to run OpenNFS, you'll need assets from NFS games. The assets of the PC version of NFS3 are required, assets for other games are optional.

Below where assets for each game go, the directory in resources will need to be created by hand:

- **NFS2 PC**: Copy the `gamedata` directory from the CD or an NFS2 installation into `OpenNFS/resources/NFS_2`.
- **NFS2 SE**: Copy the `gamedata` directory from the CD or an NFS2 SE installation into `OpenNFS/resources/NFS_2_SE`.
- **NFS3 PC**: Copy the `gamedata` and `fedata` directories from the CD or an NFS3 installation into `OpenNFS/resources/NFS_3`.
- **NFS4 PC**: Copy the `data` directory from the CD or an NFS4 installation into `OpenNFS/resources/NFS_4`.
- **NFS5 PC**: Copy the `gamedata` directory from the CD or an NFS5 installation into `OpenNFS/resources/NFS_5`.

After the assets have been copied, the executable called `OpenNFS` in `OpenNFS/build` can be launched.
