// Copyright (C) 2014 D. Kruithof
//
// GNU General Public License Usage
// --------------------------------
// This file may be used under the terms of the GNU General Public
// License version 3.0 as published by the Free Software Foundation
// and appearing in the file 'GNU-GPL.txt' included in the packaging
// of this file.  Please review the following information to ensure
// the GNU General Public License version 3.0 requirements will be
// met: http://www.gnu.org/copyleft/gpl.html.

#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <QString>
#include <QList>
#include <QStringList>
#include <QToolBar>
#include <QIcon>
#include <QLineEdit>
#include <QComboBox>
#include "codewriter.h"

enum DeviceType { dtUndefined, dtActuator, dtSensor, dtFunction };
enum VarType { vtAll = 511, vtPort = 1, vtAddress = 2, vtPrivate = 4, vtCalibrate = 8, vtSignalIn = 16, vtSignalOut = 32, vtConstant = 64, vtGlobal = 128 };

class MainWindow;
class Device;
class Tile;

/*
 * CLASS VARLIST
 */

typedef struct varstruct
{
    // example for device 'Arm' section <PORT>: "PIN_M1, Claw open/closed, 2, [Armbeweging]"
    VarType     vt;         // = vtPort
    QString     key;        // = Arm_PIN_M1
    QString     alias;      // = Claw open/closed
    QString     init;       // = 2
    QString     paragraph;  // = Armbeweging
    bool        vtab;       // = false
    bool        expanded;   // = false
} Var;

class VarList
{
public :

    ~VarList();

    void addVar( Var* var);
    Var* firstVar( VarType vt);
    Var* nextVar( VarType vt);

protected:

    QList<Var*>     m_decl;
    int             m_ix;
};

/*
 * CLASS TEMPLATE
 */

class Template
{
    Q_DECLARE_TR_FUNCTIONS(Template)

public:
    Template( MainWindow* main);

    enum ParseType { ptDevice, ptAlias, ptIcon, ptHide, ptPort, ptAddress, ptPrivate, ptCalibrate, ptSignalIn, ptSignalOut, ptConstant,
                     ptCopyright, ptInclude, ptLibrary, ptGlobal, ptRoutine, ptStepInit, ptSetup, ptSensorUpdate, ptFunctionUpdate, ptActuatorUpdate};
    void parseLine( QString line, QString platform, QString model);  // the main program reads the file and feeds the routine
    void replaceKeys();             // when all lines are parsed the keywords must
                                    // be prefixed by the template alias
    void setPath( QString path);
    void setFilename( QString filename);
    void setAlias( QString alias);

    QString path();
    QString filename();
    QString icon();
    QString alias();
    QString device();
    Tile*   deviceTile();
    DeviceType deviceType();

    Tile* createDevice();           // creates the device tile on the config page
    Tile* createTile();             // creates a program tile on the other pages

    VarList          m_vars;        // parsed information about the fields
    CodeWriter*      m_code;        // containing the programmer code as in the device template file (.rtd)
    QStringList      m_libs;        // libraries used by this template that must be linked with the program

protected:

    QString identifier( QString decl);

    MainWindow*      m_main;        // instance of MainWindow
    QString          m_filename;    // template file name
    QString          m_path;        // template path

    ParseType        m_pt;          // for temporary use while parsing
    DeviceType       m_dt;
    QString          m_device;
    QString          m_alias;
    QString          m_icon;
    Tile*            m_tile;        // tile on config page
 };

#endif // TEMPLATE_H
