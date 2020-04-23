#include "GuiMain.h"

#include <qdesktopservices.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qstylefactory.h>
#include <qsettings.h>
#include <qdir.h>
#include <qmimedata.h>

#include <urlmon.h>
#include <fstream>
#include <string>

#include <string>
#include "Banner.h"
#include "Process.h"
#include "Injection.h"
#include "Compress.h"
#include "Zip.h"

int const GuiMain::EXIT_CODE_REBOOT = -123456789;

GuiMain::GuiMain(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// Settings
	connect(ui.rb_proc,  SIGNAL(clicked()), this, SLOT(rb_process_set()));
	connect(ui.rb_pid,   SIGNAL(clicked()), this, SLOT(rb_pid_set()));
	connect(ui.btn_proc, SIGNAL(clicked()), this, SLOT(btn_pick_process_click()));
	connect(ui.cmb_proc, SIGNAL(currentTextChanged(const QString&)), this, SLOT(cmb_proc_name_change()));
	connect(ui.txt_pid,  SIGNAL(textChanged(const QString&)),		 this, SLOT(txt_pid_change()));

	// Auto, Reset, Color
	connect(ui.cb_auto,   SIGNAL(clicked()), this, SLOT(auto_inject()));
	connect(ui.btn_reset, SIGNAL(clicked()), this, SLOT(reset_settings()));
	connect(ui.btn_hooks, SIGNAL(clicked()), this, SLOT(hook_Scan()));

	// Method, Cloaking, Advanced
	connect(ui.cmb_load,   SIGNAL(currentIndexChanged(int)), this, SLOT(load_change(int)));
	connect(ui.cmb_create, SIGNAL(currentIndexChanged(int)), this, SLOT(create_change(int)));
	connect(ui.cb_main,	 SIGNAL(clicked()),				 this, SLOT(cb_main_clicked()));

	// Files
	connect(ui.btn_add,    SIGNAL(clicked()), this, SLOT(add_file_dialog()));
	connect(ui.btn_inject, SIGNAL(clicked()), this, SLOT(delay_inject()));
	connect(ui.btn_remove, SIGNAL(clicked()), this, SLOT(remove_file()));
	connect(ui.tree_files, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(select_file()));

	// Info
	connect(ui.btn_tooltip, SIGNAL(clicked()), this, SLOT(tooltip_change()));
	connect(ui.btn_help,    SIGNAL(clicked()), this, SLOT(open_help()));
	connect(ui.btn_log,     SIGNAL(clicked()), this, SLOT(open_log()));
	connect(ui.btn_version, SIGNAL(clicked()), this, SLOT(check_online_version()));

	// Update
	QObject::connect(&dl_Manager, SIGNAL(finished()), this, SLOT(download_finish()));

	gui_Picker  = new GuiProcess();
	gui_Scanner = new GuiScanHook();
	ver_Manager = new QNetworkAccessManager(this);
	t_Auto_Inj  = new QTimer(this);
	t_Delay_Inj = new QTimer(this);
	pss         = new Process_State_Struct;
	ps_picker   = new Process_Struct;

	if (this->parentWidget())
	{
		framelessPicker.setWindowTitle("Select a process");
		framelessPicker.setContent(gui_Picker);
		framelessPicker.resize(QSize(460, 500));
		framelessPicker.setWindowIcon(QIcon(":/GuiMain/gh_resource/GH Icon.ico"));
	}

	if (this->parentWidget())
	{
		framelessScanner.setWindowTitle("Scan for hooks");
		framelessScanner.setContent(gui_Scanner);
		framelessScanner.resize(QSize(320, 230));
		framelessScanner.setWindowIcon(QIcon(":/GuiMain/gh_resource/GH Icon.ico"));
	}
	

	t_Delay_Inj->setSingleShot(true);
	pss->cbSession = true;
	pss->cmbArch = 0;
	pss->txtFilter = "";
	memset(ps_picker, 0, sizeof(Process_Struct));
	lightMode = false;

	// Process Picker
	connect(this, SIGNAL(send_to_picker(Process_State_Struct*, Process_Struct*)),
		gui_Picker, SLOT(get_from_inj(Process_State_Struct*, Process_Struct*)));
	connect(gui_Picker, SIGNAL(send_to_inj(Process_State_Struct*, Process_Struct*)),
		this, SLOT(get_from_picker(Process_State_Struct*, Process_Struct*)));

	// Scan Hook
	connect(this, SIGNAL(send_to_scan_hook(int, int)),
		gui_Scanner, SLOT(get_from_inj_to_sh(int, int)));
	connect(gui_Scanner, SIGNAL(send_to_inj_sh(int, int)),
		this, SLOT(get_from_scan_hook(int, int)));

	
	connect(ver_Manager,	&QNetworkAccessManager::finished, this, &GuiMain::replyFinished);
	connect(t_Auto_Inj, SIGNAL(timeout()), this, SLOT(auto_loop_inject()));
	connect(t_Delay_Inj,SIGNAL(timeout()), this, SLOT(inject_file()));

	
	// Resize Column
	for (int i = 0; i <= 3; i++)
		ui.tree_files->resizeColumnToContents(i);
	ui.tree_files->clear();

	setAcceptDrops(true);

	load_settings();
	color_setup();
	color_change();
	load_banner();
	load_change(42);
	create_change(42);
	//check_online_version();

	
	if (!InjLib.Init())
	{
		QString failMsg = GH_INJ_MOD_NAMEA + QString("not found");
		emit injec_status(false, failMsg);
	}

	// Reduze Height
	if(this->parentWidget())
	{
		QSize winSize = this->parentWidget()->size();
		winSize.setHeight(400);
		winSize.setWidth(1200);
		parentWidget()->resize(winSize);
	}
	else
	{
		QSize winSize = this->size();
		winSize.setHeight(400);
		winSize.setWidth(1200);
		this->resize(winSize);
	}


#ifdef _DEBUG
	hide_banner();
#endif _DEBUG
	platformCheck();

	bool status = SetDebugPrivilege(true);
	int i = 42;
}

