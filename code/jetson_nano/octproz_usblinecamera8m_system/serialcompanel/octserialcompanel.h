#ifndef OCTSERIALCOMPANEL_H
#define OCTSERIALCOMPANEL_H

#include <QWidget>

#include "octserialcom.h"
#include "comportselectwidget.h"

class OCTSerialComPanel : public QWidget
{
	Q_OBJECT

public:
	explicit OCTSerialComPanel(QWidget *parent = nullptr);
	OCTSerialCom* getOCTSerialCom(){return this->octSerialCom;}

private slots:

private:
	OCTSerialCom* octSerialCom;
	ComPortSelectWidget* comPortSelectWidget;

	void setupUI();
	void setLayoutEnabled(QLayout* layout, bool enabled);
};

#endif // OCTSERIALCOMPANEL_H
