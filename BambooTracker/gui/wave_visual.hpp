#ifndef WAVE_VISUAL_HPP
#define WAVE_VISUAL_HPP

#include <QWidget>
#include <vector>
#include <memory>
#include <cstdint>

class ColorPalette;

class WaveVisual : public QWidget
{
	Q_OBJECT

public:
	explicit WaveVisual(QWidget *parent = nullptr);
	void setColorPalette(std::weak_ptr<ColorPalette> palette);
	void setStereoSamples(const int16_t *buffer, size_t frames);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	std::weak_ptr<ColorPalette> palette_;
	std::vector<int16_t> samples_;
};

#endif // WAVE_VISUAL_HPP
