#include "octserialcompanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>


OCTSerialComPanel::OCTSerialComPanel(QWidget *parent)
	: QWidget(parent), 
	  octSerialCom(new OCTSerialCom(this)),
	  comPortSelectWidget(new ComPortSelectWidget(this))
{
	connect(this->comPortSelectWidget, &ComPortSelectWidget::connectToComPortRequested, this->octSerialCom, &OCTSerialCom::connectToComPort);
	connect(this->comPortSelectWidget, &ComPortSelectWidget::disconnectFromComPortRequested, this->octSerialCom, &OCTSerialCom::disconnectFromComPort);
	this->setupUI();
}

void OCTSerialComPanel::setupUI()
{
	//default comand gui elements
	QPushButton* enableSLDButton = new QPushButton("Enable SLD", this);
	enableSLDButton->setFocusPolicy(Qt::NoFocus);
	QPushButton* disableSLDButton = new QPushButton("Disable SLD", this);
	disableSLDButton->setFocusPolicy(Qt::NoFocus);
	QVBoxLayout* defaultCommandLayout = new QVBoxLayout();
	defaultCommandLayout->addWidget(enableSLDButton);
	defaultCommandLayout->addWidget(disableSLDButton);
	
	connect(enableSLDButton, &QPushButton::clicked, [this]() {
		this->octSerialCom->sendCommand("sld=1");
	});
	connect(disableSLDButton, &QPushButton::clicked, [this]() {
		this->octSerialCom->sendCommand("sld=0");
	});

	//custom command gui elements
	QLineEdit* customCommandLineEdit = new QLineEdit(this);
	QPushButton* sendCommandButton = new QPushButton("Send", this);
	sendCommandButton->setFocusPolicy(Qt::NoFocus);
	QHBoxLayout *commandLayout = new QHBoxLayout();
	commandLayout->addWidget(customCommandLineEdit);
	commandLayout->addWidget(sendCommandButton);
	
	connect(sendCommandButton, &QPushButton::clicked, [this, customCommandLineEdit]() {
		this->octSerialCom->sendCommand(customCommandLineEdit->text());
	});
	connect(customCommandLineEdit, &QLineEdit::returnPressed, sendCommandButton, &QPushButton::click);
	

	//response text area 
	QTextEdit *responseTextEdit = new QTextEdit(this);
	responseTextEdit->setReadOnly(true);
	connect(this->octSerialCom, &OCTSerialCom::responseReceived, this, 
		[responseTextEdit](const QString &response) {
			responseTextEdit->append(response);
			//limit how much text can be in text area
			int maxTextLength = 30000;
			if (responseTextEdit->toPlainText().length() > maxTextLength) {
				responseTextEdit->setPlainText(responseTextEdit->toPlainText().right(maxTextLength));
				responseTextEdit->moveCursor(QTextCursor::End);
			}
		});

	//main layout
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(comPortSelectWidget);
	mainLayout->addLayout(defaultCommandLayout);
	mainLayout->addLayout(commandLayout);
	mainLayout->addWidget(responseTextEdit);
	setLayout(mainLayout);

	this->setLayoutEnabled(defaultCommandLayout, false);
	this->setLayoutEnabled(commandLayout, false);
	responseTextEdit->setEnabled(false);

	connect(this->octSerialCom, &OCTSerialCom::connectionEstablished, this, 
		[this, defaultCommandLayout, commandLayout, responseTextEdit](bool connected) {
			this->setLayoutEnabled(defaultCommandLayout, connected);
			this->setLayoutEnabled(commandLayout, connected);
			responseTextEdit->setEnabled(connected);
		});
}

void OCTSerialComPanel::setLayoutEnabled(QLayout *layout, bool enabled)
{
	if(!layout){
		return;
	}
	for(int i = 0; i < layout->count(); ++i){
		QWidget* widget = layout->itemAt(i)->widget();
		if(widget){
			widget->setEnabled(enabled);
		}
	}
}