GuiMain::~GuiMain()
{
	if (this->parentWidget())
		save_settings();
	
	delete gui_Picker;
	delete ver_Manager;
	delete t_Auto_Inj;
	delete t_Delay_Inj;
	delete pss;
	delete ps_picker;
}

int GuiMain::str_to_arch(const QString str)
{
	if (str == "x64") return X64;
	else if (str == "x86") return X86;
	else return NONE;
}

QString GuiMain::arch_to_str(const int arch)
{
	if (arch == 1) return QString("x64");
	else if (arch == 2) return QString("x86");
	else return QString("---");
}

void GuiMain::closeEvent(QCloseEvent* event)
{
	if (!this->parentWidget())
		save_settings();
}

std::string GuiMain::getVersionFromIE()
{
	char cacheFile[MAX_PATH] = { 0 };
	HRESULT hRes = URLDownloadToCacheFileA(nullptr, GH_VERSION_URL, cacheFile, sizeof(cacheFile), 0, nullptr);

	if (hRes != S_OK)
		return "";

	// Read file 
	std::ifstream infile(cacheFile, std::ifstream::in);

	if (!infile)
		return "";

	// ???????????????
	// Why is this different on local debug version
	std::string strVer;
	infile >> strVer;

	infile.close();

	return strVer;
}

void GuiMain::dragEnterEvent(QDragEnterEvent* e)
{
	if (e->mimeData()->hasUrls()) {
		e->acceptProposedAction();
	}
}

void GuiMain::dragMoveEvent(QDragMoveEvent* e)
{
	int i = 42;
}

void GuiMain::dragLeaveEvent(QDragLeaveEvent* e)
{
	int i = 42;
}

void GuiMain::dropEvent(QDropEvent* e)
{
	foreach(const QUrl & url, e->mimeData()->urls()) {
		QString fileName = url.toLocalFile();
		//qDebug() << "Dropped file:" << fileName;
		QFileInfo fi(fileName);
		if (fi.completeSuffix() == QString("dll"))
			add_file_to_list(fileName, "");
	}
}

void GuiMain::platformCheck()
{
#ifndef _DEBUG
#ifndef _WIN64

	// windows 64-bit == gh64.exe
	bool bPlatform = isCorrectPlatform();
	if (bPlatform == true)
		return;

	// Won't work
	ui.cb_hijack->setChecked(false);
	ui.cb_hijack->setDisabled(true);


	QMessageBox::StandardButton reply;
	reply = QMessageBox::warning(nullptr, "Warning architecture conflict", "Since you're using a\
64-bit version of Windows it's recommended to use the 64-bitversion of the injector. \
Do you want to switch to the 64-bit version?", QMessageBox::Yes | QMessageBox::No);

	if (reply == QMessageBox::No)
		return;
		
	bool bStart = StartProcess(GH_INJ_EXE_NAME64A);
	if (bStart == false)
		return; // can't start, maybe msgbox


	//qApp->quit();
	// I am qt and do not want to close
	QTimer::singleShot(250, qApp, SLOT(quit()));

#endif // _WIN64
#endif _DEBUG
}

void GuiMain::rb_process_set()
{
	ui.rb_proc->setChecked(true);
	ui.cmb_proc->setEnabled(true);
	ui.txt_pid->setEnabled(false);
}

void GuiMain::rb_pid_set()
{
	ui.cmb_proc->setEnabled(false);
	ui.rb_pid->setChecked(true);
	ui.txt_pid->setEnabled(true);
}

void GuiMain::rb_unset_all()
{
	ui.cmb_proc->setEnabled(false);
	ui.txt_pid->setEnabled(false);

	// turn off all Radio Buttons
	ui.rb_pid->setAutoExclusive(false); 
	ui.rb_pid->setChecked(false);
	ui.rb_proc->setChecked(false);
	ui.rb_pid->setAutoExclusive(true);
}

void GuiMain::btn_pick_process_click()
{
	if(gui_Picker->parentWidget())
		framelessPicker.show();
	else
		gui_Picker->show();
	emit send_to_picker(pss, ps_picker);
}

