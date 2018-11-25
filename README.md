# ProtoIt
Educational software to get on with robotics via prototyping without the need of coding (Dutch language)

Windows: Download the setup file from: 'ProtoIt Setup/ProtoIt_1.0_win32_nl_setup.exe'.
Raspbian: Unpack the 'protoit.tar' in a convenient directory and 'qtlibs.tar' to '/usr/lib/arm-linux-gnueabihf'.

ProtoIt delivers a gui in which users drag and drop sensor signals into screen tiles of the required actuators and functions.
A robot program evolves step by step literally when the user adds steps with such tiles in it to operate the robot.
When finished, ProtoIt will create code, compile it and upload it to the robot.
At the moment ProtoIt supports Lego Mindstorms, MBot, Arduino (uno/mega) and Raspberry (rasbian).

ProtoIt is written in C++ using Qt-Creator. Keep the directory structure as in the git.
You will find a 'ProtoIt.pro' and 'ProtoItHelpCreator.pro' in the ProtoIt and ProtoItHelpCreator directories.
The setup file of ProtoIt is generated by Inno-Script-Studio and resides in the ProtoIt Setup directory.
The ProtoIt program in itself is contentless software as it needs additional definition files and compilers to run.
These reside in the ProtoIt Release directory and are divided in platform, programmer and version specific files.
For Lego Mindstorms the nxc-compiler is used, for MBot and Arduino the winavr utils and for Raspberry the nixg environment.

It is not needed to modify the ProtoIt program in order to supply new sensors and actuators for a platform.
The code that makes them function is supplied via '.pid' files. You will find them in the directory tree starting with
'ProtoIt Release/platforms'.

ProtoIt has been coded to support different languages (using Qt Linguist).
In fact the program itself is ready for it, but the directory tree 'ProtoIt Release/platforms' should be adjusted.
This directory tree does not allow for multiple languages, but the files can be translated in an ordinary editor.
