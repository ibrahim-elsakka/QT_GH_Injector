
#include <qfilesystemmodel.h>
#include <QTableWidget>

#include "GuiScanHook.h"

#include <qmessagebox.h>

#include "GuiMain.h"


GuiScanHook::GuiScanHook(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);


	connect(ui.btn_scan, SIGNAL(clicked()), this, SLOT(scan_clicked()));
	connect(ui.btn_unhook, SIGNAL(clicked()), this, SLOT(unhook_clicked()));


	model = new QStringListModel(this);

	
	List << "I'm" << "a" << "test" << "output";

	model->setStringList(List);
	ui.lv_scanhook->setModel(model);

	InjLib.Init();
	if (!InjLib.LoadingStatus())
	{
		emit injec_status(false, "Library or Function not found!");
		return;
	}
}

GuiScanHook::~GuiScanHook()
{

	
}

void GuiScanHook::setItem(const std::vector<std::string>& item)
{
	//int row = model->rowCount();
	//model->insertRows(row, 1);

	//QModelIndex index = model->index(row);
	//ui.lv_scanhook->setCurrentIndex(index);

	List.clear();
	
	for(auto i : item)
		List << QString::fromStdString(i);

	model->setStringList(List);
}

std::vector<std::string> GuiScanHook::getSelectedItem()
{
	std::vector<std::string> res;
	
	foreach(const QModelIndex &index, ui.lv_scanhook->selectionModel()->selectedIndexes())
	{	
		res.push_back(index.data(Qt::DisplayRole).toString().toStdString());
	}

	return res;
}

void GuiScanHook::get_from_inj_to_sh(int pid, int error)
{
	m_pid = pid;
	m_err = 0;
	

	ui.btn_scan->setText("Scan PID " + QString::number(pid));
	
	emit scan_clicked();
}

void GuiScanHook::refresh_gui()
{

}

void GuiScanHook::scan_clicked()
{	
	if(m_pid == 0)
		return;

	std::vector<std::string> tempHookList;
	
	tempHookList.push_back( "DLL.Function");
	tempHookList.push_back( "DLL2.Function3");
	//InjLib.ScanHook(m_pid, tempHookList);
	
	setItem(tempHookList);

}

void GuiScanHook::unhook_clicked()
{
	if(m_pid == 0)
		return;
	
	std::vector<std::string> selected = getSelectedItem();
	//InjLib.RestoreHook(m_pid, selected);

	List.clear();
	model->setStringList(List);

	int i = 42;
}

void GuiScanHook::injec_status(bool ok, const QString msg)
{
	if (ok)
	{
		QMessageBox messageBox;
		messageBox.information(0, "Success", msg);
		messageBox.setFixedSize(500, 200);

	}
	else
	{
		QMessageBox messageBox;
		messageBox.critical(0, "Error", msg);
		messageBox.setFixedSize(500, 200);
	}
}