void GuiMain::cmb_proc_name_change()
{
	if (ui.rb_proc->isChecked())
	{
		QString proc = ui.cmb_proc->currentText();
		Process_Struct pl = getProcessByName(proc.toStdString().c_str());

		memcpy(ps_picker, &pl, sizeof(Process_Struct));
		ui.txt_pid->setText(QString::number(ps_picker->pid));
		ui.txt_arch->setText(GuiMain::arch_to_str(ps_picker->arch));

		btn_hook_scan_change();
	}
}

void GuiMain::txt_pid_change()
{
	if (ui.rb_pid->isChecked())
	{
		Process_Struct pl = getProcessByPID(ui.txt_pid->text().toInt());

		memcpy(ps_picker, &pl, sizeof(Process_Struct));
		ui.cmb_proc->setCurrentText(ps_picker->name);
		ui.txt_arch->setText(GuiMain::arch_to_str(ps_picker->arch));

		btn_hook_scan_change();
	}
}

void GuiMain::btn_hook_scan_change()
{
	if(ps_picker->arch)
		ui.btn_hooks->setEnabled(true);

	else
		ui.btn_hooks->setEnabled(false);
}

void GuiMain::get_from_picker(Process_State_Struct* procStateStruct, Process_Struct* procStruct)
{
	pss = procStateStruct;
	ps_picker = procStruct;
	if (gui_Picker->parentWidget())
		framelessPicker.hide();
	else
		gui_Picker->hide();

	if (ps_picker->pid)
	{
		rb_unset_all();
		int index = ui.cmb_proc->findText(ps_picker->name);
		if(index == -1) // check exists
			ui.cmb_proc->addItem(ps_picker->name);
		ui.cmb_proc->setCurrentIndex(ui.cmb_proc->findText(ps_picker->name));
		ui.txt_pid->setText(QString::number(ps_picker->pid));
		ui.txt_arch->setText(GuiMain::arch_to_str(ps_picker->arch));
		rb_pid_set();
	}
}

void GuiMain::get_from_scan_hook(int pid, int error)
{
	if (gui_Picker->parentWidget())
		framelessPicker.hide();
	else
		gui_Picker->hide();
}


void GuiMain::auto_inject()
{
	if (ui.cb_auto->isChecked())
	{
		// Restart if running
		t_Auto_Inj->start(200);
	}
	else
	{
		t_Auto_Inj->stop();
	}
}

void GuiMain::auto_loop_inject()
{
	if (ui.cb_auto->isChecked())
	{
		if (ui.rb_proc->isChecked())
		{
			QString proc = ui.cmb_proc->currentText();
			Process_Struct pl = getProcessByName(proc.toStdString().c_str());

			if (pl.pid)
			{
				ui.cb_auto->setChecked(false);
				t_Auto_Inj->stop();
				emit delay_inject();
				//injec_status(true, "Test Message");
			}
		}
	}
	else
	{
		t_Auto_Inj->stop();
	}
}

void GuiMain::color_setup()
{
	if (!this->parentWidget())
	{
		// https://gist.github.com/QuantumCD/6245215
		// https://github.com/Jorgen-VikingGod/Qt-Frameless-Window-DarkStyle/blob/master/DarkStyle.cpp
		// Style bullshit
		qApp->setStyle(QStyleFactory::create("Fusion"));
		normalPalette = qApp->palette();
		normalSheet = qApp->styleSheet();

		darkPalette.setColor(QPalette::Window, QColor(0x2D, 0x2D, 0x2D));
		darkPalette.setColor(QPalette::WindowText, Qt::white);
		darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
		darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
		darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
		darkPalette.setColor(QPalette::ToolTipText, Qt::white);
		darkPalette.setColor(QPalette::Text, Qt::white);
		darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
		darkPalette.setColor(QPalette::ButtonText, Qt::white);
		darkPalette.setColor(QPalette::BrightText, Qt::red);
		darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
		darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
		darkPalette.setColor(QPalette::HighlightedText, Qt::black);

		darkPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(50, 50, 50));
		darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0, 0, 0));

		darkSheet = ("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
		/*ui.cb_auto->setStyleSheet()*/
	}
}

void GuiMain::color_change()
{
	if (!this->parentWidget())
	{
		if (lightMode)
		{
			qApp->setPalette(normalPalette);
			qApp->setStyleSheet(normalSheet);

			QPixmap pix_banner;
			pix_banner.loadFromData(getBannerWhite(), getBannerWhiteLen(), "JPG");
			ui.lbl_img->setPixmap(pix_banner);
		}
		else
		{
			qApp->setPalette(darkPalette);
			qApp->setStyleSheet(darkSheet);

			QPixmap pix_banner;
			pix_banner.loadFromData(getBanner(), getBannerLen(), "JPG");
			ui.lbl_img->setPixmap(pix_banner);
		}
	}
}

void GuiMain::load_banner()
{
	QPixmap pix_banner;
	pix_banner.loadFromData(getBanner(), getBannerLen(), "JPG");
	ui.lbl_img->setPixmap(pix_banner);
}

void GuiMain::hide_banner()
{
	ui.lbl_img->setVisible(false);
}

