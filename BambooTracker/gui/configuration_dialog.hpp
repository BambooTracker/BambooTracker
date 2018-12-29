#ifndef CONFIGURATION_DIALOG_HPP
#define CONFIGURATION_DIALOG_HPP

#include <QDialog>
#include <memory>
#include "configuration.hpp"

namespace Ui {
	class ConfigurationDialog;
}

class ConfigurationDialog : public QDialog
{
	Q_OBJECT

public:
	ConfigurationDialog(std::weak_ptr<Configuration> config, QWidget *parent = nullptr);
	~ConfigurationDialog() override;

signals:
	void applyPressed();

private slots:
	void on_ConfigurationDialog_accepted();
	void on_generalSettingsListWidget_itemSelectionChanged();

private:
	Ui::ConfigurationDialog *ui;
	std::weak_ptr<Configuration> config_;

	inline Qt::CheckState toCheckState(bool enabled)
	{
		return enabled ? Qt::Checked : Qt::Unchecked;
	}

	inline bool fromCheckState(Qt::CheckState state)
	{
		return (state == Qt::Checked) ? true : false;
	}
};

#endif // CONFIGURATION_DIALOG_HPP
