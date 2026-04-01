#include "pch.h"
#include "TimeManager.h"

float TimeManager::m_elapsedTime = 0;
std::unique_ptr<TimeManager> TimeManager::Time = nullptr;

void TimeManager::Initialize()
{
	QueryPerformanceFrequency(&m_frequency);
	QueryPerformanceCounter(&m_previousTime);
	QueryPerformanceCounter(&m_currentTime);
}

void TimeManager::Update()
{
	m_previousTime = m_currentTime;
	QueryPerformanceCounter(&m_currentTime);
	m_deltaTime = (float)(m_currentTime.QuadPart - m_previousTime.QuadPart) / (float)(m_frequency.QuadPart);
	m_elapsedTime += m_deltaTime;
}