void GuiMain::reset_settings()
{
	// delete file
	QString iniName = QCoreApplication::applicationName() + ".ini";
	QFile iniFile(iniName);
	if (iniFile.exists())
	{
		iniFile.remove();
	}

	emit slotReboot();
}

void GuiMain::slotReboot()
{
	//qDebug() << "Performing application reboot...";
	qApp->exit(GuiMain::EXIT_CODE_REBOOT);
}

void GuiMain::hook_Scan()
{
	if (gui_Scanner->parentWidget())
		framelessScanner.show();
	else
		gui_Scanner->show();
	
	emit send_to_scan_hook(ps_picker->pid, 0);
}

void GuiMain::save_settings()
{
	QSettings settings((QCoreApplication::applicationName() + ".ini"), QSettings::IniFormat);

	settings.beginWriteArray("FILES");
	int i = 0;
	QTreeWidgetItemIterator it(ui.tree_files);
	while (*it)
	{
		settings.setArrayIndex(i);
		settings.setValue(QString::number(i), (*it)->text(2));
		settings.setValue(QString::number(++i), (*it)->text(0));
		++it; i++;
	}
	settings.endArray();


	settings.beginWriteArray("PROCESS");
	for (int i = 0; i < ui.cmb_proc->count(); i++)
	{
		settings.setArrayIndex(i);
		settings.setValue(QString::number(i), ui.cmb_proc->itemText(i));
	}
	settings.endArray();


	settings.beginGroup("CONFIG");

	// Settings
	settings.setValue("PROCESS",		ui.cmb_proc->currentIndex());
	settings.setValue("PID",			ui.txt_pid->text());
	settings.setValue("PROCESSBYNAME",	ui.rb_proc->isChecked());
	settings.setValue("ARCH",			ui.txt_arch->text());	
	settings.setValue("DELAY",			ui.txt_delay->text());	
	settings.setValue("AUTOINJ",		ui.cb_auto->isChecked());
	settings.setValue("CLOSEONINJ",		ui.cb_close->isChecked());

	// Method
	settings.setValue("MODE",			ui.cmb_load->currentIndex());
	settings.setValue("LAUNCHMETHOD",	ui.cmb_create->currentIndex());
	settings.setValue("HIJACK",			ui.cb_hijack->isChecked());
	settings.setValue("CLOAK",			ui.cb_clock->isChecked());

	// Cloaking
	settings.setValue("PEH",			ui.cmb_peh->currentIndex());
	settings.setValue("UNLINKPEB",		ui.cb_unlink->isChecked());
	settings.setValue("RANDOMIZE",		ui.cb_random->isChecked());
	settings.setValue("DLLCOPY",		ui.cb_copy->isChecked());

	// Manuel mapping
	settings.setValue("SHIFTMODULE",	ui.cb_shift->isChecked());
	settings.setValue("CLEANDIR",		ui.cb_clean->isChecked());
	settings.setValue("IMPORTS",		ui.cb_imports->isChecked());
	settings.setValue("DELAYIMPORTS",	ui.cb_delay->isChecked());
	settings.setValue("TLS",			ui.cb_tls->isChecked());
	settings.setValue("SEH",			ui.cb_seh->isChecked());
	settings.setValue("PROTECTION",		ui.cb_protection->isChecked());
	settings.setValue("DLLMAIN",		ui.cb_main->isChecked());

	// Process picker
	settings.setValue("PROCNAMEFILTER", pss->txtFilter);
	settings.setValue("PROCESSTYPE",	pss->cmbArch);
	settings.setValue("CURRENTSESSION", pss->cbSession);

	// Info
	settings.setValue("TOOLTIPSON",		ui.btn_tooltip->isChecked());

	// Not visible
	settings.setValue("LASTDIR",			lastPathStr);
	settings.setValue("IGNOREUPDATES",	ignoreUpdate);
	settings.setValue("LIGHTMODE",		lightMode);
	settings.setValue("STATE",		saveState());
	settings.setValue("GEOMETRY", saveGeometry());	
	// Broken on frameless window

	// Selected DLL
	for (QTreeWidgetItemIterator it2(ui.tree_files); (*it2) != nullptr; ++it2)
		if ((*it2)->text(0) == "YES")
			settings.setValue("ACTIVEDLL", (*it2)->text(1));

	settings.endGroup();
}

