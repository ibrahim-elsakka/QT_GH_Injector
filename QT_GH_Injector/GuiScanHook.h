#pragma once

#include <qfilesystemmodel.h>
#include <QWidget>
#include "ui_GuiScanHook.h"



class GuiScanHook : public QWidget
{
	Q_OBJECT

public:
	GuiScanHook(QWidget* parent = Q_NULLPTR);
	~GuiScanHook();

private:
	Ui::Form ui;

signals:
	void send_to_inj_sh(int pid, int error);

public slots:
	void get_from_inj_sh(int pid, int error);

private slots:

	void refresh_gui();
	void scan_clicked();
	void unhook_clicked();
};
