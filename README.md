# ProtoIt
Educational software to get on with robotics via prototyping without the need of coding (Dutch language)

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
ProtoIt Release/platforms

ProtoIt has been coded to support different languages (using the 'tr' translate function of Qt).
In fact the program itself is ready for it and the only thing needed is the actual translation.
Especially the files in the directory tree ProtoIt Release/platforms will take a lot of time to translate.
