#pragma once

enum class StatusRet    { ERROR     = 0, SUCCESS = 1 };
enum class State        { OFF       = 0, ON      = 1 };
enum class Mode         { FORBIDDEN = 0, ALLOWED = 1 };
enum class R            { NOT_READY = 0, READY   = 1 };
enum class F            { NOT_FAULT = 0, FAULT   = 1 };
enum class MeterPhase   { RISING    = 0, _13DEG  = 1 };

enum class StatusHVS 
{ 
  OFF = 0, 
  ON = 1,
  ERR_BC = 2
};
