#include "autoflowsettingsdialog.h"
#include "usblinecamera8msystemsettingsdialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

AutoFlowSettingsDialog::AutoFlowSettingsDialog(QWidget* parent)
	: QDialog(parent)
{
	this->setWindowTitle(tr("Auto-flow command settings"));

	auto* form = new QFormLayout;

	this->prestartCmdEdit = new QLineEdit("sld=1", this);
	this->prestartCmdEdit->setToolTip(tr("Sent at the start of every acquisition, just before the trigger command. Default enables the SLD."));
	form->addRow(tr("Pre-start command:"), this->prestartCmdEdit);

	this->triggerStartCmdEdit = new QLineEdit("run=1", this);
	this->triggerStartCmdEdit->setToolTip(tr("Sent after CUDA has initialised, when the main app's Start button is pressed."));
	form->addRow(tr("Trigger start command:"), this->triggerStartCmdEdit);

	this->triggerStopCmdEdit = new QLineEdit("run=0", this);
	this->triggerStopCmdEdit->setToolTip(tr("Sent at the very beginning of Stop, before the camera halts."));
	form->addRow(tr("Trigger stop command:"), this->triggerStopCmdEdit);

	this->shutdownCmdEdit = new QLineEdit("sld=0", this);
	this->shutdownCmdEdit->setToolTip(tr("Sent when OCTproZ exits. Default disables the SLD."));
	form->addRow(tr("Shutdown command:"), this->shutdownCmdEdit);

	this->cudaDelaySpinBox = new QSpinBox(this);
	this->cudaDelaySpinBox->setRange(0, 10000);
	this->cudaDelaySpinBox->setSuffix(" ms");
	this->cudaDelaySpinBox->setValue(500);
	this->cudaDelaySpinBox->setToolTip(tr("Delay between Start (CUDA init kicks off) and sending the trigger start command. Also serves as the SLD warmup window on each Start."));
	form->addRow(tr("CUDA init delay:"), this->cudaDelaySpinBox);

	auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto* root = new QVBoxLayout(this);
	root->addLayout(form);
	root->addWidget(buttons);
}

void AutoFlowSettingsDialog::setSettings(const QVariantMap& settings)
{
	this->prestartCmdEdit->setText(settings.value(PRESTART_CMD, "sld=1").toString());
	this->triggerStartCmdEdit->setText(settings.value(TRIGGER_START_CMD, "run=1").toString());
	this->triggerStopCmdEdit->setText(settings.value(TRIGGER_STOP_CMD, "run=0").toString());
	this->shutdownCmdEdit->setText(settings.value(SHUTDOWN_CMD, "sld=0").toString());
	this->cudaDelaySpinBox->setValue(settings.value(CUDA_INIT_DELAY_MS, 500).toInt());
}

void AutoFlowSettingsDialog::getSettings(QVariantMap* settings) const
{
	settings->insert(PRESTART_CMD,       this->prestartCmdEdit->text());
	settings->insert(TRIGGER_START_CMD,  this->triggerStartCmdEdit->text());
	settings->insert(TRIGGER_STOP_CMD,   this->triggerStopCmdEdit->text());
	settings->insert(SHUTDOWN_CMD,       this->shutdownCmdEdit->text());
	settings->insert(CUDA_INIT_DELAY_MS, this->cudaDelaySpinBox->value());
}