void GuiMain::load_settings()
{
	QFile iniFile((QCoreApplication::applicationName() + ".ini"));
	if (!iniFile.exists())
		return;


	QSettings settings((QCoreApplication::applicationName() + ".ini"), QSettings::IniFormat);


	int fileSize = settings.beginReadArray("FILES");
	for (int i = 0; i < fileSize; ++++i) {
		settings.setArrayIndex(i);
		add_file_to_list(
			settings.value(QString::number(i)).toString(),
			settings.value(QString::number(i + 1)).toString());
	}
	settings.endArray();

	int procSize = settings.beginReadArray("PROCESS");
	for (int i = 0; i < procSize; ++++i) {
		settings.setArrayIndex(i);
		ui.cmb_proc->addItem(settings.value(QString::number(i)).toString());
	}
	settings.endArray();


	settings.beginGroup("CONFIG");

	// Settings
	ui.cmb_proc		->setCurrentIndex(settings.value("PROCESS").toInt());
	ui.txt_pid		->setText(settings.value("PID").toString());
	ui.rb_proc		->setChecked(settings.value("PROCESSBYNAME").toBool());
	ui.txt_arch		->setText(settings.value("ARCH").toString());
	ui.txt_delay	->setText(settings.value("DELAY").toString());
	ui.cb_auto		->setChecked(settings.value("AUTOINJ").toBool());
	ui.cb_close		->setChecked(settings.value("CLOSEONINJ").toBool());

	// Method
	ui.cmb_load		->setCurrentIndex(settings.value("MODE").toInt());
	ui.cmb_create	->setCurrentIndex(settings.value("LAUNCHMETHOD").toInt());
	ui.cb_hijack	->setChecked(settings.value("HIJACK").toBool());
	ui.cb_clock		->setChecked(settings.value("CLOAK").toBool());

	// Cloaking
	ui.cmb_peh		->setCurrentIndex(settings.value("PEH").toInt());
	ui.cb_unlink	->setChecked(settings.value("UNLINKPEB").toBool());
	ui.cb_random	->setChecked(settings.value("RANDOMIZE").toBool());
	ui.cb_copy		->setChecked(settings.value("DLLCOPY").toBool());

	// Manuel mapping
	ui.cb_shift		->setChecked(settings.value("SHIFTMODULE").toBool());
	ui.cb_clean		->setChecked(settings.value("CLEANDIR").toBool());
	ui.cb_imports	->setChecked(settings.value("IMPORTS").toBool());
	ui.cb_delay		->setChecked(settings.value("DELAYIMPORTS").toBool());
	ui.cb_tls		->setChecked(settings.value("TLS").toBool());
	ui.cb_seh		->setChecked(settings.value("SEH").toBool());
	ui.cb_protection->setChecked(settings.value("PROTECTION").toBool());
	ui.cb_main		->setChecked(settings.value("DLLMAIN").toBool());

	// Process picker
	pss->txtFilter	= settings.value("PROCNAMEFILTER").toString();
	pss->cmbArch	= settings.value("PROCESSTYPE").toInt();
	pss->cbSession	= settings.value("CURRENTSESSION").toBool();

	// Info
	ui.btn_tooltip	->setChecked(settings.value("TOOLTIPSON").toBool());

	// Not visible
	lastPathStr		= settings.value("LASTDIR").toString();
	ignoreUpdate	= settings.value("IGNOREUPDATES").toBool();
	lightMode		= settings.value("LIGHTMODE").toBool();
	restoreState	(settings.value("STATE").toByteArray());
	restoreGeometry	(settings.value("GEOMETRY").toByteArray());

	for (QTreeWidgetItemIterator it2(ui.tree_files); (*it2) != nullptr; ++it2)
		if ((*it2)->text(1) == settings.value("ACTIVEDLL").toString())
			(*it2)->setText(0, "YES");

	settings.endGroup();
}

void GuiMain::load_change(int i)
{
	INJECTION_MODE mode = (INJECTION_MODE)ui.cmb_load->currentIndex();

	switch (mode)
	{
		case INJECTION_MODE::IM_LoadLibraryExW:
			ui.cmb_load->setToolTip("LoadLibraryExW is the default injection method which simply uses LoadLibraryExW.");
			ui.grp_adv->setVisible(false);
			ui.cb_unlink->setEnabled(true);
			break;
		case INJECTION_MODE::IM_LdrLoadDll:
			ui.cmb_load->setToolTip("LdrLoadDll is an advanced injection method which uses LdrLoadDll and bypasses LoadLibrary(Ex) hooks.");
			ui.grp_adv->setVisible(false);
			ui.cb_unlink->setEnabled(true);
			break;
		case INJECTION_MODE::IM_LdrpLoadDll:
			ui.cmb_load->setToolTip("LdrpLoadDll is an advanced injection method which uses LdrpLoadDll and bypasses LdrLoadDll hooks.");
			ui.grp_adv->setVisible(false);
			ui.cb_unlink->setEnabled(true);
			break;
		default: 
			ui.cmb_load->setToolTip("ManualMap is an advanced injection technique which bypasses most module detection methods.");
			ui.grp_adv->setVisible(true);
			ui.cb_unlink->setEnabled(false);
			ui.cb_unlink->setChecked(false);
			break;
	}
}

void GuiMain::create_change(int i)
{
	LAUNCH_METHOD method = (LAUNCH_METHOD)ui.cmb_create->currentIndex();

	switch (method)
	{
		case LAUNCH_METHOD::LM_NtCreateThreadEx:
			ui.cmb_load->setToolTip("NtCreateThreadEx: Creates a simple remote thread to load the dll(s).");
			ui.cb_clock->setEnabled(true);
			break;
		case LAUNCH_METHOD::LM_HijackThread:
			ui.cmb_load->setToolTip("Thread hijacking: Redirects a thread to a codecave to load the dll(s).");
			ui.cb_clock->setEnabled(false);
			ui.cb_clock->setChecked(false);
			break;		
		case LAUNCH_METHOD::LM_SetWindowsHookEx:
			ui.cmb_load->setToolTip("SetWindowsHookEx: Adds a hook into the window callback list which then loads the dll(s).");
			ui.cb_clock->setEnabled(false);
			ui.cb_clock->setChecked(false);
			break;		
		default:
			ui.cmb_load->setToolTip("QueueUserAPC: Registers an asynchronous procedure call to the process' threads which then loads the dll(s).");
			ui.cb_clock->setEnabled(false);
			ui.cb_clock->setChecked(false);
			break;
	}	
}

