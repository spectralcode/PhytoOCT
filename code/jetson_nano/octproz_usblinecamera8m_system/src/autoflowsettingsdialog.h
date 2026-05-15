#pragma once

#include <QDialog>
#include <QVariantMap>

class QLineEdit;
class QSpinBox;

class AutoFlowSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AutoFlowSettingsDialog(QWidget* parent = nullptr);

	void setSettings(const QVariantMap& settings);
	void getSettings(QVariantMap* settings) const;

private:
	QLineEdit* prestartCmdEdit;
	QLineEdit* triggerStartCmdEdit;
	QLineEdit* triggerStopCmdEdit;
	QLineEdit* shutdownCmdEdit;
	QSpinBox* cudaDelaySpinBox;
};
