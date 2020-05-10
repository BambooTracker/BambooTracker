#ifndef SAMPLE_LENGTH_DIALOG_HPP
#define SAMPLE_LENGTH_DIALOG_HPP

#include <QDialog>

namespace Ui {
	class SampleLengthDialog;
}

class SampleLengthDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SampleLengthDialog(int len, QWidget *parent = nullptr);
	~SampleLengthDialog() override;

	int getLength() const;

private:
	Ui::SampleLengthDialog *ui;
};

#endif // SAMPLE_LENGTH_DIALOG_HPP
