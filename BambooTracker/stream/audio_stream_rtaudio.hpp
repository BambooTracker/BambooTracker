#pragma once

#include "audio_stream.hpp"
#include <memory>

class RtAudio;

class AudioStreamRtAudio final : public AudioStream
{
public:
	explicit AudioStreamRtAudio(QObject* parent = nullptr);
	~AudioStreamRtAudio() override;

	bool initialize(uint32_t rate, uint32_t duration, uint32_t intrRate, const QString& backend, const QString& device) override;
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
