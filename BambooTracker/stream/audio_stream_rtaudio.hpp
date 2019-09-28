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

	std::vector<std::string> getAvailableDevices() const override;
	std::vector<std::string> getAvailableBackends() const override;
	std::string getCurrentBackend() const override;

	void start() override;
	void stop() override;

private:
	std::unique_ptr<RtAudio> audio_;

	void setBackend(const QString& backend);
};