void GuiMain::cb_main_clicked()
{
	if (ui.cb_main->isChecked())
	{
		ui.cb_imports->setEnabled(false);
		ui.cb_imports->setChecked(true);
	}
	else
	{
		ui.cb_imports->setEnabled(true);
	}
}

void GuiMain::add_file_dialog()
{
	QFileDialog fDialog(this, "Select dll files", lastPathStr, "Dynamic Link Libraries (*.dll)");
	fDialog.setFileMode(QFileDialog::ExistingFiles);
	fDialog.exec();

	for (auto l : fDialog.selectedFiles())
		GuiMain::add_file_to_list(l, "");

	lastPathStr = fDialog.windowFilePath();
}

void GuiMain::add_file_to_list(QString str, QString active)
{
	// nop, not the same files
	for (QTreeWidgetItemIterator it(ui.tree_files); (*it) != nullptr; ++it)
		if ((*it)->text(2) == str)
			return;

	QFileInfo fi(str);
	QTreeWidgetItem* item = new QTreeWidgetItem(ui.tree_files);

	item->setText(0, active);
	item->setText(1, fi.fileName());
	item->setText(2, fi.absoluteFilePath());
	int arch = (int)getFileArch(fi.absoluteFilePath().toStdString().c_str());
	item->setText(3, arch_to_str(arch));
}

void GuiMain::remove_file()
{
	QTreeWidgetItem* item = ui.tree_files->currentItem();
	delete item;
}


void GuiMain::select_file()
{
	QTreeWidgetItem* it2 = ui.tree_files->currentItem();

	if(it2->text(0) == "")
		it2->setText(0, "YES");
	else
		it2->setText(0, "");

	return;
	// Old
	QTreeWidgetItemIterator it(ui.tree_files);
	while (*it)
	{
		(*it)->setText(0, "");
		++it;
	}

	it2->setText(0, "YES");
}

void GuiMain::delay_inject()
{
	int delay = ui.txt_delay->text().toInt();
	if (delay > 0)
	{
		t_Delay_Inj->start(delay);
	}
	else
	{
		emit inject_file();
	}
}


