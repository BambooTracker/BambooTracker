#pragma once

#include "audio_stream.hpp"
#include <memory>

class RtAudio;

class AudioStreamRtAudio final : public AudioStream
{
public:
	explicit AudioStreamRtAudio(QObject* parent = nullptr);
	~AudioStreamRtAudio() override;

	void initialize(uint32_t rate, uint32_t duration, uint32_t intrRate, const QString& device) override;
	void shutdown() override;

	std::vector<std::string> getAvailableDevices() override;

private:
	std::unique_ptr<RtAudio> audio_;
};
