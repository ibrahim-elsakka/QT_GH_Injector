/*
###############################################################################
#                                                                             #
# The MIT License                                                             #
#                                                                             #
# Copyright (C) 2017 by Juergen Skrotzky (JorgenVikingGod@gmail.com)          #
#               >> https://github.com/Jorgen-VikingGod                        #
#                                                                             #
# Sources: https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle  #
#                                                                             #
###############################################################################
*/

#include <QApplication>
#include "DarkStyle.h"
#include "framelesswindow.h"
#include "mainwindow.h"
#include "GuiMain.h"
#include "CmdArg.hpp"

#define SKIP
#define OLD_STYLE


int main(int argc, char *argv[]) {

#ifndef SKIP
    int res = CmdArg(argc, argv);
    if (res)
        return res;
#endif

    // Restart Application loop
    int currentExitCode = 0;
    do {

        QApplication a(argc, argv);
#ifndef OLD_STYLE
        // style our application with custom dark style

        DarkStyle* dark = new DarkStyle;
        a.setStyle(dark);
        // create frameless window (and set windowState or title)
        FramelessWindow framelessWindow;
        //framelessWindow.setWindowState(Qt::WindowFullScreen);
        framelessWindow.setWindowTitle("GH Injector");
        //framelessWindow.setWindowIcon(a.style()->standardIcon(QStyle::SP_DesktopIcon));
        QSize winSize = framelessWindow.size();

        // create our mainwindow instance
        //MainWindow *mainWindow = new MainWindow;
        GuiMain* mainWindow = new GuiMain(&framelessWindow);

        // add the mainwindow to our custom frameless window
        framelessWindow.setContent(mainWindow);
        framelessWindow.show();
#else
        GuiMain* mainWindow = new GuiMain;
        mainWindow->show();
#endif

    	
        currentExitCode = a.exec();
   	
        int i = 42;
    } while (currentExitCode == GuiMain::EXIT_CODE_REBOOT);

    return currentExitCode;
}