void GuiMain::inject_file()
{
	INJECTIONDATAA data;
	memset(&data, 0, sizeof(INJECTIONDATAA));

	//Process_Struct ps_inject;
	//memset(&ps_inject, 0, sizeof(Process_Struct));

	int fileType	= NONE;
	int processType = NONE;
	int injCounter  = 0;

	// Process ID
	if (ui.rb_pid->isChecked())
	{
		int id = ui.txt_pid->text().toInt();
		if (id)
		{
			data.ProcessID = id;
			processType = getProcArch(id);
		}
		else
		{
			emit injec_status(false, "Invalid PID");
			return;
		}
	}
	else // Process Name
	{

		int index = ui.cmb_proc->currentIndex();
		Process_Struct p = getProcessByName(ui.cmb_proc->itemText(index).toStdString().c_str());
		if (p.pid)
		{
			data.ProcessID = p.pid;
			processType = p.arch;
		}
		else
		{
			emit injec_status(false, "Invalid Process Name");
			return;
		}
	}
	
	switch (ui.cmb_load->currentIndex())
	{
	case 1:  data.Mode = INJECTION_MODE::IM_LoadLibraryExW; break;
	case 2:  data.Mode = INJECTION_MODE::IM_LdrLoadDll;		break;
	case 3:  data.Mode = INJECTION_MODE::IM_LdrpLoadDll;    break;
	default: data.Mode = INJECTION_MODE::IM_LoadLibraryExW; break;
	}

	switch (ui.cmb_create->currentIndex())
	{
	case 1:  data.Method = LAUNCH_METHOD::LM_HijackThread;		break;
	case 2:  data.Method = LAUNCH_METHOD::LM_SetWindowsHookEx;	break;
	case 3:  data.Method = LAUNCH_METHOD::LM_QueueUserAPC;		break;
	default: data.Method = LAUNCH_METHOD::LM_NtCreateThreadEx;	break;
	}

	if (ui.cmb_peh->currentIndex() == 1)	data.Flags |= INJ_ERASE_HEADER;
	if (ui.cmb_peh->currentIndex() == 2)	data.Flags |= INJ_FAKE_HEADER;
	if (ui.cb_unlink->isChecked())			data.Flags |= INJ_UNLINK_FROM_PEB;
	if (ui.cb_clock->isChecked())			data.Flags |= INJ_THREAD_CREATE_CLOAKED;
	if (ui.cb_random->isChecked())			data.Flags |= INJ_SCRAMBLE_DLL_NAME;
	if (ui.cb_copy->isChecked())			data.Flags |= INJ_LOAD_DLL_COPY;
	if (ui.cb_hijack->isChecked())			data.Flags |= INJ_HIJACK_HANDLE;

	if (data.Mode == INJECTION_MODE::IM_ManualMap)
	{
		if (ui.cb_shift->isChecked())		data.Flags |= INJ_MM_SHIFT_MODULE;
		if (ui.cb_clean->isChecked())		data.Flags |= INJ_MM_CLEAN_DATA_DIR;
		if (ui.cb_imports->isChecked())		data.Flags |= INJ_MM_RESOLVE_IMPORTS;
		if (ui.cb_delay->isChecked())		data.Flags |= INJ_MM_RESOLVE_DELAY_IMPORTS;
		if (ui.cb_tls->isChecked())			data.Flags |= INJ_MM_EXECUTE_TLS;
		if (ui.cb_seh->isChecked())			data.Flags |= INJ_MM_ENABLE_SEH;
		if (ui.cb_protection->isChecked())	data.Flags |= INJ_MM_SET_PAGE_PROTECTIONS;
		if (ui.cb_main->isChecked())		data.Flags |= INJ_MM_RUN_DLL_MAIN;
	}


	data.GenerateErrorLog = true;

	if (!InjLib.LoadingStatus())
	{
		emit injec_status(false, "Library or Function not found!");
		return;
	}
	

	for (QTreeWidgetItemIterator it(ui.tree_files); (*it) != nullptr; ++it)
	{
		// Find Item
		if ((*it)->text(0) != "YES")
			continue;

		// Convert String
		QString fileStr	= (*it)->text(2);
		for (int i = 0, j = 0; fileStr[i].toLatin1() != '\0'; i++, j++)
		{
			if (fileStr[i] == '/')
				data.szDllPath[j] = '\\';
			else
				data.szDllPath[j] = fileStr[i].toLatin1();		
		}
		
		// Check Existens
		QFile qf(fileStr);
		if (!qf.exists())
		{
			emit injec_status(false, "File not found");
			return;
		}

		// Check Architecture
		fileType = str_to_arch((*it)->text(3));
		if (fileType == NONE)
		{
			emit injec_status(false, "File Architecture invalid");
			return;
		}

		// Check File Selected
		if (fileType == NONE)
		{
			emit injec_status(false, "File not selected");
			return;
		}


		if (processType != fileType || processType == NULL || fileType == NULL)
		{
			emit injec_status(false, "File and Process are incompatible");
			return;
		}


		DWORD res = InjLib.InjectFuncA(&data);
		if (res)
		{
			QString failMsg = "Inject failed with" + QString::number(res);
			emit injec_status(false, failMsg);
			return;
		}
		injCounter++;
	}

	if(injCounter)
	{
		emit injec_status(true, "Success Injection");
	}
	else
	{
		emit injec_status(true, "No file selected");
	}
	
	
	if (ui.cb_close->isChecked())
	{
		qApp->exit(0);
		return;
	}
	
	return;
}

void GuiMain::injec_status(bool ok, const QString msg)
{
	if(ok)
	{
		QMessageBox messageBox;
		messageBox.information(0, "Success", msg);
		messageBox.setFixedSize(500, 200);

		if (ui.cb_close->isChecked())
		{
			this->close();
		}
	}
	else
	{
		QMessageBox messageBox;
		messageBox.critical(0, "Error", msg);
		messageBox.setFixedSize(500, 200);
	}
}

void GuiMain::load_Dll()
{

}


void GuiMain::tooltip_change()
{
	if (ui.btn_tooltip->isChecked())
		ui.btn_tooltip->setText("Disable tooltips");
	else
		ui.btn_tooltip->setText("Enable tooltips");

	int duration = 1;
	if (ui.btn_tooltip->isChecked())
		duration = -1;

	// Settings
	ui.lbl_proc->setToolTipDuration(duration);
	ui.rb_proc->setToolTipDuration(duration);
	ui.cmb_proc->setToolTipDuration(duration);

	ui.lbl_pid->setToolTipDuration(duration);
	ui.rb_pid->setToolTipDuration(duration);
	ui.txt_pid->setToolTipDuration(duration);
	ui.btn_proc->setToolTipDuration(duration);

	ui.lbl_delay->setToolTipDuration(duration);
	ui.txt_delay->setToolTipDuration(duration);
	ui.cb_close->setToolTipDuration(duration);
	ui.cb_auto->setToolTipDuration(duration);

	// Method
	ui.cmb_load->setToolTipDuration(duration);
	ui.cb_hijack->setToolTipDuration(duration);
	ui.cmb_create->setToolTipDuration(duration);
	ui.cb_clock->setToolTipDuration(duration);

	// Cloaking
	ui.cmb_peh->setToolTipDuration(duration);
	ui.cb_unlink->setToolTipDuration(duration);
	ui.cb_random->setToolTipDuration(duration);
	ui.cb_copy->setToolTipDuration(duration);

	// Manuel mapping
	ui.cb_shift->setToolTipDuration(duration);
	ui.cb_clean->setToolTipDuration(duration);
	ui.cb_imports->setToolTipDuration(duration);
	ui.cb_delay->setToolTipDuration(duration);
	ui.cb_tls->setToolTipDuration(duration);
	ui.cb_seh->setToolTipDuration(duration);
	ui.cb_protection->setToolTipDuration(duration);
	ui.cb_main->setToolTipDuration(duration);

	ui.btn_reset->setToolTipDuration(duration);
	ui.btn_hooks->setToolTipDuration(duration);

	// Files
	ui.btn_add->setToolTipDuration(duration);
	ui.btn_inject->setToolTipDuration(duration);
	ui.btn_remove->setToolTipDuration(duration);

	// Info
	ui.btn_tooltip->setToolTipDuration(duration);
	ui.btn_help->setToolTipDuration(duration);
	ui.btn_log->setToolTipDuration(duration);
	ui.btn_version->setToolTipDuration(duration);
}

