
#include <qfilesystemmodel.h>
#include <QTableWidget>

#include "GuiScanHook.h"
#include "GuiMain.h"


GuiScanHook::GuiScanHook(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);


	connect(ui.btn_scan, SIGNAL(clicked()), this, SLOT(scan_clicked()));
	connect(ui.btn_unhook, SIGNAL(clicked()), this, SLOT(unhook_clicked()));



}

GuiScanHook::~GuiScanHook()
{

	
}

void GuiScanHook::get_from_inj_sh(int pid, int error)
{



	emit refresh_gui();
}

void GuiScanHook::refresh_gui()
{

}

void GuiScanHook::scan_clicked()
{
	
}

void GuiScanHook::unhook_clicked()
{
	
}