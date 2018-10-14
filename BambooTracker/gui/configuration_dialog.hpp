#ifndef CONFIGURATION_DIALOG_HPP
#define CONFIGURATION_DIALOG_HPP

#include <QDialog>
#include <memory>
#include "bamboo_tracker.hpp"
#include "audio_stream.hpp"

namespace Ui {
	class ConfigurationDialog;
}

class ConfigurationDialog : public QDialog
{
	Q_OBJECT

public:
	ConfigurationDialog(std::weak_ptr<BambooTracker> core, std::weak_ptr<AudioStream> stream,
						QWidget *parent = nullptr);
	~ConfigurationDialog() override;

private slots:
	void on_ConfigurationDialog_accepted();

private:
	Ui::ConfigurationDialog *ui;
	std::weak_ptr<BambooTracker> core_;
	std::weak_ptr<AudioStream> stream_;
};

#endif // CONFIGURATION_DIALOG_HPP
