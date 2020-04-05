#include <QApplication>
#include "DarkStyle.h"
#include "framelesswindow.h"
#include "mainwindow.h"
#include "GuiMain.h"
#include "CmdArg.hpp"

#define SKIP
#define OLD_STYLE_nop


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


        DarkStyle* dark = new DarkStyle;
        a.setStyle(dark);

        FramelessWindow framelessWindow;
        framelessWindow.setWindowTitle("GH Injector");
        framelessWindow.setWindowIcon(QIcon(":/GuiMain/gh_resource/GH Icon.ico"));


        GuiMain* mainWindow = new GuiMain(&framelessWindow);

    	
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
