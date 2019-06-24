#pragma once
#include <vector>
#include "sequence_iterator_interface.hpp"

class ArpeggioEffectIterator : public SequenceIteratorInterface
{
public:
	ArpeggioEffectIterator(int second, int third);
	int getPosition() const override;
	int getSequenceType() const override;
	int getCommandType() const override;
	int getCommandData() const override;
	int next(bool isReleaseBegin = false) override;
	int front() override;
	int end() override;

private:
	int pos_;
	bool started_;
	int second_, third_;
};

class WavingEffectIterator : public SequenceIteratorInterface
{
public:
	WavingEffectIterator(int period, int depth);
	int getPosition() const override;
	int getSequenceType() const override;
	int getCommandType() const override;
	int getCommandData() const override;
	int next(bool isReleaseBegin = false) override;
	int front() override;
	int end() override;

private:
	int pos_;
	bool started_;
	std::vector<int> seq_;
};

class NoteSlideEffectIterator : public SequenceIteratorInterface
{
public:
	NoteSlideEffectIterator(int speed, int seminote);
	int getPosition() const override;
	int getSequenceType() const override;
	int getCommandType() const override;
	int getCommandData() const override;
	int next(bool isReleaseBegin = false) override;
	int front() override;
	int end() override;

private:
	int pos_;
	bool started_;
	std::vector<int> seq_;
};
