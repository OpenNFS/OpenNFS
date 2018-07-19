===========================================================================
FSHTOOL version 1.00 - (c) Denis Auroux 2000 - auroux@math.polytechnique.fr
===========================================================================

 Second generation FSH/QFS decompressor/compressor to BMP format
 Version 1.00 - copyright (c) Denis Auroux 1998-2000
 e-mail: auroux@math.polytechnique.fr

CONTENTS
========
- Download sites
- Important note
- New features
- Installation
- Using FSHTOOL
- Troubleshooting
- About INDEX.FSH

Download sites:
---------------
 http://www.math.polytechnique.fr/cmat/auroux/nfs/
 http://auroux.free.fr/nfs/

Important note:
---------------
 FSHTOOL is free software. It is distributed under the terms of the 
 GNU General Public License. No warranty. Distributing this software 
 without its source code is illegal.

New features:
-------------
 FSHTOOL is a new software that replaces the older QFS Editing Suite.
 It performs the same functionality, with greatly improved features.
 New features in the initial release include:

 - integration of all functions into one single program
 - eliminated intermediary FSH step for QFS decoding/encoding
 - Linux portability
 - QFS compression code is 20 times faster :-)
 - easily run from Windows by double-clicking on FSH/QFS files
 - added support for new NFS5 FSH files (including compressed/multiscale)
 - extracts alpha layer (transparency data) to a separate file
 - SHPI.DAT has been replaced by a human-readable INDEX.FSH
   (if you know what you are doing and are very careful, this file can
    be edited to allow more fundamental changes than before)

Installation:
-------------
 1. Uncompress the zip archive to the directory of your choice.
 2. If you plan on running FSHTOOL from Windows: the first time you
    need to unpack a FSH file, double-click on its icon ; Windows should
    ask you for the name of a program with which to open it. Use the
    "Browse..." button or any other method to select the executable file
    FSHTOOL.EXE in the directory where you installed it. Repeat the same
    process the first time you open a QFS file.

Using FSHTOOL:
--------------
1. To unpack a .QFS or .FSH file into a directory of .BMP files:

* from Windows: double-click on a .FSH or .QFS file. 

  Normally, FSHTOOL automatically runs. A directory with the same 
  name as the .FSH/.QFS file is created containing the .BMP files for 
  to the various graphics in the file. For example, if you selected a 
  file named x:\directory\filename.fsh, the bitmaps will be placed in 
  the directory x:\directory\filename\
  A file called index.fsh is also created for the purpose of allowing
  conversion back to .FSH/.QFS.

  NOTE: to work on a file located on a CD-ROM, you must copy it to your
  hard disk before it can be converted.

  If this is the first time you double-click on a .FSH/.QFS file, you
  need to tell Windows to open it with FSHTOOL. For this, in the dialog
  box prompting you for a program to use, select the button called
  "Browse" or "Other", and browse your hard disk until you find the
  executable file FSHTOOL.EXE.

* from a DOS command line: go into the directory containing
  the FSHTOOL executable file, and type

       fshtool x:\directory\filename.fsh

  Replace the drive letter, directory name and file name to suit your
  actual needs; the file extension can be either .fsh or .qfs.

  This creates a directory x:\directory\filename\ containing several
  .BMP files (the various grahics in the FSH/QFS file), and also a
  file called index.fsh file used for repacking things back into FSH/QFS
  format.

  If you want to place the BMP files in a differently named directory,
  specify this directory as second command-line argument: for example,

       fshtool x:\directory\filename.fsh c:\another\directory

  If you are using a Linux port of FSHTOOL, this method also works
  (with the obvious adaptations) from a command shell.

2. To re-create a .QFS or .FSH file from a directory of .BMP files:

* from Windows: double-click on the INDEX.FSH file in the directory.

  Normally, FSHTOOL automatically runs. The .FSH or .QFS file which
  was previously unpacked into the selected directory is directly 
  updated. The original .FSH/.QFS file is overwritten.

  BEWARE: it may be safer to make a backup copy of the original 
  .FSH/.QFS file BEFORE you run FSHTOOL ! Otherwise, be prepared to
  copy it again from your CD.

