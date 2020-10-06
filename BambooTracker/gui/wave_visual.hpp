/*
 * Copyright (C) 2019 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

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
	void setColorPalette(std::shared_ptr<ColorPalette> palette);
	void setStereoSamples(const int16_t *buffer, size_t frames);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	std::shared_ptr<ColorPalette> palette_;
	std::vector<int16_t> samples_;
};

#endif // WAVE_VISUAL_HPP
