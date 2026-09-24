#pragma once
#include <stdint.h>

constexpr uint16_t PC_START_ADDR = 0x100;
constexpr uint16_t IF_ADDR = 0xff0f;  // interrupt req
constexpr uint16_t IE_ADDR = 0xffff;  // interrupt enabled

constexpr uint16_t STAT_ADDR = 0xff44;

/* ------------------------- PPU ------------------------ */
constexpr uint16_t OAM_START = 0xfe00;
constexpr uint16_t OAM_END = 0xfe9f;
constexpr uint16_t LCDC_ADDR = 0xff40;

constexpr uint16_t VRAM_START = 0x8000;
constexpr uint16_t VRAM_END = 0x97ff;

/* ------------------------ timer ----------------------- */
constexpr uint16_t DIV_ADDR = 0xff04;   // div register
constexpr uint16_t TIMA_ADDR = 0xff05;  // timer counter
constexpr uint16_t TMA_ADDR = 0xff06;   // timer modulo
constexpr uint16_t TAC_ADDR = 0xff07;   // timer control