* from a DOS command line: go into the directory containing
  the FSHTOOL executable file, and type

       fshtool x:\path\dir\index.fsh

  Replace the drive letter and path to suit your actual needs.
  The .FSH or .QFS file which was previously unpacked into the chosen
  directory is directly updated. The original file is overwritten.

  BEWARE: it may be safer to make a backup copy of the original 
  .FSH/.QFS file BEFORE you run FSHTOOL ! Otherwise, be prepared to
  copy it again from your CD.

  If you want to place the FSH/QFS file in a different location, specify 
  the target file name as second command-line argument: for example,

       fshtool x:\path\dir\index.fsh c:\another\directory\filename.fsh

  If you are using a Linux port of FSHTOOL, this method also works
  (with the obvious adaptations) from a command shell.

Troubleshooting:
----------------

1. System-related issues:

Q: I cannot manage to run FSHTOOL when I am in DOS mode.
A: FSHTOOL is a 32-bit command-line program. You need to run it either
   from a DOS window inside Windows 95/98/NT/2000, or from a Linux command
   shell. (I haven't tested but Windows 3.1 with Win32s might also work).

Q: When I double click on a .FSH (or .QFS) file, another program completely
   unrelated to FSHTOOL runs.
A: This other program probably registered the .FSH or .QFS file. You need to
   modify the Windows configuration of file extensions (look in the Windows
   help if you don't know how to do this) so that .FSH and .QFS files are
   associated with FSHTOOL isntead. If you really are using a program whose
   native document format is .FSH or .QFS, the only solution is to run 
   FSHTOOL from a DOS command prompt.

Q: How do I run FSHTOOL under Linux ?
A: You simply need to compile the source code file fshtool.c by running
   "gcc -O2 fshtool.c -o fshtool" ; this creates an executable file
   called "fshtool" which can be used like the standard version
   (but from a Linux command line only).

Q: When I double-click on FSHTOOL I just get a message about its usage.
A: This is normal. FSHTOOL needs to be given command-line arguments, which
   can be done either by specifying a file name if you are running it from
   a command prompt, or by double clicking on a .FSH/.QFS file instead of
   clicking on FSHTOOL.EXE itself.

2. Basic troubleshooting:

Q: FSHTOOL says "Unsuccessful termination."
A: Some error happened. Look at the previous lines of output for information
   on the cause of the problem.

Q: When I try to open a .FSH/.QFS file on a CD-ROM or a read-only network
   drive, I get an error message saying "The directory is not empty or 
   cannot be created" and 3 choices, neither of which help me decompress 
   the file.
A: FSHTOOL attempts to create a bitmap directory on the read-only disk,
   which cannot be done. You must either copy the file to another location
   on your hard disk (where a subdirectory can be created), or run FSHTOOL
   from a command-line and specify a second argument to specify the location
   where a subdirectory is to be created.

Q: When I try to open a .FSH/.QFS file on my hard disk, I get an error 
   message saying "The directory is not empty or cannot be created" and 
   3 choices. What should I do ?
A: The directory that FSHTOOL is about to be create already exists and
   is not empty. If you want to overwrite the files it contains, choose
   choice 1 (press '1' and 'Enter'). If you want to create a differently
   named directory, choose 2 (press '2' then 'Enter'); the new directory
   name will be obtained by appending ".1" to the normal name (then ".2"
   the following time, etc...) If you want to abort FSHTOOL execution, 
   choose 3. If you chose '1' or '2' but still get an error, check that
   the FSH/QFS file is not located in a read-only directory (e.g. on a
   CD-ROM); if this is the case, see previous topic.

Q: I am stuck at a menu with 3 choices: I answered, but nothing happens.
A: Press Enter to validate your choice.

Q: I get one of the error messages "Unable to create file",
   "Unable to access directory" or "Cannot create output file".
A: Check that you are not trying to write on a read-only media such as
   a CD-ROM. Check that your disk is not full. Check that the directory
   or file names mentioned in the preceding output lines make sense.
   Also, if you are re-packing a .FSH/.QFS file that already exists,
   check that the target file is not marked read-only by Windows
   (in that case you need to manually erase it before running FSHTOOL).

Q: I get the error message "Input file not found".
A: Make sure you didn't mistype the name of the file to convert.

Q: I get the error message "Don't know how to derive a directory name".
A: Make sure that your input file has a .FSH or .QFS extension. If not,
   you can either rename it to .FSH/.QFS, or run FSHTOOL from the command
   line and specify explicitly a target directory name.

Q: I get the error message "Unknown file format" even though the file I
   clicked on is a .FSH or a .QFS.
A: The file is an unrecognized format. The only known instance of this is 
   the 8-bit texture file for the "Summit" track in NFS3, which uses a
   nonstandard compression algorithm. If this happens to you with a game
   in the NFS series at least as recent as NFS3 and with another file than
   the Summit track textures, please let me know the NFS version and the 
   directory and file name.

Q: When opening a specific file I get a warning message saying that
   "FSH bitmaps are not correctly ordered".
A: This means that the QFS/FSH file has a non-standard structure (this
   can happen mostly with the very first NFS versions); when re-packing
   the BMP data back into QFS/FSH format, data corruption may occur and
   cause NFS to crash. If this happens, try using the old QFS Suite
   instead of FSHTOOL to edit this file.

Q: I get one of the following errors: "Warning: bad length ? ... instead
   of ..." ; "Something strange happened at the end of compression" ;
   "Unknown palette format" ; "Unable to create index.fsh" ; "Incorrect
   attachment structure" ; "Attached data too large !" ; "Passed the next
   block ?" ; "Unknown FSH bitmap code" ; "Unknown FSH data type" ;
   or I get the "Insufficient memory" message even though I have lots of
   memory.
A: You may have found a new type of .FSH/.QFS file or a bug in FSHTOOL.
   E-mail me a precise description of what you attempted to do, the
   version number of your copy of FSHTOOL, the name of the game in the NFS
   series with which this happened, the directory and name of the FSH/QFS
   file that caused the problem, whether it is the original file or
   it has already been modified, and the exact error message.

3. BMP editing problems:

Q: I get an error message "Format error in INDEX.FSH", "Truncated
   INDEX.FSH", "Insufficiently many objects listed", or "End of file 
   not found after all objects were added".
A: You incorrectly modified a INDEX.FSH file. Please cross-check using
   the guidelines below.

Q: I get an error message "FSH data buffer full. Increase BUFSZ parameter".
A: You added bitmaps or other data to a .FSH file, which increased its size
   as a result. The BUFSZ parameter, located at the beginning of INDEX.FSH,
   contains the size (in bytes) of the memory area that will be allocated 
   to prepare the FSH data. It must be larger than the size of the FSH file.
   Increase this value.

Q: I get an error message "Cannot open file".
A: One of the bitmap files needed to reconstruct the FSH data is missing.
   Make sure you didn't erase it or rename it accidentally.

Q: I get one of the messages "Not a BMP file" or "Unsupported BMP format".
A: FSHTOOL only understands plain uncompressed BMP files. Check if your 
   graphics software creates uncompressed BMP files, and disable all fancy
   file format options.

Q: I get a message "Incorrect geometry".
A: The width or height or the BMP file supplied is not compatible with the
   size declared in INDEX.FSH. Resize your BMP file, or if you are sure of
   what you are doing you can modify INDEX.FSH.

Q: I get a message "Not a 8-bit BMP" / "Not a 24-bit BMP".
A: The color depth for the supplied BMP file is incorrect. FSHTOOL needs
   256-color BMPs for 8-bit bitmaps and for alpha channel bitmaps;
   24-bit TrueColor BMPs for all other cases.

Q: I get a warning "Color palette is not grayscale in alpha channel bitmap".
A: The BMP files for alpha channel bitmaps should be 256-color greyscale
   bitmaps. The color palette of the supplied bitmap is not greyscale
   (check your graphics software). The blue component of the palette will 
   be kept as alpha component.

Q: I get a warning "Bitmap contains more colors than its palette".
A: The number of colors usable by this bitmap is less than the number of
   actually used colors. Some of the pixels will therefore be garbled.
   Normally you should never modify the color palettes of the bitmaps you 
   edit. If you are sure of what you are doing, you can try to edit the 
   INDEX.FSH file.

Q: I get a warning "Bitmap palette differs from the global palette".
A: The colors usable by this bitmap are constrained by the global palette
   of the FSH file. Your graphics software modified the color palette,
   which is not allowed. Colors will appear garbled in the resulting file.

Q: I get an error "Multiscale BMP has wrong size" or "Compressed multiscale
   bitmaps not allowed".
A: Multiscale bitmaps (for NFS5 track textures) follow strict rules.
   Undo the changes you made in INDEX.FSH.


About INDEX.FSH:
----------------
IMPORTANT NOTE: do not modify this file if you are not sure of what you are
doing. Any modification may cause either FSHTOOL to malfunction or your
NFS game to crash. Don't ask me for any advice unless you are completely 
sure that the fault is with FSHTOOL and not with you.

The text file called INDEX.FSH holds info about the structure of the FSH
file; by modifying it carefully you can perform operations such as
increasing the number of bitmaps in a FSH file, changing the bitmap
geometries, etc. It is a purely text file, so do not edit it with a
word processor -- use only text editors such as the Windows Notepad. 
Be extremely careful with the amount of whitespace -- do NOT add any
whitespace where there were none or leave any blank lines in the file.
Failure to do so will almost always result in a crash of FSHTOOL.

The file structure is the following:

- the first line is a file format header and shouldn't be modified.
- the second line contains the name of the FSH/QFS file to be created.
- the third line contains "QFS" if the file is compressed (QFS format),
  "FSH" otherwise (FSH format).
- the 4th line gives the number of objects in the FSH file (change this
  if you add new bitmaps !) and the directory tag in encoded printable
  form (all non-alphanumeric characters are replaced by '%' and their
  hexadecimal ascii code).
- the 5th line gives the size in bytes of the buffer to be allocated for
  FSH data (this must always be at least 2K more than the size of the
  uncompressed FSH data).
- the 6th line contains "NOGLOBPAL" if there is no global palette, else
  "GLOBPAL " followed by the name of the file containing the global palette

Then, for each of the FSH objects:

- 1 line with the 4-character FSH directory entry name (in encoded printable
  form with %xx for non-alphanumeric characters) followed by the name of
  the master BMP file (see below for multiscale bitmaps)
- 1 line with the header data. If the file is plain binary data, this line
  only contains "BIN". If it is an ordinary bitmap it starts with "BMP".
  For a multiscale bitmap it starts with "MBP".

- in the case of a BMP entry, "BMP" is followed by the (2-digit hex) type
  code of the entry format (see below), "+n" where n is the number of
  attached data chunks (palette/text/data), the width, the height, and
  four auxiliary display position data entries between '{}'.
- in the case of a MBP entry, the declared filename contains the sequence 
  "%d" which will be replaced by the scale (0,1,2,...) to get the actual 
  name. In the header data, "MBP" is followed by the (2-digit hex) type
  code of the entry format (see below), "xn" where n is the maximum
  available scale, the width, the height, and three auxiliary data between
  '{}'. Both width and height must be equal to 2^n, where n is the maximum
  available scale.
- bitmap entry format codes: (add 0x80 == compressed bitmap)
    6D  16-bit 4:4:4:4 (with alpha channel)
    78  16-bit 0:5:6:5
    7B  8-bit (with palette)
    7D  32-bit 8:8:8:8 (with alpha channel)
    7E  16-bit 1:5:5:5 (with alpha channel)
    7F  24-bit 0:8:8:8
- if an alpha channel is expected, the line after the header data contains
  "alpha" and the file name of the alpha channel BMP (with a "%d" in the 
  case of a multiscale bitmap).

- for a BMP entry with attachments, additional lines as follows:
    "TXT 6F <n> 0" followed by a line containing n characters in encoded
                   printable form : attached text data.
    "PAL xx n 1 {.. .. .. ..}" : local palette placeholder. 
                                 xx = format code, n = # palette entries.
      palette format codes: 22  24-bit DOS
                            24  24-bit
                            29  16-bit NFS5
                            2A  32-bit
                            2D  16-bit
    "BIN xx n" followed by a line containing n bytes in hexadecimal :
               attached binary data.

- between any two FSH object blocks, or even between a FSH BMP object and an
  attachment / between attachments, padding bytes can be added by a line
  containing "!PAD n ....." (n = length of padding, .... = n bytes in
  hexadecimal).
- after all objects have been listed, a line containing "#END" finishes
  INDEX.FSH.