void GuiMain::open_help()
{
	bool ok = QDesktopServices::openUrl(QUrl(GH_HELP_URL, QUrl::TolerantMode));
}

void GuiMain::open_log()
{
	bool ok = QDesktopServices::openUrl(QUrl(GH_LOG_URL, QUrl::TolerantMode));
}

void GuiMain::check_online_version()
{

	ui.btn_version->setText("check version...");
	ui.btn_version->setEnabled(false);

	std::string ver = getVersionFromIE();
	ui.btn_help->setText(QString::fromStdString(ver));
	onlineVersion = QString::fromUtf8(ver.c_str());
	emit download_start();
	return;

	// Old
	ver_Manager->get(QNetworkRequest(QUrl(GH_VERSION_URL)));

	return;	
}

void GuiMain::replyFinished(QNetworkReply* reply)
{
	QByteArray bytes = reply->readAll();
	QString str = QString::fromUtf8(bytes.data(), bytes.size());
	int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if (reply->error() != QNetworkReply::NoError)
		ui.btn_version->setText("Error: no SSL");

	else if (statusCode == 200)
	{
		onlineVersion = str;
		emit download_start();
	}

	else if (statusCode == 0)
		ui.btn_version->setText("Fail: redirect " + str);

	else
		ui.btn_version->setText("Fail: http " + QString::number(statusCode));

	return;
}

void GuiMain::download_start()
{
	if (onlineVersion.isEmpty())
	{
		ui.btn_version->setText("Fail: no version");
		return;
	}

	if (onlineVersion == QString(GH_INJ_VERSIONA))
	{
		ui.btn_version->setText("&Version " + QString(GH_INJ_VERSIONA));
		return;
	}

#ifdef _WIN64
	
	char zipFile[MAX_PATH];
	char zipFolder[MAX_PATH];

	GetZipFilePath(zipFile, sizeof(zipFile));
	GetZipFolderPath(zipFolder, sizeof(zipFolder));

	RemoveFile(zipFile);
	RemoveFolder(zipFolder);

	ui.btn_version->setText("Updating to V" + onlineVersion);


	QString argument(GH_DOWNLOAD_PREFIX + onlineVersion + GH_DOWNLOAD_SUFFIX);
	//QUrl url("http://speedtest.tele2.net/1MB.zip");

	
	zipName = dl_Manager.saveFileName(QUrl(argument));
	dl_Manager.append(argument);
	
	
#else
	// We don't support x86 auto update

	ui.btn_version->setText("Old Version");
	
	QMessageBox messageBox;
	messageBox.information(0, "Success", "There is a new Version check the website");
	messageBox.setFixedSize(500, 200);

	QMessageBox::StandardButton reply;
	reply = QMessageBox::warning(nullptr, "New version", 
		"A new version is available. Should the web page be opened?", QMessageBox::Yes | QMessageBox::No);

	if (reply == QMessageBox::No)
		return;

	if (reply == QMessageBox::Yes)
		emit open_help();
	
#endif
}

void GuiMain::download_finish()
{
	ui.btn_version->setText("download finished");

	QString strCurDir = QFileInfo(zipName).absolutePath();
	QDir dirCurDir(strCurDir);

	QFile zipFile(zipName);
	if (!zipFile.exists())
	{
		ui.btn_version->setText(".zip not found");
		return;
	}

	std::string zipPath = QFileInfo(zipName).absoluteFilePath().toStdString();
	replaceAll(zipPath, "/", "\\");

	ui.btn_version->setText("unzip...");
	
	char zipFolder[MAX_PATH];
	GetZipFolderPath(zipFolder, sizeof(zipFolder));
	Unzip(zipPath.c_str(), zipFolder);

	char zipFolderExe[MAX_PATH];
	GetZipFolderExePath(zipFolderExe, sizeof(zipFolderExe));
	if(!QFile::exists(QString::fromLatin1(zipFolderExe)))
	{
		ui.btn_version->setText("unzip failed");
		return;
	}
	
	ui.btn_version->setText("unzip complete");

	ui.btn_version->setText("finished");
	return;
}