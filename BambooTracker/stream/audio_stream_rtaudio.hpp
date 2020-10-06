/*
 * Copyright (C) 2019-2020 Rerrah
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

#pragma once

#include "audio_stream.hpp"
#include <memory>

class RtAudio;

class AudioStreamRtAudio final : public AudioStream
{
public:
	explicit AudioStreamRtAudio(QObject* parent = nullptr);
	~AudioStreamRtAudio() override;

	bool initialize(uint32_t rate, uint32_t duration, uint32_t intrRate,
					const QString& backend, const QString& device,
					QString* errDetail) override;
	void shutdown() override;

	std::vector<QString> getAvailableBackends() const override;
	QString getCurrentBackend() const override;
	std::vector<QString> getAvailableDevices() const override;
	virtual std::vector<QString> getAvailableDevices(const QString& backend) const override;
	QString getDefaultOutputDevice() const override;
	QString getDefaultOutputDevice(const QString& backend) const override;

	void start() override;
	void stop() override;

private:
	std::unique_ptr<RtAudio> audio_;

	void setBackend(const QString& backend);
};
