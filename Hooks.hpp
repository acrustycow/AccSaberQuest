#pragma once

#include "beatsaber-hook/shared/utils/logging.hpp"

// Logger used across the whole mod
static constexpr auto MOD_LOGGER = Paper::ConstLoggerContext("AccSaberQuest");

#define LOG_INFO(...)  MOD_LOGGER.info(__VA_ARGS__)
#define LOG_WARN(...)  MOD_LOGGER.warn(__VA_ARGS__)
#define LOG_ERROR(...) MOD_LOGGER.error(__VA_ARGS__)
#define LOG_DEBUG(...) MOD_LOGGER.debug(__VA_ARGS__)
